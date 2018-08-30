#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "client.h"
#include "server.h"

/***
SAMPLE USAGE

$ ncp --listen [--out=NAME] [--port=PORT]
Listening on IP:port...

=======================
$ ncp [--file=FILE] --addr=IP [--port=PORT]
Connecting to IP:port...

allow stdin and stdout if file or out are not specified
***/

static const char* VERSION = "0.3";

void help(int argc, char *argv[]) {
	fprintf(stderr,"ncp v%s -- Copy files over the network\n\
This software can work in two modes:\n\
  * Client: You choose the file to send and the address to be sent.\n\
  Usage: %s --addr=IP [--file=FILE] [--port=PORT]\n\
    --addr: Specify IP address to connect to.\n\
    --file: Specify file to be sent. If no file is specified, ncp will read from standart input.\n\
    --port: Specify port to connect. If no port is specified, ncp will use the default one.\n\
  * Server: You listen to a client which will send the file.\n\
  Usage: %s --listen [--out=FILE] [--port=PORT]\n\
    --listen: Set server mode.\n\
    --out: Specify file name that will be used to save the received file. If no file is specified, ncp will write to standart output.\n\
    --port: Specify port to open. If no port is specified, ncp will use the default one.\n"
			,VERSION,argv[0],argv[0]);
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
