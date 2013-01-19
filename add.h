#include "types.h"

void createDirectory(char* filename);
GList* readFile(char* filename);
void writeFile(char* filename, GList* list);
void entryToString(struct File_entry*, char** line);
void stringToEntry(struct File_entry** entry, char* line);

