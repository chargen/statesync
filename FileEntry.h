#ifndef FILE_ENTRY_H
#define FILE_ENTRY_H

#include <sys/stat.h>
#include "types.h"

/**
 * Returns a struct File_entry that contains the information for the given
 * filename as it is currently on the disk. The entry needs
 * to be freed if no longer needed. The filename is duplicated internally
 * and can be freed as well.
 */
struct File_entry* getEntryFromFilename(char* filename);

/**
 * This function retrieves all the current entries for a filename from the
 * object store. The function will return NULL terminated array of
 * File_entry pointers. In the current implementation there will be two
 * entries where the second entry is possibly empty depending if the file
 * has been changed after it was added to the object store.
 */
struct File_entry** getObjectEntry(char* filename);

/**
 * Returns a comma separated string with the values of a struct File_entry*
 * The string must be freed when no longer needed.
 */
char* entryToString(struct File_entry* entry);

/**
 * This will convert a comma separated string with 5 values to a
 * struct File_entry*
 * A line will look like this:
 *     <filename>,<size>,<mtime>,<mode>,<40byte base 16 hash string>
 * @entry_pointer:
 *      A pointer to an allocated entry structure.
 * @line:
 *      The line to be converted to the entry.
 */
void stringToEntry(struct File_entry* entry_pointer, char* line);

/**
 * Compare two FileEntries and return 0 if they match, otherwise 1
 */
int compareFileEntries(struct File_entry* entry1, struct File_entry* entry2);

/**
 * Frees the entry and its nested pointers
 */
void freeEntry(struct File_entry* entry);

#endif
