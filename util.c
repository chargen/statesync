#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include "util.h"

char* sha1ToString(unsigned char* hash_data) {
    //each byte becomes two characters + 1 for terminating '\0'
    char* hash = (char*) malloc(2 * SHA_DIGEST_LENGTH+1);
    char* hash_head = hash;
    for(int i=0; i<SHA_DIGEST_LENGTH; i++) {
        snprintf(hash_head, 3, "%02hhx", hash_data[i]);
        hash_head += 2;
    }
    return hash;
}

char* filenameToHash(char* filename) {
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    int size = strlen(filename);
    unsigned char buffer[1024] = {0};
    if(size >= 1024) {
        for(int i=0; i < (size / 1024); i++) {
            SHA1_Update(&ctx, filename, 1024);
        }
    }
    if((size % 1024) > 0) {
        SHA1_Update(&ctx, filename, size % 1024);
    }
    unsigned char hash_data[SHA_DIGEST_LENGTH];
    SHA1_Final(hash_data, &ctx);
    return sha1ToString(hash_data);
}

char* fileToHash(char* filename) {
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    FILE * file = fopen(filename, "r");
    unsigned char buffer[1024] = {0};
    if(size <= 0) return NULL;
    if(size >= 1024) {
        for(int i=0; i < (size / 1024); i++) {
            fread(buffer, 1024, 1, file);
            SHA1_Update(&ctx, buffer, 1024);
        }
    }
    if((size % 1024) > 0) {
        fread(buffer, size % 1024, 1, file);
        SHA1_Update(&ctx, buffer, size % 1024);
    }
    unsigned char hash_data[SHA_DIGEST_LENGTH];
    SHA1_Final(hash_data, &ctx);
    return sha1ToString(hash_data);
}
