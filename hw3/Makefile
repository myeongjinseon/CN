CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -pthread

all: hw3_server hw3_client

server: hw3_server.c
	$(CC) $(CFLAGS) -o hw3_server hw3_server.c $(LDFLAGS)

client: hw_3client.c
	$(CC) $(CFLAGS) -o hw3_client hw3_client.c

clean:
	rm -f hw3_server hw3_client *.o
