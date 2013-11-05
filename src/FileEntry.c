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

struct File_entry** getObjectEntry(char* filename) {
    //1. Hash filename
    char* basename_hash = filenameToHash(filename);
    //2. Check if .statesync/objects/<hash> exists
    char* object_filename =
        g_build_filename(".statesync", "objects", basename_hash, NULL);
    FILE* object = fopen(object_filename, "r");
    if(object != NULL) {
        //3. Yes -> Load File_entry datas
        char line1[2000] = {0};
        char line2[2000] = {0};
        char* exists2;
        char* new_line;
        if(fgets(line1, 2000, object) == NULL) {
            return NULL; //this is actually an error
        }
        exists2 = fgets(line2, 2000, object);
        //4. We allocate a static array of size 3 to store a NULL
        //   terminated list of File_entry objects and then we place the
        //   File_entries for the two lines for the object file in it.
        int num_entries = 3;
        //This is dangerous and wrong, use double pointer instead
        struct File_entry** object_entries = malloc(num_entries*sizeof(void *));
        struct File_entry* entry1 = malloc(sizeof(struct File_entry));
        struct File_entry** entry_head = object_entries;
        *object_entries = entry1;
        stringToEntry(*entry_head, line1);
        entry_head++;
        if(exists2 != NULL) {
            *entry_head = malloc(sizeof(struct File_entry));
            stringToEntry(*entry_head, line2);
            entry_head++;
        } else {
            *entry_head = NULL;
            entry_head++;
        }
        *entry_head = NULL;
        return object_entries;
    }
    return NULL;
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

void stringToEntry(struct File_entry* entry, char* line) {
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
    if((entry1 == NULL) ^ (entry2 == NULL)) {
        //if the either but not both entry pointer is NULL then return
        return -1;
    }
    if(entry1 == entry2) return 0;
    if(entry1->file_name != NULL && entry2->file_name != NULL &&
        strcmp(entry1->file_name, entry2->file_name) != 0) return 1;
    if(entry1->size != entry2->size) return 1;
    if(entry1->mtime != entry2->mtime) return 1;
    if(entry1->st_mode != entry2->st_mode) return 1;
    if(entry1->hash != NULL && entry2->hash != NULL &&
        strcmp(entry1->hash, entry2->hash) != 0) return 1;
    return 0;
}

void freeEntry(struct File_entry* entry) {
    free(entry->file_name);
    free(entry->object_hash);
    free(entry->hash);
    free(entry);
}
