#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>

char* sha1ToString(unsigned char* hash_data) {
    //each byte becomes two characters + 1 for terminating '\0'
    char* hash = malloc(2 * SHA_DIGEST_LENGTH+1);
    for(int i=0; i<(2* SHA_DIGEST_LENGTH); i++) {
        snprintf(hash, 2, "%02hhx", hash_data[i]);
    }
    hash[2*SHA_DIGEST_LENGTH] = '\0';
    return hash;
}
