#include "msg.h"
#include "tools.h"
#include <stdlib.h>
#include <stdio.h>

/*
*-----------------*-------*--------*
| LAST_CHUNK_FLAG | SIZE  |  DATA  |
*-----------------*-------*--------*

LAST_CHUNK_FLAG(4B): True if this chunk msg is the
                     last one, false in other case
SIZE(4B):            Size in bytes of the following
                     data
DATA(?):             Its size is specified in 'SIZE'
                     field. 'DATA' contains a chunk
                     of the file
*/

//Returns true if msg was sent successfully
int send_msg_chunk(int socket, int last_chunk_flag, char* content, int content_size) {
  if(!write_all(socket,(char *)&last_chunk_flag,sizeof(char)))
    return BOOLEAN_FALSE;
  if(!write_all(socket,(char *)&content_size,sizeof(int)))
    return BOOLEAN_FALSE;
  return write_all(socket,content,content_size);
}

//Returns true if msg was recieved successfully
int read_msg_chunk(int socket, int* last_chunk_flag, char* content, int* content_size) {
  if(!read_all(socket,(char *)last_chunk_flag,sizeof(char)))
    return BOOLEAN_FALSE;

  if(!read_all(socket,(char *)content_size,sizeof(int)))
    return BOOLEAN_FALSE;

  return read_all(socket,content,*content_size);
}
