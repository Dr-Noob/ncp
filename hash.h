#ifndef __HASH__
#define __HASH__

#include <openssl/sha.h>
#include <semaphore.h>
#include "tools.h"

struct hash_struct {
	sem_t *thread_sem;
	sem_t *main_sem;
	int* eof;
	int* data_size;
  char* data;
	unsigned char hash[SHA_DIGEST_LENGTH];
};

void *sha1sum(void* hs);

#endif
