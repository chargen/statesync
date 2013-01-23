#include <openssl/sha.h>

/**
 * Converts the binary data of a SHA digest to a hex string which needs to
 * be freed when no longer needed.
 */
char* sha1ToString(unsigned char* hash_data);

/**
 * Hashes the filename of a file into a hex string which needs to be
 * freed when no longer needed.
 */
char* filenameToHash(char* filename);


/**
 * Hashes the contents of a file into a hex string which needs to be
 * freed when no longer needed;
 */
char* fileToHash(char* filename);