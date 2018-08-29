#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tools.h"

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
