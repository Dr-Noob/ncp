CXX=gcc
SANITY_FLAGS=-Wall -Wextra -Werror -fstack-protector-all -pedantic -Wno-unused -Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2
OUTPUT=client

$(OUTPUT): Makefile client.c
	$(CXX) client.c -g -pthread $(SANITY_FLAGS) -o $(OUTPUT)

clean:
	@rm $(OUTPUT)
