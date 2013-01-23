#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "types.h"

#include "FileEntry.h"
#include "util.h"

#include "glib.h"

struct File_entry* getEntryFromFilename(char* filename) {
    struct File_entry* entry = malloc(sizeof(struct File_entry));
    struct stat st;
    stat(filename, &st);
    if(entry != NULL) {
        entry->file_name = strndup(filename, strlen(filename));
        entry->size = st.st_size;
        entry->mtime = st.st_mtime;
        entry->st_mode = st.st_mode;
        //Calculate the hash value of the file and copy to the
        char* hash = fileToHash(filename);
        snprintf(entry->hash, SHA_DIGEST_LENGTH*2+1, "%s", hash);
        free(hash);
        return entry;
    } else {
        return NULL;
    }
}

char* entryToString(struct File_entry* entry) {
    char* line = malloc(2000);
    snprintf(line, 2000, "%s,%d,%ld,%d,%s\n",
            entry->file_name,
            entry->size,
            entry->mtime,
            entry->st_mode,
            entry->hash);
    return line;
}

void stringToEntry(struct File_entry** entry_pointer, char* line) {
    struct File_entry* entry = *entry_pointer;
    gchar** tokens = g_strsplit(line, ",", 5);
    int i = 0;
    entry->file_name = tokens[i++];
    entry->size = atoi(tokens[i++]);
    entry->mtime = atol(tokens[i++]);
    entry->st_mode = atoi(tokens[i++]);
    if(tokens[i] != NULL) {
        //at this point the token contains '\n' as the last character, which snprintf will replace with '\0'
        snprintf(entry->hash, 2*SHA_DIGEST_LENGTH+1, "%s", tokens[i++]);
    }
}

int compareFileEntries(struct File_entry* entry1, struct File_entry* entry2) {
    if(strcmp(entry1->file_name, entry2->file_name) != 0) return 1;
    if(entry1->size != entry2->size) return 1;
    if(entry1->mtime != entry2->mtime) return 1;
    if(entry1->st_mode != entry2->st_mode) return 1;
    if(strcmp(entry1->hash, entry2->hash) != 0) return 1;    
    return 0;
}

void freeEntry(struct File_entry* entry) {
    free(entry->file_name);
    free(entry->hash);
    free(entry);
}