#include "types.h"

void createDirectory(char* filename);

/**
 * Returns 0 on failure, 1 on success
 */
int addFile(char* filename);


/**
 * Read the file database and return a list of struct File_entry
 * @return List of struct File_entry* which must be freed is no longer needed.
 */
GList* readFile(char* filename);

/**
 * Write the database file from a list of file entries
 * @list: List of struct File_entry.
 */
void writeFile(char* filename, GList* list);