#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <openssl/sha.h>
#include <glib.h>

#include "status.h"
#include "FileEntry.h"
#include "util.h"

void perform_send(char *hostname) {
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if(pid == 0) {
        //child
        close(fd[1]); //close output
        //read from fd[0]
        dup2(fd[0], 0);
        execl("/usr/bin/ssh", "/usr/bin/ssh", hostname, "./Documents/Code/statesync/statesync", "receive", NULL);
    } else {
        //parent
        close(fd[0]); //close input
        //write to fd[1]
        GList* changed_list = NULL;
        GList* untracked_list = NULL;
        getFilesRecursive(".", &changed_list, &untracked_list);
        printf("# Sending data items: %d\n", g_list_length(changed_list) + g_list_length(untracked_list));
        dup2(fd[1], 1);
        fflush(stdout);
        char* line;
        //TODO: Add sending support for untracked files
        for(GList* current = changed_list; current != NULL; current = g_list_next(current)) {
            if(current->data == NULL) break;
            struct File_entry* entry = (struct File_entry*) current->data;
            line = entryToString(entry);
            fwrite(line, 1, strlen(line), stdout);
            free(line);
        }
        fwrite("0000\n", 1, 5, stdout);
        //start sending files
        for(GList* current = changed_list; current != NULL; current = g_list_next(current)) {
            struct File_entry* entry = (struct File_entry*) current->data;
            char* filename = entry->file_name;
            FILE* file = fopen(filename, "r");
            unsigned char buffer[1024];
            SHA_CTX ctx;
            SHA1_Init(&ctx);
            for(int i = 0; i<entry->size / 1024;i++) {
                fread(buffer, 1024, 1, file);
                fwrite(buffer, 1024, 1, stdout);
                SHA1_Update(&ctx, buffer, 1024);
            }
            if(entry->size % 1024 > 0) {
                fread(buffer, entry->size % 1024, 1, file);
                fwrite(buffer, entry->size % 1024, 1, stdout);
                SHA1_Update(&ctx, buffer, entry->size % 1024);
            }
            unsigned char* hash_data = malloc(SHA_DIGEST_LENGTH);
            SHA1_Final(hash_data, &ctx);
            char* hash = sha1ToString(hash_data);
            free(hash_data);
            fwrite(hash, 41, 1, stdout);
        }
        fflush(stdout);
        //In order to close the pipe's output stream we need to close both file descriptors
        close(fd[1]);
        fclose(stdout);
        int pid_status;
        waitpid(pid, &pid_status, 0);
    }
}
