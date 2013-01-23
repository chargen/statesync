#include <sys/stat.h>
#include "types.h"

/**
 * Returns a struct File_entry for the given filename, which needs
 * to be freed if no longer needed. The filename is duplicated internally
 * and can be freed.
 * 
 */
struct File_entry* getEntryFromFilename(char* filename);

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
 */
void stringToEntry(struct File_entry** entry_pointer, char* line);

/**
 * Compare two FileEntries and return 0 if they match, otherwise 1
 */
int compareFileEntries(struct File_entry* entry1, struct File_entry* entry2);

/**
 * Frees the entry and its nested pointers
 */
void freeEntry(struct File_entry* entry);