#ifndef __PROGRESSBAR__
#define __PROGRESSBAR__

#define PB_STR "|||||||||||||||||||||||||||||||||||||||||||||"
#define PB_WIDTH 45

struct stats_info {
	long int* bytes_transferred;
	long int* file_size;
	int* all_bytes_transferred;
};

void *print_status(void* stats_struct);

#endif
