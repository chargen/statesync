#ifndef STATUS_H
#define STATUS_H

#include <glib.h>

/**
 * Recursively gets the files in the current directory.
 *
 * @list: Must be a double pointer to an empty GList, the function will
 *        allocat the File_entry objects and you must take care to free
 *        them when you free the list.
 */
void getFilesRecursive(char* filename, GList** changed_list, GList** untracked_list);

/**
 * This takes a list of File_entries and prints them in a formated
 * way to the standard output.
 */
void printStatus(GList* changed_list, GList* untracked_list);

#endif
