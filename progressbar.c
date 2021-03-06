#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "progressbar.h"
#include "tools.h"

/***
_NOTE_

Speed expressed as mbps could show unexpected results:
On the server, will show a reasonable speed while on client
will show speed with no decimal part(eg 5.0 Mbps, 4.0 Mbps...etc)

This happen if BUF_SIZE is big, because client will take so much
time in a read syscall, so in 0.5s(while this thread is sleeping)
there will be few syscall completed(maybe zero!), so if in 0.5s
there are four syscalls and BUF_SIZE is 1MiB, speed will show as
4 Mbps

However, this could not happen on server because it will issue
a read call which will not wait to read BUF_SIZE, instead, will
end before this happens, because won't see more data on the socket
to read, and therefore, will end reading less bytes than BUF_SIZE
***/

void *print_status(void* stats_struct) {
	struct stats_info *stats = (struct stats_info*)stats_struct;
	long int last_bytes_transferred = 0;
	long int* bytes_transferred = stats->bytes_transferred;
	long int* file_size = stats->file_size;

	assert(*file_size > 0);

	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;

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
