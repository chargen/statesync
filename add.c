#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

#include <glib.h>

#include "types.h"
#include "util.h"
#include "FileEntry.h"
#include "add.h"

void createDirectory(char* basedir) {
    char* directory = g_build_filename(basedir, ".statesync", "objects", NULL);
    g_mkdir_with_parents(directory, S_IRWXU);
}

/**
 * Check if stat indicates a file change, so we know if we need to hash the file or not.
 * @returns 1 if file is the same, 0 if it changed
 */
int isChanged(struct File_entry* entry, struct stat* st) {
    //identify if the file has changed and if we need to hash it
    if(entry->size != st->st_size) return 0;
    if(entry->mtime != st->st_mtime) return 0;
    //TODO Add ctime
    if(entry->st_mode != st->st_mode) return 0;
    return 1;
}

int addFile(char* filename) {
    //1. Hash filename
    char* basename_hash = filenameToHash(filename);
    //2. Check if .statesync/objects/<hash> exists
    char* object_filename = g_build_filename(".statesync", "objects", basename_hash, NULL);    
    FILE* object = fopen(object_filename, "r");
    struct File_entry* entry = getEntryFromFilename(filename);
    if(object != NULL) {
        //3a. Yes -> Load File_entry datas
        char* line1 = (char*) malloc(2000);
        char* line2 = (char*) malloc(2000);
        char* new_line;
        line1 = fgets(line1, 2000, object);
        line2 = fgets(line2, 2000, object);
        struct File_entry* object_entry1 = (struct File_entry*) malloc(sizeof(struct File_entry));
        struct File_entry* object_entry2 = (struct File_entry*) malloc(sizeof(struct File_entry));
        stringToEntry(object_entry1, line1);
        if(line2 != NULL) {
            stringToEntry(object_entry2, line2);
        }
        //4. Compare current information with the loaded data
        if(line2 != NULL) {
            //A second line exists if the file has been modified before
            if(compareFileEntries(entry, object_entry2) == 0) {
                //same: Nothing to do
                return 1;
            } else {
                //different: remove line one, move line 2 up and add new line
                fclose(object);
                object = fopen(object_filename, "w");
                if(object == NULL) return 0; //error
                fputs(line2, object);
                new_line = entryToString(entry);
                fputs(new_line, object);
                free(new_line);
            }
        } else {
            if(compareFileEntries(entry, object_entry1) == 0) {
                //same: Nothing to do
                return 1;
            } else {
                //different: Reopen object file in append mode and add a second line
                fclose(object);
                object = fopen(object_filename, "a");
                if(object == NULL) return 0; //error
                new_line = entryToString(entry);
                fputs(new_line, object);
            }
        }
        free(line1);
        free(line2);
        free(object_entry1);
        free(object_entry2);
    } else {
        //3b. No -> create it
        //The file was not there, so we are going to add it to the object tree
        object = fopen(object_filename, "w");
        char* line = entryToString(entry);
        fwrite(line, strlen(line), 1, object);
    }
    fclose(object);
    free(entry);
    return 1;
}

GList* readFile(char* basedirectory) {
    char* path = g_build_filename(basedirectory, ".statesync", "files.db", NULL);
    FILE* config_file = fopen(path, "r");
    GList* list = NULL;
    char* line = (char*) malloc(2000);
    if(config_file) {
        while((line = fgets(line, 2000, config_file)) != NULL) {
            //split line;
            struct File_entry* entry = (struct File_entry*) malloc(sizeof(struct File_entry));
            stringToEntry(entry, line);
            list = g_list_append(list, entry);
        }
    }
    free(line);
    return list;
}

void writeFile(char* basedirectory, GList* list) {
    char* path = g_build_filename(basedirectory, ".statesync", "files.db", NULL);
    FILE* config_file = fopen(path, "w");
    if(list == NULL) return;
    char* line;
    GList* current = list;
    if(config_file) {
        do {
            struct File_entry* entry = (struct File_entry*) current->data;
            //split line;
            line = entryToString(entry);
            fputs(line, config_file);
        } while((current = g_list_next(current)) != NULL);
        fflush(config_file);
        fclose(config_file);
    }
    free(line);
}
