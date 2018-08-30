#ifndef __ARGS__
#define __ARGS__

typedef int MODE;

#define MODE_EMPTY  0
#define MODE_CLIENT 1
#define MODE_SERVER 2

#define INVALID_PORT -1

int parseArgs(int argc, char* argv[]);

int run_server();
int run_client();
int show_help();

int show_bar();           /*** BOTH ***/
int get_port();           /*** BOTH ***/
char* get_filename();     /*** BOTH ***/
char* get_addr();         /*** CLIENT ***/

void print_args();

#endif
