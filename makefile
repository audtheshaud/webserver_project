CC		= gcc
CFLAGS  = -Og

default: all

all: make_server make_client

make_server: server.c
	$(CC) $(CFLAGS) server.c -o server


make_client: client.c client.h
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm client server