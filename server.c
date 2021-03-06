#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "args.h"
#include "tools.h"
#include "progressbar.h"
#include "hash.h"
#include "msg.h"

#define BUF_SIZE 1<<15

int getFileToWrite(char* filename,mode_t file_mode) {
  if(filename == NULL)
    return STDOUT_FILENO;

  if( access( filename, F_OK ) != -1 ) {
    fprintf(stderr,"ERROR: File '%s' exists, not replacing it\n",filename);
    return -1;
  }

  int file = open(filename,file_mode);
  if(file == -1) {
    perror("open");
    return -1;
  }

  return file;
}

long int read_longint(int fd) {
  long int ret = 0;
  int bytes_read = read(fd, (char*)&ret, sizeof(long));

  if(bytes_read == -1) {
    perror("read");
    return -1;
  }
  else if(bytes_read != sizeof(long)) {
    fprintf(stderr,"ERROR: Failed to read file size(read %d bytes, expected %li)\n",bytes_read,sizeof(long));
    return -1;
  }

  return ret;
}

int read_hash(int fd, unsigned char* hash[SHA_DIGEST_LENGTH]) {
  int bytes_read = read(fd, hash, SHA_DIGEST_LENGTH);

  if(bytes_read == -1) {
    perror("read");
    return EXIT_FAILURE;
  }
  else if(bytes_read != SHA_DIGEST_LENGTH) {
    fprintf(stderr,"ERROR: Failed to read file hash(read %d bytes, expected %d)\n",bytes_read,SHA_DIGEST_LENGTH);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

//show_bar: flag that tells if show_bar thread should be spawned or not
//filename: optional(if not passed, use STDOUT)
//port: optional(if not passed, use DEFAULT_PORT)
int server(int show_bar,char* filename,int port) {
  socklen_t length;
  int listenfd = -1;
  int socketfd = -1;
	static struct sockaddr_in cli_addr;
	static struct sockaddr_in serv_addr;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(port == INVALID_PORT)serv_addr.sin_port = htons(DEFAULT_PORT);
  else serv_addr.sin_port = htons(port);

  if((listenfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

  if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1) {
		perror("bind");
		return EXIT_FAILURE;
	}

  if(listen(listenfd,0) == -1) {
		perror("listen");
		return EXIT_FAILURE;
	}

  fprintf(stderr,"Listening on port %d...\n",port == INVALID_PORT ? DEFAULT_PORT : port);
  length = sizeof(cli_addr);
	if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
		perror("accept");
    return EXIT_FAILURE;
	}

  /*** CONECTION ESTABLISHED ***/
  fprintf(stderr,"Connection established\n");
	int bytes_read = 0;
	char buf[BUF_SIZE];
  struct timeval t0,t1;
  struct stats_info stats;
  struct hash_struct hash;
  sem_t thread_sem;
	sem_t main_sem;
  long int bytes_transferred = 0;
  int all_bytes_transferred = BOOLEAN_FALSE;
	int closeSocket = BOOLEAN_FALSE;
  int last_chunk = BOOLEAN_FALSE;
  unsigned char received_hash[SHA_DIGEST_LENGTH];

  if(sem_init(&thread_sem,0,0) == -1) {
    perror("server");
    return EXIT_FAILURE;
  }

  if(sem_init(&main_sem,0,0) == -1) {
    perror("server");
    return EXIT_FAILURE;
  }

  long file_size;
  mode_t file_mode;
  if((file_size = read_longint(socketfd)) == -1)
    return EXIT_FAILURE;
  if(file_size == UNKNOWN_FILE_SIZE)
    show_bar = BOOLEAN_FALSE;

  if((file_mode = (mode_t)read_longint(socketfd)) == 0)
    return EXIT_FAILURE;

  int file = getFileToWrite(filename,file_mode);
  if(file == -1)
    return BOOLEAN_FALSE;

  fprintf(stderr,"File size is %s\n",bytes_to_hr(file_size,BOOLEAN_FALSE));
  stats.bytes_transferred = &bytes_transferred;
	stats.all_bytes_transferred = &all_bytes_transferred;
	stats.file_size = &file_size;

  hash.thread_sem = &thread_sem;
  hash.main_sem = &main_sem;
  hash.data = buf;
  hash.data_size = &bytes_read;
  hash.eof = &all_bytes_transferred;

  gettimeofday(&t0, 0);

	pthread_t status_thread;
  if(show_bar) {
    if((errno = pthread_create(&status_thread, NULL, &print_status, &stats)) != 0) {
  		perror("pthread_create");
  		return EXIT_FAILURE;
  	}
  }

  pthread_t hash_thread;
  if((errno = pthread_create(&hash_thread, NULL, sha1sum, &hash)) != 0) {
		perror("pthread_create");
		return EXIT_FAILURE;
	}

  do {
		if(read_msg_chunk(socketfd, &last_chunk, buf, &bytes_read)) {
      //Wake up thread, data is ready
      if(sem_post(&thread_sem) == -1) {
        perror("server");
        return EXIT_FAILURE;
      }

      //Force closing socket if write to file fails
      if(!write_all(file,buf,MIN(bytes_read,BUF_SIZE)))
        closeSocket = BOOLEAN_TRUE;
      else
        bytes_transferred += bytes_read;

      //Wait thread to have calculated the partial hash
      if(sem_wait(&main_sem) == -1) {
        perror("server");
        return EXIT_FAILURE;
      }
		}
    else
      closeSocket = BOOLEAN_TRUE;
  } while(!last_chunk && !closeSocket);

  gettimeofday(&t1, 0);
  all_bytes_transferred = BOOLEAN_TRUE;
  //Wake up thread, file read completely
  if(sem_post(&thread_sem) == -1) {
    perror("client");
    return EXIT_FAILURE;
  }

  if((errno = pthread_join(hash_thread, NULL)) != 0) {
    perror("pthread_join");
    return EXIT_FAILURE;
  }

  if(!closeSocket) {
    /*** JUST CONTINUE IF NO ERROR HAPPENED ***/
    read_hash(socketfd,(unsigned char**)&received_hash);

    /*** CHECK HASH MATCHES ***/
    if(memcmp(received_hash,hash.hash,SHA_DIGEST_LENGTH) != 0) {
      fprintf(stderr, "FATAL ERROR: Hash does not match(calculated vs received)\n'");

      for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        fprintf(stderr,"%x", hash.hash[i]);

      fprintf(stderr, "' vs '");

      for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        fprintf(stderr,"%x", received_hash[i]);

      fprintf(stderr, "'\n");
    }
  }

  if(show_bar) {
    if((errno = pthread_join(status_thread, NULL)) != 0) {
  		perror("pthread_join");
  		return EXIT_FAILURE;
  	}
  }
  else
    fprintf(stderr, "Connection closed\n");

	if(close(socketfd) == -1)
    perror("close");

  if(close(file) == -1)
    perror("close");

  double e_time = (double)((t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec)/1000000;
  fprintf(stderr,"           Time = %.2f s\n",e_time);
  fprintf(stderr,"     Bytes read = %s\n",bytes_to_hr(bytes_transferred,BOOLEAN_FALSE));
  fprintf(stderr,"  Average Speed = %sps\n",bytes_to_hr((long)(bytes_transferred/e_time),BOOLEAN_TRUE));
	fprintf(stderr,"                  %s/s\n",bytes_to_hr((long)(bytes_transferred/e_time),BOOLEAN_FALSE));

	return EXIT_SUCCESS;
}
