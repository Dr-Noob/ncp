CXX=gcc
SANITY_FLAGS=-Wall -Wextra -Werror -fstack-protector-all -pedantic -Wno-unused -Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

MAIN=main.c
ARGS=args.c
CLIENT=client.c
SERVER=server.c
TOOLS=tools.c

HEADERS=args.h client.h server.h tools.h

OUTPUT=ncp

$(OUTPUT): Makefile $(MAIN) $(ARGS) $(CLIENT) $(SERVER) $(TOOLS) $(HEADERS)
	$(CXX) -g $(MAIN) $(ARGS) $(CLIENT) $(SERVER) $(TOOLS) -pthread $(SANITY_FLAGS) -o $(OUTPUT)

clean:
	@rm $(OUTPUT)
