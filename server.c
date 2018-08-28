#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PORT 5000
#define BOOLEAN_FALSE 0
#define BOOLEAN_TRUE  1
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int write_all(int fd, char* buf,int bytes_read) {
	int bytes_write = 0;
	int ret = 0;
	do {
		ret = write(fd,buf+bytes_write,bytes_read-bytes_write);
		if(ret != -1)bytes_write+=ret;
		else perror("write_all: write");
	} while(bytes_write != bytes_read);

	return BOOLEAN_TRUE;
}

int main() {
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
	serv_addr.sin_port = htons(PORT);

  if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1) {
		perror("bind");
		return EXIT_FAILURE;
	}

  if(listen(listenfd,0) == -1) {
		perror("listen");
		return EXIT_FAILURE;
	}

  printf("Listening...\n");
  length = sizeof(cli_addr);
	if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
		perror("accept");
	}

  int buf_size = 1<<10;
	int bytes_read = 0;
	char buf[buf_size];
	int socketClosed = BOOLEAN_FALSE;

  printf("Socket accepted\n");
		
  do {
    bytes_read = 0;
    memset(buf, 0, buf_size);

    bytes_read = read(socketfd, buf, buf_size);
		if(bytes_read > 0) {
      write_all(STDERR_FILENO,buf,MIN(bytes_read,buf_size));
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
      printf("bug\n");
    }
  } while(!socketClosed);

  printf("Connection closed\n");

	if(close(socketfd) == -1)
    perror("close");

}
