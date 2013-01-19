#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "types.h"

void createDirectory(char* filename) {
    char* directory = g_build_filename(filename, ".statesync", NULL);
    mkdir(directory, S_IRWXU);
}

void entryToString(struct File_entry* entry, char** line) {
    snprintf(*line, 2000, "%s,%d,%ld,%d,%s\n", 
            entry->file_name,
            entry->size,
            entry->mtime,
            entry->st_mode,
            entry->hash);
}

void stringToEntry(struct File_entry** entry, char* line) {
    gchar** tokens = g_strsplit(line, ",", 5);
    int i = 0;
    (*entry)->file_name = tokens[i++];
    (*entry)->size = atoi(tokens[i++]);
    (*entry)->mtime = atol(tokens[i++]);
    (*entry)->st_mode = atoi(tokens[i++]);
    //            snprintf(entry->hash, 40, "%s", tokens[i++]);
    (*entry)->hash[40] = '\0';
}

GList* readFile(char* filename) {
    char* path = g_build_filename(filename, ".statesync", "files.db", NULL);
    FILE* config_file = fopen(path, "r");
    GList* list = NULL;
    char* line = malloc(2000);
    if(config_file) {
        while((line = fgets(line, 2000, config_file)) != NULL) {
            //split line;
            struct File_entry* entry = malloc(sizeof(struct File_entry));
            stringToEntry(&entry, line);
            list = g_list_append(list, entry);
        }
    }
    free(line);
    return list;
}

void writeFile(char* filename, GList* list) {
    char* path = g_build_filename(filename, ".statesync", "files.db", NULL);
    FILE* config_file = fopen(path, "w");
    if(list == NULL) return;
    char* line = malloc(2000);
    GList* current = list;
    if(config_file) {
        do {
            struct File_entry* entry = (struct File_entry*) current->data;
            //split line;
            entryToString(entry, &line);
            fputs(line, config_file);
        } while((current = g_list_next(current)) != NULL);
        fflush(config_file);
        fclose(config_file);
    }
    free(line);
}
