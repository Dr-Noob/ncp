#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "tools.h"
#include "progressbar.h"
#include "args.h"

int getFileToWrite(char* filename) {
  if(filename == NULL)
    return STDOUT_FILENO;

  if( access( filename, F_OK ) != -1 ) {
    fprintf(stderr,"ERROR: File '%s' exists, not replacing it\n",filename);
    return -1;
  }

  FILE *fp = fopen(filename, "w");
  if(fp == NULL) {
    perror("fopen");
    return -1;
  }

  return fileno(fp);
}

long int read_file_size(int fd) {
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

//filename: optional(if not passed, use STDOUT)
//port: optional(if not passed, use DEFAULT_PORT)
int server(int show_bar,char* filename,int port) {
  int file = getFileToWrite(filename);
  if(file == -1)
    return BOOLEAN_FALSE;

  int listenfd = -1;
  int socketfd = -1;
  socklen_t length;
	static struct sockaddr_in cli_addr;
	static struct sockaddr_in serv_addr;

  if((listenfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

  serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(port == INVALID_PORT)serv_addr.sin_port = htons(DEFAULT_PORT);
  else serv_addr.sin_port = htons(port);

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
	}

  int buf_size = 1<<20;
	int bytes_read = 0;
	char buf[buf_size];
  struct timeval t0,t1;
  struct stats_info stats;
  long int bytes_transferred = 0;
  int all_bytes_transferred = BOOLEAN_FALSE;
	int socketClosed = BOOLEAN_FALSE;

  fprintf(stderr,"Connection established\n");
  long file_size = read_file_size(socketfd);
  if(file_size == -1)
    return EXIT_FAILURE;

  stats.bytes_transferred = &bytes_transferred;
	stats.all_bytes_transferred = &all_bytes_transferred;
	stats.file_size = &file_size;

  gettimeofday(&t0, 0);

	pthread_t status_thread;
  if(show_bar) {
    if(pthread_create(&status_thread, NULL, &print_status, &stats)) {
  		fprintf(stderr, "Error creating thread\n");
  		return EXIT_FAILURE;
  	}
  }

  do {
    bytes_read = 0;
    memset(buf, 0, buf_size);

    bytes_read = read(socketfd, buf, buf_size);
		if(bytes_read > 0) {
      //Force closing socket if write to file fails
      if(!write_all(file,buf,MIN(bytes_read,buf_size)))
        socketClosed = BOOLEAN_TRUE;
      else
        bytes_transferred += bytes_read;
		}
    else if(bytes_read == -1)
		{
			perror("read");
			socketClosed = BOOLEAN_TRUE;
		}
		else if(bytes_read == 0)
		{
			socketClosed = BOOLEAN_TRUE;
		}
    else {
      fprintf(stderr,"Bug at line number %d in file %s\n", __LINE__, __FILE__);
    }
  } while(!socketClosed);

  all_bytes_transferred = BOOLEAN_TRUE;
  gettimeofday(&t1, 0);

  if(show_bar) {
    if(pthread_join(status_thread, NULL)) {
  		fprintf(stderr, "Error joining thread\n");
  		return EXIT_FAILURE;
  	}
  }
  else
    fprintf(stderr, "Connection closed\n");

  double e_time = (double)((t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec)/1000000;
  fprintf(stderr,"           Time = %.2f s\n",e_time);
  fprintf(stderr,"     Bytes read = %s\n",bytes_to_hr(bytes_transferred,BOOLEAN_FALSE));
  fprintf(stderr,"  Average Speed = %sps\n",bytes_to_hr((long)(bytes_transferred/e_time),BOOLEAN_TRUE));
	fprintf(stderr,"                  %s/s\n",bytes_to_hr((long)(bytes_transferred/e_time),BOOLEAN_FALSE));

	if(close(socketfd) == -1)
    perror("close");

	return EXIT_SUCCESS;
}
