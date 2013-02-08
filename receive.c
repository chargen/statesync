#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>
#include <glib.h>

#include "FileEntry.h"
#include "util.h"

void perform_receive() {
    GList* list = NULL;
    unsigned char buffer[1024];
    char* data = malloc(2000);
    //TODO: read directly from pipe and not from stdin
    while(fgets(data, 2000, stdin) != NULL) {
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
        SHA1_Final(hash_data, &ctx);
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
