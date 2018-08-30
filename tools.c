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
