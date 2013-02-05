#ifndef STATUS_H
#define STATUS_H

#include <glib.h>

/**
 * Recursively gets the files in the current directory.
 */
void getFilesRecursive(char* filename, GList** list);

/**
 * 
 */
void printStatus(GList* list);

#endif
