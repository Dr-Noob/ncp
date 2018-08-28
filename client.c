#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define PORT 5000
#define BOOLEAN_FALSE 0
#define BOOLEAN_TRUE  1

struct stats_info {
	unsigned long int* bytes_transferred;
	int* all_bytes_transferred;
};

void *print_status(void* stats_struct) {
	unsigned long int last_bytes_transferred = 0;
	struct stats_info *stats = (struct stats_info*)stats_struct;
	int* n = stats->all_bytes_transferred;
	unsigned long int* bytes_transferred = stats->bytes_transferred;

	while(!*n) {
		last_bytes_transferred=*bytes_transferred;
		sleep(1);
		printf("wrote %fMiB at %fMbps\n",
		(*bytes_transferred-last_bytes_transferred)/(double)(1<<20),
		(double)8*((*bytes_transferred-last_bytes_transferred)/(double)(1000000)));
	}

	return NULL;
}

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
  int socketfd = -1;
  socklen_t length;
	static struct sockaddr_in serv_addr;

  if((socketfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <=0 )
  {
      printf("Invalid address\n");
      return EXIT_FAILURE;
  }

  printf("Trying connection...\n");

  if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
      perror("connect");
      return EXIT_FAILURE;
  }

  printf("Connected\n");
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
  bytes_read = read(STDIN_FILENO, buf, buf_size);

  while(bytes_read > 0) {
    write_all(socketfd,buf,bytes_read);
    bytes_transferred += bytes_read;

    memset(buf, 0, bytes_read);
    bytes_read = 0;

    bytes_read = read(STDIN_FILENO, buf, buf_size);
  }

	all_bytes_transferred = BOOLEAN_TRUE;
  gettimeofday(&t1, 0);

	if(pthread_join(status_thread, NULL)) {
		fprintf(stderr, "Error joining thread\n");
		return 2;
	}

  double e_time = (double)((t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec)/1000000;

  printf("EOF received, closing connection\n");
  printf("%fs\n",e_time);
  printf("%luB\n",bytes_transferred);
  printf("%fMbps, %fMiB/s\n",(double)8*((bytes_transferred/e_time)/(double)(1000000)),(bytes_transferred/e_time)/(double)(1<<20));

  if(bytes_read == -1)
    perror("read");

  if(close(socketfd) == -1)
    perror("close");
}
