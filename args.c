#include <getopt.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"
#include "tools.h"

#define ARG_STR_LISTEN    "listen"
#define ARG_STR_OUT       "out"
#define ARG_STR_FILE      "file"
#define ARG_STR_ADDR      "addr"
#define ARG_STR_PORT      "port"
#define ARG_STR_HELP      "help"
#define ARG_CHAR_LISTEN   'l'
#define ARG_CHAR_OUT      'o'
#define ARG_CHAR_FILE     'f'
#define ARG_CHAR_ADDR     'a'
#define ARG_CHAR_PORT     'p'
#define ARG_CHAR_HELP     'h'

#define MAX_VALID_PORT  2<<15
#define MIN_VALID_PORT  0

static struct args_struct args;
struct args_struct {
  int help_flag;
  int listen_flag;
  int port;
  char* addr;
  char* file;
  MODE mode;
};

int check_options() {
  if(args.listen_flag) {
    args.mode = MODE_SERVER;
    if(args.addr != 0) {
      fprintf(stderr,"ERROR: Invalid args\n");
      args.help_flag = BOOLEAN_TRUE;
      return BOOLEAN_FALSE;
    }
  } else if(args.addr != NULL) {
    args.mode = MODE_CLIENT;

    //Check ip is valid
    struct sockaddr_in sa;
    if(inet_pton(AF_INET, args.addr, &(sa.sin_addr)) == 0) {
      fprintf(stderr,"ERROR: IP address('%s') is invalid\n",args.addr);
      return BOOLEAN_FALSE;
    }
  } else if(!args.help_flag){
    fprintf(stderr,"ERROR: None address specified nor listen option\n");
    args.help_flag = BOOLEAN_TRUE;
    return BOOLEAN_FALSE;
  }

  if(args.port != INVALID_PORT) {
    //Check port is valid
    if (args.port <= MIN_VALID_PORT || args.port >= MAX_VALID_PORT) {
      fprintf(stderr,"ERROR: Port('%d') is invalid\n",args.port);
      return BOOLEAN_FALSE;
    }
  }

  return BOOLEAN_TRUE;
}

int parseArgs(int argc, char* argv[]) {
  int c;
  int digit_optind = 0;
  int option_index = 0;
  opterr = 0;

  args.help_flag = BOOLEAN_FALSE;
  args.listen_flag = BOOLEAN_FALSE;
  args.port = INVALID_PORT;
  args.addr = NULL;
  args.file = NULL;
  args.mode = MODE_EMPTY;

  static struct option long_options[] = {
      {ARG_STR_HELP,     no_argument,         0, ARG_CHAR_HELP   },
      {ARG_STR_LISTEN,   no_argument,         0, ARG_CHAR_LISTEN },
      {ARG_STR_OUT,      required_argument,   0, ARG_CHAR_OUT    },
      {ARG_STR_FILE,     required_argument,   0, ARG_CHAR_FILE   },
      {ARG_STR_ADDR,     required_argument,   0, ARG_CHAR_ADDR   },
      {ARG_STR_PORT,     required_argument,   0, ARG_CHAR_PORT   },
      {0, 0, 0, 0}
  };

  c = getopt_long(argc, argv,"",long_options, &option_index);

  while (c != -1) {
    if(c == ARG_CHAR_LISTEN) {
      args.listen_flag = BOOLEAN_TRUE;
    }
    else if(c == ARG_CHAR_OUT || c == ARG_CHAR_FILE) {
      if(args.file != NULL) {
        fprintf(stderr,"ERROR: Output file specified more than once\n");
        return BOOLEAN_FALSE;
      }
      args.file = optarg;
    }
    else if(c == ARG_CHAR_ADDR) {
      if(args.addr != NULL) {
        fprintf(stderr,"ERROR: IP Address specified more than once\n");
        return BOOLEAN_FALSE;
      }
      args.addr = optarg;
    }
    else if(c == ARG_CHAR_PORT) {
      if(args.port != -1) {
        fprintf(stderr,"ERROR: Port specified more than once\n");
        return BOOLEAN_FALSE;
      }
      args.port = atoi(optarg);
    }
    else if(c == ARG_CHAR_HELP) {
      if(args.help_flag) {
         fprintf(stderr,"ERROR: Help option specified more than once\n");
         return BOOLEAN_FALSE;
      }
      args.help_flag = BOOLEAN_TRUE;
    }
    else if(c == '?') {
       fprintf(stderr,"WARNING: Invalid options\n");
       args.help_flag  = BOOLEAN_TRUE;
       break;
    }
    else
      fprintf(stderr,"Bug at line number %d in file %s\n", __LINE__, __FILE__);

    option_index = 0;
    c = getopt_long(argc, argv,"",long_options, &option_index);
  }

  if (optind < argc) {
    fprintf(stderr,"WARNING: Invalid options\n");
    args.help_flag  = BOOLEAN_TRUE;
  }

  return check_options();
}

int run_server() {
  return args.mode == MODE_SERVER;
}
int run_client() {
  return args.mode == MODE_CLIENT;
}
int show_help() {
  return args.help_flag;
}

int get_port() {
  return args.port;
}
char* get_filename() {
  return args.file;
}
char* get_addr() {
  return args.addr;
}

void print_args() {
  fprintf(stderr,"help_flag=%s\n", args.help_flag ? "true" : "false");
  fprintf(stderr,"listen_flag=%s\n", args.listen_flag ? "true" : "false");
  fprintf(stderr,"port=%d\n",args.port);
  fprintf(stderr,"ip=%s\n",args.addr);
  fprintf(stderr,"file=%s\n",args.file);
  if(args.mode == MODE_CLIENT)
    fprintf(stderr,"mode=client\n");
  else if(args.mode == MODE_SERVER)
    fprintf(stderr,"mode=server\n");
  else
    fprintf(stderr,"mode=empty\n");
}
