#ifndef __ARGS__
#define __ARGS__

int parseArgs(int argc, char* argv[]);

int run_server();
int run_client();
int show_help();

int get_port();           /*** BOTH ***/
char* get_filename(); /*** BOTH ***/
char* get_addr();         /*** CLIENT ***/

#endif
