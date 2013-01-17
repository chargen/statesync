#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <error.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <glib.h>

struct File_entry {
    char* file_name;
    int size;
    time_t mtime;
    mode_t st_mode;
    char hash[40]; //SHA-1 hash of file
};

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

int main(int argc, char** argv) {
    char *red = "\033[01;31m";
    char *reset_color = "\033[00m";
    char* green = "\033[00;32m";
    GList* list = NULL;
    getFilesRecursive(".", &list);
    printf("# Statesync status\n");
    printf("#\n");
    for(GList* current = g_list_next(list); current != NULL; current = g_list_next(current)) {
        struct File_entry* entry = (struct File_entry*) current->data;
        printf("#    %s%s%s\n", red, entry->file_name, reset_color);
//        printf("%d\t", entry->size);
//        printf("%ld\t", entry->mtime);
//        printf("%s\n", S_ISDIR(entry->st_mode) ? "d" : "f");

    }
    printf("#\n");
    printf("# Found %d files and folders\n", g_list_length(list));
}

