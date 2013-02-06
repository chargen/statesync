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
            // object_entries is a NULL terminated array
            struct File_entry** object_entries = getObjectEntry(currentPath);
            // Here we should check if the entry is different from
            // the entry saved in the object storage and only print files
            // that are either not in the object storage or that have
            // changed.
            if(object_entries != NULL) {
                struct File_entry* second_entry = *(object_entries+1);
                // First we check if a second entry exists, and if not then we
                // compare the first one.
                if(second_entry != NULL) {
                    if(compareFileEntries(file_entry, second_entry) != 0) {
                        // Second line is different so the current file has
                        // changed since it was last modified in the
                        // object store.
                        // TODO: we probably want to add files that have
                        //      been modified to a different list
                        *list = g_list_append(*list, file_entry);
                    }
                } else if (compareFileEntries(file_entry, *object_entries) != 0) {
                    // First entry has changed, so the current file has
                    // changed since it was added to the object store.
                    // TODO: we probably want to add files that have
                    //      been modified to a different list
                    *list = g_list_append(*list, file_entry);
                } else {
                    // The file has not been modified since it was added
                    // to the object store, we do nothing.
                }
            } else {
                // There are no entries yet in the object store, this
                // is an untracked file.
                *list = g_list_append(*list, file_entry);
            }
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
