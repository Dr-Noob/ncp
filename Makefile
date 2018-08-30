CXX=gcc
SANITY_FLAGS=-Wall -Wextra -Werror -fstack-protector-all -pedantic -Wno-unused -Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

MAIN=main.c
ARGS=args.c
CLIENT=client.c
SERVER=server.c
TOOLS=tools.c
PROGRESSBAR=progressbar.c

HEADERS=args.h client.h server.h tools.h progressbar.h

OUTPUT=ncp

$(OUTPUT): Makefile $(MAIN) $(ARGS) $(CLIENT) $(SERVER) $(TOOLS) $(PROGRESSBAR) $(HEADERS)
	$(CXX) -g $(MAIN) $(ARGS) $(CLIENT) $(SERVER) $(TOOLS) $(PROGRESSBAR) -pthread $(SANITY_FLAGS) -o $(OUTPUT)

clean:
	@rm $(OUTPUT)
