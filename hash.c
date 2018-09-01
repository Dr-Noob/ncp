#include <stdio.h>
#include "hash.h"

void *sha1sum(void* hs) {
  SHA_CTX ctx;
  SHA1_Init(&ctx);

  struct hash_struct *s = (struct hash_struct*)hs;

  while(!*s->eof) {
    //Wait to main thread to prepare the data
    if(sem_wait(s->thread_sem) == -1) {
      perror("sha1sum");
      return NULL;
    }

    if(*s->eof)
      break;

    //Data is ready, update hash
    SHA1_Update(&ctx, s->data, BUF_SIZE);

    //Tell main thread we're done
    if(sem_post(s->main_sem) == -1) {
      perror("sha1sum");
      return NULL;
    }
  }

  SHA1_Final(s->hash, &ctx);
  return NULL;
}
