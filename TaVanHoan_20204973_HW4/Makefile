CC = gcc
CFLAGS = -Wall

all : server client
server: TCP_Server/server.c
	$(CC) $(CFLAGS) -o server TCP_Server/server.c 

client: TCP_Client/client.c
	$(CC) $(CFLAGS) -o client TCP_Client/client.c

clean:
	rm -f server client