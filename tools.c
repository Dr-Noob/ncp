#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tools.h"

/*
This function assumes that not writing 'bytes_to_write' bytes
on 'fd' is a error condition
*/

int write_all(int fd, char* buf,int bytes_to_write) {
	int bytes_write = 0;
	int ret = 0;
	do {
		ret = write(fd,buf+bytes_write,bytes_to_write-bytes_write);
		if(ret != -1)bytes_write+=ret;
		else {
			perror("write_all: write");
			return BOOLEAN_FALSE;
		}
	} while(bytes_write != bytes_to_write);

	return BOOLEAN_TRUE;
}

/*
This function assumes that not reading 'bytes_to_read' bytes
on 'fd' is a error condition
*/

int read_all(int fd, char* buf,int bytes_to_read) {
	int bytes_read = 0;
	int ret = 0;
	do {
		ret = read(fd,buf+bytes_read,bytes_to_read-bytes_read);
		if(ret == 0) {
			if(bytes_read != bytes_to_read) {
				fprintf(stderr, "read_all: Unexpected end of file\n");
				return BOOLEAN_FALSE;
			}
			return BOOLEAN_TRUE;
		}
		if(ret == -1) {
			perror("read_all: read");
			return BOOLEAN_FALSE;
		}
		bytes_read+=ret;
	} while(bytes_read != bytes_to_read);

	return BOOLEAN_TRUE;
}

char* bytes_to_hr(long int bytes, int base2_flag) {
  char *suffix[5];
	int division = 0;
	if(base2_flag) {
		division = 1024;
		suffix[0] = "B";
    suffix[1] = "KiB";
    suffix[2] = "MiB";
    suffix[3] = "GiB";
    suffix[4] = "TiB";
	}
	else {
		division = 1000;
    suffix[0] = "B";
    suffix[1] = "KB";
    suffix[2] = "MB";
    suffix[3] = "GB";
    suffix[4] = "TB";
	}

	int i = 0;
	double dblBytes = bytes;

	for (i = 0; (bytes / division) > 0 && i<4; i++, bytes /= division)
		dblBytes = bytes / (double)division;

	static char output[20];
	sprintf(output, "%.2f %s", dblBytes, suffix[i]);
	return output;
}
