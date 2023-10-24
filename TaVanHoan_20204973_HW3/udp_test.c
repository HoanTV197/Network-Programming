#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_NTHREADS 100
#define MAX_NREQUEST 10
#define SERVER_IP "127.0.0.1"
#define BUFF_SIZE 2048

void *worker(void *);
int server_port;

int main(int argc, char **argv)
{ 
	int nthreads = 0;
	pthread_t tid_worker[MAX_NTHREADS];
	if (argc != 3){
		printf("usage: test <#server_port> <#threads>");
		return 0;
	}
	server_port = atoi(argv[1]);
	
	nthreads = atoi(argv[2]);
	if (nthreads > MAX_NTHREADS)
		nthreads = MAX_NTHREADS;
	printf("Number of thread: %d\n", nthreads);
	
	
	/* create all workers */
	for (int i = 0; i < nthreads; i++)
		pthread_create(&tid_worker[i], NULL, worker, NULL);	

	/* wait for all worker */
	for (int i = 0; i < nthreads; i++)
		pthread_join(tid_worker[i], NULL);		
	
	return 0;	
}

void * worker(void *arg)
{
	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent,bytes_received, sin_size;
	char domains[7][255] = {"facebook.com", "123mua.com", "8.8.8.8", "1.1.1.1", "123mua.def", "202.191.56.66", "259.154.1.1"};
	
	//Step 1: Construct a UDP socket
	if ((client_sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}

	//Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	sin_size = sizeof(struct sockaddr);
	
	//Step 3: Communicate with server
	srand((int)time(0));
	for(int i = 0; i < MAX_NREQUEST; i++){
		int k = rand() % 7;	
		bytes_sent = sendto(client_sock, domains[k], strlen(domains[k]), 0, (struct sockaddr *) &server_addr, sin_size);
		if(bytes_sent < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		printf("Sent to server: %s\n", domains[k]);
		
		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
		if(bytes_received < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		buff[bytes_received] = '\0';
		printf("Reply from server: %s\n", buff);
	}	
	close(client_sock);
}

