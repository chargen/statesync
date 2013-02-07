#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <error.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#include <openssl/sha.h>

#include <glib.h>
#include "add.h"
#include "types.h"
#include "util.h"
#include "status.h"
#include "send.h"
#include "receive.h"
#include "FileEntry.h"

int main(int argc, char** argv) {
    createDirectory(".");
    if(argc >= 2) {
        if(strcmp(argv[1], "status") == 0) {
            GList* changed_list = NULL;
            GList* untracked_list = NULL;
            getFilesRecursive(".", &changed_list, &untracked_list);
            printStatus(changed_list, untracked_list);
        } else if(strcmp(argv[1], "add") == 0) {
            //DO THE ADDING
            if(argc < 3) {
                printf("# No files to add were specified\n");
                printf("#\n");
            } else {
                GList* list = NULL;
                for(int i=2; i<argc; i++) {
                    list = g_list_append(list, argv[i]);
                }
                for(GList* current = list; current != NULL; current = g_list_next(current)) {
                    char* filename = (char*) current->data;
                    int result = addFile(filename);
                    if(result != 1) {
                        printf("# Error adding file");
                    }
                }
            }
        } else if(strcmp(argv[1], "send") == 0) {
            if(argc < 3) {
                printf("# No host name given. Format: %s hostname\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            perform_send(argv[2]);
        } else if(strcmp(argv[1], "receive") == 0) {
            perform_receive();
        }
    }
}
