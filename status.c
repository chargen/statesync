#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <glib.h>
#include "types.h"
#include "FileEntry.h"

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
        int isDir = g_file_test(currentPath, G_FILE_TEST_IS_DIR);
        if(isDir == 0) {
            struct File_entry* file_entry = getEntryFromFilename(currentPath);
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
