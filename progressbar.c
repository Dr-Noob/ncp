#include <time.h>
#include <stdio.h>
#include <string.h>
#include "progressbar.h"
#include "tools.h"

void *print_status(void* stats_struct) {
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;

	struct stats_info *stats = (struct stats_info*)stats_struct;
	long int last_bytes_transferred = 0;
	long int* bytes_transferred = stats->bytes_transferred;
	long int* file_size = stats->file_size;

	int printed_chars = 0;
	char* end_msg = "Connection closed";

	while(!*stats->all_bytes_transferred) {
		double perc = *bytes_transferred / (double)*file_size;
		last_bytes_transferred=*bytes_transferred;
    nanosleep(&ts, NULL);

		int val = (int) (perc * 100);
    int lpad = (int) (perc * PB_WIDTH);
    int rpad = PB_WIDTH - lpad;
    printed_chars = MAX(printed_chars,
			                  fprintf(stderr,"\r%3d%% [%.*s%*s] %sps", val, lpad, PB_STR, rpad, "",bytes_to_hr(*bytes_transferred-last_bytes_transferred,BOOLEAN_TRUE)));
    fflush (stdout);
	}

	fprintf(stderr,"\r%s%*s\n",end_msg,(int)(printed_chars-strlen(end_msg)),"");
	return NULL;
}
