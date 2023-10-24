CC = gcc
CFLAGS = -Wall

all : server client
server: UDP_Server/server.c
	$(CC) $(CFLAGS) -o server UDP_Server/server.c 

client: UDP_Client/client.c
	$(CC) $(CFLAGS) -o client UDP_Client/client.c

clean:
	rm -f server client
