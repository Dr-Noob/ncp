#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "args.h"
#include "tools.h"
#include "progressbar.h"
#include "hash.h"
#define BUF_SIZE 1<<15

int getFileToRead(char* filename) {
  if(filename == NULL)
    return STDIN_FILENO;

  if( access( filename, F_OK ) == -1 ) {
    fprintf(stderr,"ERROR: File '%s' doest not exists\n",filename);
    return -1;
  }

  int file = open(filename,O_CREAT | O_RDONLY);
  if(file == -1) {
    perror("open");
    return -1;
  }

  return file;
}

int send_file_size(int fd, long int size) {
	return write_all(fd,(char*)&size,sizeof(long));
}

int send_hash(int fd,unsigned char hash[SHA_DIGEST_LENGTH]) {
  return write_all(fd,(char *)hash,SHA_DIGEST_LENGTH);
}

long int getFileSize(char* filename) {
	struct stat st;
  if(filename == NULL)
    return UNKNOWN_FILE_SIZE;
	if(stat(filename, &st) == -1) {
		if(errno == EFAULT) {
			//File does not exist
			return UNKNOWN_FILE_SIZE;
		}
		perror("getFileSize");
		return -1;
	}
	return st.st_size;
}

struct sockaddr_in* get_server_struct(char* addr, int port) {
  struct sockaddr_in* serv_addr = malloc(sizeof(struct sockaddr_in));
  memset(serv_addr,0,sizeof(struct sockaddr_in));
  serv_addr->sin_family = AF_INET;
  if(port == INVALID_PORT)serv_addr->sin_port = htons(DEFAULT_PORT);
  else serv_addr->sin_port = htons(port);

  if(inet_pton(AF_INET, addr, &serv_addr->sin_addr) > 0 )
    return serv_addr;

  //If inet_pton fails, we'll issue a DNS search
  struct addrinfo hints, *res;
  char addrstr[INET_ADDRSTRLEN];
  int err = 0;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  err = getaddrinfo (addr, NULL, &hints, &res);
  if(err != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return NULL;
  }

  while (res != NULL) {
    inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, INET_ADDRSTRLEN);

    if(res->ai_family == AF_INET) {
      //Is a IPv4 address
      inet_ntop (res->ai_family, &((struct sockaddr_in *) res->ai_addr)->sin_addr, addrstr, INET_ADDRSTRLEN);

      if(inet_pton(AF_INET, addrstr, &serv_addr->sin_addr) <= 0 ) {
        perror("inet_pton");
        return NULL;
      }
      else
        return serv_addr;
    }
    res = res->ai_next;
  }

  fprintf(stderr, "getaddrinfo: No DNS name for %s\n", addr);
  return NULL;
}

//show_bar: flag that tells if show_bar thread should be spawned or not
//filename: optional(if not passed, use STDIN)
//addr: mandatory
//port: optinal(if not passed, use DEFAULT_PORT)
int client(int show_bar,char* filename, char* addr, int port) {
	int file = getFileToRead(filename);
  if(file == -1)
    return BOOLEAN_FALSE;

  long int file_size = getFileSize(filename);
	if(file_size == -1)
		return EXIT_FAILURE;
  else if(file_size == 0) {
    fprintf(stderr,"ERROR: File '%s' is empty, will not be sent\n",filename);
    return EXIT_FAILURE;
  }

  //Ignore SIGPIPE, we'll treat them later if necessary(in write_all)
  signal(SIGPIPE, SIG_IGN);
	assert(addr != NULL);

  socklen_t length;
  struct sockaddr_in* serv_addr;
  char* ip = NULL;
  int socketfd = -1;

  if((socketfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

  //serv_addr will not be freed to simplify code
  if((serv_addr = get_server_struct(addr,port)) == NULL)
    return EXIT_FAILURE;

  fprintf(stderr,"Trying connection to %s:%d...\n",addr,port == INVALID_PORT ? DEFAULT_PORT : port);

  if (connect(socketfd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)) < 0)
  {
      perror("connect");
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
  int status = BOOLEAN_TRUE;

  if(sem_init(&thread_sem,0,0) == -1) {
    perror("client");
    return EXIT_FAILURE;
  }

  if(sem_init(&main_sem,0,0) == -1) {
    perror("client");
    return EXIT_FAILURE;
  }

	stats.bytes_transferred = &bytes_transferred;
	stats.all_bytes_transferred = &all_bytes_transferred;
	stats.file_size = &file_size;

  hash.thread_sem = &thread_sem;
  hash.main_sem = &main_sem;
  hash.data = buf;
  hash.data_size = &bytes_read;
  hash.eof = &all_bytes_transferred;

	if(!send_file_size(socketfd,file_size))
    return EXIT_FAILURE;
  gettimeofday(&t0, 0);

  pthread_t status_thread;
  if(show_bar) {
    if(pthread_create(&status_thread, NULL, &print_status, &stats)) {
  		fprintf(stderr, "Error creating thread\n");
  		return EXIT_FAILURE;
  	}
  }

  pthread_t hash_thread;
  if(pthread_create(&hash_thread, NULL, sha1sum, &hash)) {
		fprintf(stderr, "Error creating thread\n");
		return EXIT_FAILURE;
	}

  bytes_read = read(file, buf, BUF_SIZE);
  //Wake up thread, data is ready
  if(sem_post(&thread_sem) == -1) {
    perror("client");
    return EXIT_FAILURE;
  }

  while(status && bytes_read > 0) {
    status = write_all(socketfd,buf,bytes_read);

    if(status) {
      //read if and only if write succedded
      bytes_transferred += bytes_read;

      //Wait thread to have calculated the partial hash
      if(sem_wait(&main_sem) == -1) {
        perror("client");
        return EXIT_FAILURE;
      }
      bytes_read = read(file, buf, BUF_SIZE);

      if(bytes_read > 0) {
        //Wake up thread, data is ready
        if(sem_post(&thread_sem) == -1) {
          perror("client");
          return EXIT_FAILURE;
        }
      }
    }
  }

  if(bytes_read == -1) {
    perror("read");
    return EXIT_FAILURE;
  }

  gettimeofday(&t1, 0);
	all_bytes_transferred = BOOLEAN_TRUE;
  //Wake up thread, file read completely
  if(sem_post(&thread_sem) == -1) {
    perror("client");
    return EXIT_FAILURE;
  }

  if(pthread_join(hash_thread, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return EXIT_FAILURE;
  }

  if(close(file) == -1)
    perror("close");

  /*** CLOSE DATA SOCKET ***/
  if(close(socketfd) == -1)
    perror("close");

  if(show_bar) {
    if(pthread_join(status_thread, NULL)) {
  		fprintf(stderr, "Error joining thread\n");
  		return EXIT_FAILURE;
  	}
  }
  else
    fprintf(stderr, "Connection closed\n");

  /*** JUST OPEN A NEW CONNECTION IF FIRST ONE ENDED SUCCESSFULLY ***/
  if(status) {
    /*** OPEN NEW SOCKET TO SEND HASH ***/
    if((socketfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
      perror("socket");
      return EXIT_FAILURE;
    }

    if (connect(socketfd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect");
        return EXIT_FAILURE;
    }

    if(!send_hash(socketfd,hash.hash))
      return EXIT_FAILURE;

    if(close(socketfd) == -1)
      perror("close");
  }

  double e_time = (double)((t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec)/1000000;
  fprintf(stderr,"           Time = %.2f s\n",e_time);
  fprintf(stderr,"     Bytes sent = %s\n",bytes_to_hr(bytes_transferred,BOOLEAN_FALSE));
  fprintf(stderr,"  Average Speed = %sps\n",bytes_to_hr((long)(bytes_transferred/e_time),BOOLEAN_TRUE));
	fprintf(stderr,"                  %s/s\n",bytes_to_hr((long)(bytes_transferred/e_time),BOOLEAN_FALSE));

	return EXIT_SUCCESS;
}
