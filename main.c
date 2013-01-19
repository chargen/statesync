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

void  getFilesRecursive(char* filename, GList** list) {
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
        //Instead of the path append a struct with the necessary information;
        struct File_entry* file_entry = malloc(sizeof(struct File_entry));
        file_entry->file_name = currentPath;
        file_entry->size = getFileSize(currentPath);
        struct stat filestat;
        stat(currentPath, &filestat);
        file_entry->mtime = filestat.st_mtime;
        file_entry->st_mode = filestat.st_mode;
        *list = g_list_append(*list, file_entry);
        if(isDir)
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

struct File_entry* getEntryForAdd(char* filename) {

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
            GList* list = NULL;
            getFilesRecursive(".", &list);
            pid_t pid = fork();
            if(pid == 0) {
                //child
                close(fd[1]); //close output
                //read from fd[0]
                dup2(fd[0], 0);
                execl("./statesync", "./statesync", "receive", NULL);
            } else {
                //parent
                close(fd[0]); //close input
                //write to fd[1]
                printf("# Sending data items: %d\n", g_list_length(list));
                dup2(fd[1], 1);
                for(GList* current = list; current != NULL; current = g_list_next(current)) {
                    if(current->data == NULL) break;
                    fwrite(current->data, 1, sizeof(struct File_entry), stdout);
                }
                fflush(stdout);
                close(fd[1]);
                int pid_status;
                waitpid(pid, &pid_status, 0);
            }
        } else if(strcmp(argv[1], "receive") == 0) {
            GList* list = NULL;
            struct File_entry* data = malloc(sizeof(struct File_entry));
            while(fread(data, 1, sizeof(struct File_entry), stdin) != 0) {
                list = g_list_append(list, data);
                data = malloc(sizeof(struct File_entry));
            }
            printf("Finished loop, got: %d items!\n", g_list_length(list));
            printStatus(list);
            printf("Done\n");
            fflush(stdout);
        }
    }
}

