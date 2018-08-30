#ifndef __TOOLS__
#define __TOOLS__

#define BOOLEAN_FALSE 0
#define BOOLEAN_TRUE  1
#define UNKNOWN_FILE_SIZE 0
#define DEFAULT_PORT 5000

int write_all(int fd, char* buf,int bytes_read);
char* bytes_to_hr(long int bytes, int base2_flag);

#endif
