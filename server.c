#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tools.h"
#include "args.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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

//filename: optional(if not passed, use STDOUT)
//port: optional(if not passed, use DEFAULT_PORT)
int server(char* filename,int port) {
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

  fprintf(stderr,"Listening...\n");
  length = sizeof(cli_addr);
	if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
		perror("accept");
	}

  int buf_size = 1<<10;
	int bytes_read = 0;
	char buf[buf_size];
	int socketClosed = BOOLEAN_FALSE;

  fprintf(stderr,"Socket accepted\n");

  do {
    bytes_read = 0;
    memset(buf, 0, buf_size);

    bytes_read = read(socketfd, buf, buf_size);
		if(bytes_read > 0) {
      write_all(file,buf,MIN(bytes_read,buf_size));
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
      fprintf(stderr,"bug\n");
    }
  } while(!socketClosed);

  fprintf(stderr,"Connection closed\n");

	if(close(socketfd) == -1)
    perror("close");

	return EXIT_SUCCESS;
}
