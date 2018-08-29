#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "client.h"
#include "server.h"

/***
SAMPLE USAGE

$ ncp --listen --out=NAME [--port=PORT]
Listening on IP:port...

=======================
$ ncp --file=FILE --addr=IP [--port=PORT]
Connecting to IP:port...

***/

void help(int argc, char *argv[]) {

}

int main(int argc, char* argv[]) {
  if(!parseArgs(argc,argv))
    return EXIT_FAILURE;

  if(show_help()) {
    help(argc,argv);
    return EXIT_SUCCESS;
  }

  if(run_server())
    server(get_filename(),get_port());
  else
    client(get_filename(),get_addr(),get_port());
}
