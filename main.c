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
            GList* list = NULL;
            unsigned char buffer[1024];
            char* data = malloc(2000);
            //TODO: cread directly from pipe and not from stdin
            while((data = fgets(data, 2000, stdin)) != NULL) {
                if(strcmp(data, "0000\n") == 0) break;
                list = g_list_append(list, data);
                data = malloc(2000);
            }
            for(GList* current = list; current != NULL; current = g_list_next(current)) {
                char* data = (char*) current->data;
                struct File_entry* entry = malloc(sizeof(struct File_entry));
                stringToEntry(entry, data);
                char* receive_file_name = g_build_filename("received", entry->file_name, NULL);
                FILE* file = fopen(receive_file_name, "w");
                if(file == NULL) {
                    char* pathname = g_path_get_dirname(receive_file_name);
                    int result = g_mkdir_with_parents(pathname, 0755);
                    if(result == -1) {
                        perror("Could not create folder");
                        printf("%s\n", pathname);
                        exit(EXIT_FAILURE);
                    } else {
                        file = fopen(receive_file_name, "w");
                        if(file == NULL) {
                            perror("Error");   
                            printf("%s\n", receive_file_name);
                        }
                    }
                    free(pathname);
                }
                SHA_CTX ctx;
                SHA1_Init(&ctx);
                for(int i = 0; i<entry->size/1024; i++) {
                    fread(buffer, 1024, 1, stdin);
                    fwrite(buffer, 1024, 1, file);
                    SHA1_Update(&ctx, buffer, 1024);
                }
                if(entry->size % 1024 > 0) {
                    fread(buffer, 1, entry->size % 1024, stdin);
                    fwrite(buffer, 1, entry->size % 1024, file);
                    SHA1_Update(&ctx, buffer, entry->size % 1024);
                }
                unsigned char* hash_data = malloc(SHA_DIGEST_LENGTH);
                SHA1_Final(entry->hash, &ctx);
                char* hash_new = sha1ToString(hash_data);
                char* hash_source = malloc(2 * SHA_DIGEST_LENGTH+1);
                fread(hash_source, 2 * SHA_DIGEST_LENGTH+1, 1, stdin);
                fflush(file);
                fclose(file);
                if(strncmp(hash_source, hash_new, 2 * SHA_DIGEST_LENGTH + 1) != 0) {
                    printf("Error receiving file. HASH Mismatch!\n");
                }
                printf("# Received file: %s\n", receive_file_name);
                free(hash_data);
                free(hash_source);
                free(data);
                free(receive_file_name);
            }
            fflush(stdout);
            g_list_free(list);
        }
    }
}

