#include <stdlib.h>
#include <sys/types.h>

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
