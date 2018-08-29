#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include "args.h"
#include "tools.h"

#define PB_STR "|||||||||||||||||||||||||||||||||||||||||||||"
#define PB_WIDTH 45


/***

TODO
234234762 -> 234.23 MiB

***/

char* bytes_to_hr(int bytes) {
	return NULL;
}

struct stats_info {
	unsigned long int* bytes_transferred;
	int* all_bytes_transferred;
};

void *print_status(void* stats_struct) {
	struct stats_info *stats = (struct stats_info*)stats_struct;
	unsigned long int last_bytes_transferred = 0;
	unsigned long int* bytes_transferred = stats->bytes_transferred;
	double size = 4 * ((long)1 << 30);
	int printed_chars = 0;
	char* end_msg = "Connection closed";

	while(!*stats->all_bytes_transferred) {
		double perc = *bytes_transferred/size;
		last_bytes_transferred=*bytes_transferred;
		sleep(1);

		int val = (int) (perc * 100);
    int lpad = (int) (perc * PB_WIDTH);
    int rpad = PB_WIDTH - lpad;
    printed_chars = printf ("\r%3d%% [%.*s%*s] %.3fMbps", val, lpad, PB_STR, rpad, "",(double)8*((*bytes_transferred-last_bytes_transferred)/(double)(1000000)));
    fflush (stdout);
	}

	printf ("\r%s%*s\n",end_msg,(int)(printed_chars-strlen(end_msg)),"");
	return NULL;
}

int getFileToRead(char* filename) {
  if(filename == NULL)
    return STDIN_FILENO;

  if( access( filename, F_OK ) == -1 ) {
    fprintf(stderr,"ERROR: File '%s' doest not exists\n",filename);
    return -1;
  }

  FILE *fp = fopen(filename, "r");
  if(fp == NULL) {
    perror("fopen");
    return -1;
  }

  return fileno(fp);
}

//filename: optional(if not passed, use STDIN)
//addr: mandatory
//port: optinal(if not passed, use DEFAULT_PORT)
int client(char* filename, char* addr, int port) {
	int file = getFileToRead(filename);
  if(file == -1)
    return BOOLEAN_FALSE;

	assert(addr != NULL);
  int socketfd = -1;
  socklen_t length;
	static struct sockaddr_in serv_addr;

  if((socketfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

  serv_addr.sin_family = AF_INET;
	if(port == INVALID_PORT)serv_addr.sin_port = htons(DEFAULT_PORT);
  else serv_addr.sin_port = htons(port);

  if(inet_pton(AF_INET, addr, &serv_addr.sin_addr) <= 0 )
  {
      fprintf(stderr,"Invalid address\n");
      return EXIT_FAILURE;
  }

  fprintf(stderr,"Trying connection...\n");

  if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
      perror("connect");
      return EXIT_FAILURE;
  }

  fprintf(stderr,"Connected\n");
  int buf_size = 1<<10;
  int bytes_read = 0;
	struct stats_info stats;
  unsigned long int bytes_transferred = 0;
	int all_bytes_transferred = BOOLEAN_FALSE;
	char buf[buf_size];
  struct timeval t0,t1;

	stats.bytes_transferred = &bytes_transferred;
	stats.all_bytes_transferred = &all_bytes_transferred;
  gettimeofday(&t0, 0);

	pthread_t status_thread;
	if(pthread_create(&status_thread, NULL, &print_status, &stats)) {
		fprintf(stderr, "Error creating thread\n");
		return EXIT_FAILURE;
	}

  memset(buf, 0, buf_size);
  bytes_read = read(file, buf, buf_size);

  while(bytes_read > 0) {
    write_all(socketfd,buf,bytes_read);
    bytes_transferred += bytes_read;

    memset(buf, 0, bytes_read);
    bytes_read = 0;

    bytes_read = read(file, buf, buf_size);
  }

	all_bytes_transferred = BOOLEAN_TRUE;
  gettimeofday(&t1, 0);

	if(pthread_join(status_thread, NULL)) {
		fprintf(stderr, "Error joining thread\n");
		return 2;
	}

  double e_time = (double)((t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec)/1000000;
  fprintf(stderr,"           Time = %.2f s\n",e_time);
  fprintf(stderr,"     Bytes sent = %.2f MiB\n",bytes_transferred/(double)(1<<20));
  fprintf(stderr,"  Average Speed = %.2f Mbps\n",(double)8*((bytes_transferred/e_time)/(double)(1000000)));
	fprintf(stderr,"                  %.2f MiB/s\n",(bytes_transferred/e_time)/(double)(1<<20));

  if(bytes_read == -1)
    perror("read");

  if(close(socketfd) == -1)
    perror("close");

	return EXIT_SUCCESS;
}
