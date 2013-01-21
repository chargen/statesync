#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

#include <glib.h>

#include "types.h"
#include "util.h"

void createDirectory(char* basedir) {
    char* directory = g_build_filename(basedir, ".statesync", NULL);
    g_mkdir_with_parents(directory, S_IRWXU);
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

/**
 * Check if stat indicates a file change, 
 */
int isChanged(struct File_entry* entry, struct stat* st) {
    //identify if the bits 
}

/**
 * Returns 0 on failure, 1 on success
 */
int addFile(char* filename) {
    //1. Hash filename
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    FILE * file = fopen(filename, "r");
    unsigned char buffer[1024] = {0};
    if(size == -1) return 0;
    if(size >= 1024) {
        for(int i=0; i < (size / 1024); i++) {
            fread(buffer, 1024, 1, file);
            SHA1_Update(&ctx, buffer, 1024);
        }
    }
    if((size % 1024) > 0) {
        fread(buffer, size % 1024, 1, file);
        SHA1_Update(&ctx, buffer, size % 1024);
    }
    unsigned char hash_data[SHA_DIGEST_LENGTH];
    SHA1_Final(hash_data, &ctx);
    char* hash = sha1ToString(hash_data);
    //2. Check if .statesync/objects/<hash> exists
    FILE* object = fopen(hash, "r");
    struct File_entry* entry = malloc(sizeof(struct File_entry));
    if(object != NULL) {
        //3b. Yes -> Load File_entry datas
        char* line = malloc(2000);
        line = fgets(line, 2000, object);
        stringToEntry(&entry, line);
    } else {
        //3b. No -> create it
        object = fopen(hash, "w");
        
    }
    //4. If file changed: update data
    return 1;
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
