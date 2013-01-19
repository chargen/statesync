#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <error.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#include <glib.h>
#include "add.h"
#include "types.h"

char* getLastModTime(char* filename) {
    struct stat currentStat;
    struct tm* localtm;
    stat(filename, &currentStat);
    time_t t = currentStat.st_mtime;
    localtm = localtime(&t);
    char outstr[200];
    if(strftime(outstr, sizeof(outstr), "%Y.%m.%d %H:%M:%S", localtm) != 0) {
        char* resultStr = malloc(strlen(outstr)+1);
        strncpy(resultStr, outstr, strlen(outstr)+1);
        return resultStr;
    } else {
        return NULL;
    }
}

off_t getFileSize(char* filename) {
    struct stat currentStat;
    stat(filename, &currentStat);
    return currentStat.st_size;
}

int isDirectory(char* filename) {
    struct stat currentStat;
    stat(filename, &currentStat);
    if(S_ISDIR(currentStat.st_mode))
        return 1;
    else
        return 0;
}

void printIndent(int count) {
    for(int i=0;i<count;i++) {
        printf("\t");
    }
}

void printDirectoryStructure(char *filename) {
    static int indent = 0;
    DIR* dir = opendir(filename);
    if(dir == NULL) {
        return;
    }
    struct dirent* directory;

    while((directory = readdir(dir)) != NULL) {
        char* currentPath = g_build_filename(filename, directory->d_name, NULL);
        char* currentFile = directory->d_name;
        int isDir = isDirectory(currentPath);
        printIndent(indent);
        printf("%s\t", currentFile);
        if(isDir)
            printf("d\t");
        else
            printf("f\t");
        printf("%s\t", getLastModTime(currentPath));
        printf("%d\n", (int) getFileSize(currentPath));
        if(isDir && strcmp(".", currentFile) && strcmp("..", currentFile)) {
            indent++;
            printDirectoryStructure(currentPath);
        }
    }
    --indent;
    printf("\n");
}

struct File_entry* statToEntry(char* currentPath, struct stat* st) {
    struct File_entry* entry = malloc(sizeof(struct File_entry));
    if(entry != NULL) {
        entry->file_name = currentPath;
        entry->size = st->st_size;
        entry->mtime = st->st_mtime;
        entry->st_mode = st->st_mode;
        //Calculate the hash value of the file
        //TODO:...
    } else {
        return NULL;
    }
}

void getFilesRecursive(char* filename, GList** list) {
    DIR* dir = opendir(filename);
    if(dir == NULL) return;
    struct dirent* directory;

    while((directory = readdir(dir)) != NULL) {
        char* currentPath;
        if(strcmp(".", filename)) {
            currentPath = g_build_filename(filename, directory->d_name, NULL);
        } else {
            currentPath = g_build_filename(directory->d_name, NULL);
        }
        char* currentFile = directory->d_name;
        //XOR is used to continue if either string is matched but not both
        if(strcmp(".", currentFile) ^ strcmp("..", currentFile)) continue;
        int isDir = isDirectory(currentPath);
        if(isDir == 0) {
            struct File_entry* file_entry = malloc(sizeof(struct File_entry));
            file_entry->file_name = currentPath;
            file_entry->size = getFileSize(currentPath);
            struct stat filestat;
            stat(currentPath, &filestat);
            file_entry->mtime = filestat.st_mtime;
            file_entry->st_mode = filestat.st_mode;
            *list = g_list_append(*list, file_entry);
        } else
            getFilesRecursive(currentPath, list);
    }
}

void printStatus(GList* list) {
    char *red = "\033[01;31m";
    char *reset_color = "\033[00m";
    char* green = "\033[00;32m";
    printf("# Statesync status\n");
    printf("#\n");
    for(GList* current = list; current != NULL; current = g_list_next(current)) {
        struct File_entry* entry = (struct File_entry*) current->data;
        printf("#    %s%s%s\n", red, entry->file_name, reset_color);
    }
    printf("#\n");
    printf("# Found %d files and folders\n", g_list_length(list));
}

int main(int argc, char** argv) {
    createDirectory(".");
    if(argc == 2) {
        if(strcmp(argv[1], "status") == 0) {
            GList* list = NULL;
            getFilesRecursive(".", &list);
            printStatus(list);
        } else if(strcmp(argv[1], "add") == 0) {
            //TO THE ADDING
            if(argc < 3) {
                printf("# No files to add were specified\n");
                printf("#\n");
            } else {
                GList* list = NULL;
                for(int i=2; i<argc; i++) {
                    list = g_list_append(list, argv[i]);
                }
                writeFile(".", list);
                list = NULL;
                list = readFile(".");
                printf("Read: %d items\n", g_list_length(list));
            }
        } else if(strcmp(argv[1], "send") == 0) {
            int fd[2];
            pipe(fd);
            pid_t pid = fork();
            if(pid == 0) {
                //child
                close(fd[1]); //close output
                //read from fd[0]
                dup2(fd[0], 0);
                execl("/usr/bin/ssh", "/usr/bin/ssh", "localhost", "./Documents/Code/statesync/statesync", "receive", NULL);
            } else {
                //parent
                close(fd[0]); //close input
                //write to fd[1]
                GList* list = NULL;
                getFilesRecursive(".", &list);
                printf("# Sending data items: %d\n", g_list_length(list));
                dup2(fd[1], 1);
                fflush(stdout);
                char* line = malloc(2000);
                for(GList* current = list; current != NULL; current = g_list_next(current)) {
                    if(current->data == NULL) break;
                    struct File_entry* entry = (struct File_entry*) current->data;
                    entryToString(entry, &line);
                    fwrite(line, 1, strlen(line), stdout);
                }
                fwrite("0000\n", 1, 5, stdout);
                //start sending files
                for(GList* current = list; current != NULL; current = g_list_next(current)) {
                    struct File_entry* entry = (struct File_entry*) current->data;
                    char* filename = entry->file_name;
                    FILE* file = fopen(filename, "r");
                    unsigned char buffer[1024];
                    for(int i = 0; i<entry->size / 1024;i++) {
                        fread(buffer, 1024, 1, file);
                        fwrite(buffer, 1024, 1, stdout);
                    }
                    if(entry->size % 1024 > 0) {
                        fread(buffer, entry->size % 1024, 1, file);
                        fwrite(buffer, entry->size % 1024, 1, stdout);
                    }
                }
                fflush(stdout);
                //In order to close the pipe's output stream we need to close both file descriptors
                close(fd[1]);
                fclose(stdout);
                int pid_status;
                waitpid(pid, &pid_status, 0);
            }
        } else if(strcmp(argv[1], "receive") == 0) {
            GList* list = NULL;
            unsigned char buffer[1024];
            char* data = malloc(2000);
            //TODO: cread directly from pipe and not from stdin
            while((data = fgets(data, 2000, stdin)) != NULL) {
                if(strcmp(data, "0000\n") == 0) break;
                list = g_list_append(list, data);
                data = malloc(2000);
            }
            for(GList* current = list; current != NULL; current = g_list_next(current)) {
                char* data = (char*) current->data;
                struct File_entry* entry = malloc(sizeof(struct File_entry));
                stringToEntry(&entry, data);
                char* receive_file_name = g_build_filename("received", entry->file_name, NULL);
                FILE* file = fopen(receive_file_name, "w");
                if(file == NULL) {
                    char* pathname = g_path_get_dirname(receive_file_name);
                    int result = g_mkdir_with_parents(pathname, 0755);
                    if(result == -1) {
                        perror("Could not create folder");
                        printf("%s\n", pathname);
                        exit(EXIT_FAILURE);
                    } else {
                        file = fopen(receive_file_name, "w");
                        if(file == NULL) {
                            perror("Error");   
                            printf("%s\n", receive_file_name);
                        }
                    }
                    free(pathname);
                }
                for(int i = 0; i<entry->size/1024; i++) {
                    fread(buffer, 1024, 1, stdin);
                    fwrite(buffer, 1024, 1, file);
                }
                if(entry->size % 1024 > 0) {
                    fread(buffer, 1, entry->size % 1024, stdin);
                    fwrite(buffer, 1, entry->size % 1024, file);
                }
                fflush(file);
                fclose(file);
                printf("# Received file: %s\n", receive_file_name);
                free(data);
                free(receive_file_name);
            }
            fflush(stdout);
            g_list_free(list);
        }
    }
}

