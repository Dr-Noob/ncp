#ifndef __MSG__
#define __MSG__

int send_msg_chunk(int socket, int last_chunk_flag, char* content, int content_size);
int read_msg_chunk(int socket, int* last_chunk_flag, char* content, int* bytes_read);

#endif
