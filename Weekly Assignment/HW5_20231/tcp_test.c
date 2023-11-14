#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_NTHREADS 10
#define MAX_NREQUEST 10
#define MAX_NCONNS 64
#define MAX_NCONNS_PER_THREAD 16
#define SERVER_IP "127.0.0.1"
#define BUFF_SIZE 2048
#define SUCCESS_LOGIN "110"
#define SUCCESS_POST "120"
#define SUCCESS_LOGOUT "130"

void *worker1(void *);
void *worker2(void *);
void *worker3(void *);

int serverPort;
struct sockaddr_in serverAddr;

int main(int argc, char **argv)
{ 
	int nthreads = 0;
	if (argc != 3){
		printf("usage: test <#serverPort> <#threads>\n");
		return 0;
	}
	
	serverPort = atoi(argv[1]);
	
	int client;
	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];	
	int ret;
		
	//Define the address of the server
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
		
	if ((client = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("\nsocket() Error: ");
		exit(EXIT_FAILURE);
	}
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; //Time-out interval: 100000ms
	
	if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) == -1 ){
		perror("\nsetsockopt() Error: ");
		exit(EXIT_FAILURE);
	}
	
	if (connect(client, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
		perror("\nconnect() Error: ");
		exit(EXIT_FAILURE);
	}
	
	//Connection test
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Connect test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("%s\n", rBuff);
	}
	
	//Sequence test
	strcpy(sBuff, "POST Hello\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Sequence test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_POST) != 0)
			printf("Sequence test fail!\n");
	}
	
	strcpy(sBuff, "BYE\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Sequence test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_LOGOUT) != 0)
			printf("Sequence test fail!\n");
	}
	
	//Function test
	strcpy(sBuff, "USER ductq\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_LOGIN) != 0)
			printf("Login test fail!\n");
	}
	else
		printf("Login test fail!\n");

	strcpy(sBuff, "USER admin\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_LOGIN) == 0)
			printf("Login test fail!\n");
	}
	else
		printf("Login test fail!\n");

	strcpy(sBuff, "USER tungbt\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_LOGIN) != 0)
			printf("Login test fail!\n");
	}
	else
		printf("Login test fail!\n");

	strcpy(sBuff, "POST Hello\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_POST) == 0)
			printf("Post message test fail!\n");
	}
	else
		printf("Post message test fail!\n");

	strcpy(sBuff, "BYE\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_LOGOUT) == 0)
			printf("Logout test fail!\n");
	}
	else
		printf("Logout test fail!\n");
		
	strcpy(sBuff, "POST Hello\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Sequence test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_POST) != 0)
			printf("Sequence test fail!\n");
	}
	
	strcpy(sBuff, "USER tungbt\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_LOGIN) == 0)
			printf("Login test fail!\n");
	}
	else
		printf("Login test fail!\n");
		
	
	strcpy(sBuff, "BYE\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, SUCCESS_LOGOUT) == 0)
			printf("Logout test fail!\n");
	}
	else
		printf("Logout test fail!\n");
			
	//Syntax test
	strcpy(sBuff, "USER \r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Syntax test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
		if (strstr(rBuff, "300") != 0)
			printf("Syntax test fail!\n");
	}

	strcpy(sBuff, "foo\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Syntax test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}
	
	//Stream test
	strcpy(sBuff, "USER admin\r\nPOST Hello world\r\nPOST Test stream\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Stream test 1 fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Stream test 1 fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Stream test 1 fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	strcpy(sBuff, "POST I am tungbt");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0)
		printf("Stream test 2 fail!\n");

	strcpy(sBuff, "\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Stream test 2 fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}
	
	close(client);
	
	pthread_t tid_worker1, tid_worker2, tid_worker3[MAX_NTHREADS];
		
	int any;
	printf("Press any number!\n");
	scanf("%d", &any);
	
	//Concurency test 1
	pthread_create(&tid_worker1, NULL, worker1, NULL);
	pthread_create(&tid_worker2, NULL, worker2, NULL);
	
	pthread_join(tid_worker1, NULL);
	pthread_join(tid_worker2, NULL);
	
	//Concurency test 2
	int numConn = 0;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 10000000;
	
	printf("Number of concurent connections: ");
	scanf("%d", &numConn);
	if (numConn > 0) {
		
		char buff[BUFF_SIZE];
		int numSession = 0, numConnected = 0;
		int clients[MAX_NCONNS];
		if (numConn > MAX_NCONNS) numConn = MAX_NCONNS;
		for (int i = 0; i < numConn; i++) {
			clients[i] = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(clients[i], (struct sockaddr *)&serverAddr, sizeof(serverAddr))) {
				perror("\nsocket() Error: ");
				break;
			}
			nanosleep(&ts, &ts);
			numConnected++;
			struct timeval tv2;
			tv2.tv_sec = 0;
			tv2.tv_usec = 100000; //Time-out interval: 100000ms
			setsockopt(clients[i], SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv2), sizeof(struct timeval));
			ret = recv(clients[i], rBuff, BUFF_SIZE, 0);
			if (ret <= 0)
				printf("Connect test fail!\n");
			else {
				rBuff[ret] = 0;
				printf("%s\n", rBuff);
			}
			strcpy(sBuff, "USER admin\r\n");
			ret = send(clients[i], sBuff, strlen(sBuff), 0);
			ret = recv(clients[i], rBuff, BUFF_SIZE, 0);

			if (ret <= 0)
				printf("recv() fail.\n");
			else {
				rBuff[ret] = 0;
				printf("Concurent test: %s\n", rBuff);
				numSession++;
			}
		}

		printf("\nNumber of success connection: %d", numConnected);
		printf("\nNumber of success session: %d\n", numSession);

		for (int i = 0; i < numConn; i++)
			close(clients[i]);
	}
	
	printf("Press any number!\n");
	scanf("%d", &any);
	
	//Concurency test 3
	nthreads = atoi(argv[2]);
	if (nthreads > MAX_NTHREADS)
		nthreads = MAX_NTHREADS;
	printf("Number of thread: %d\n", nthreads);
	
	/* create all workers */
	for (int i = 0; i < nthreads; i++)
		pthread_create(&tid_worker3[i], NULL, worker3, NULL);	

	/* wait for all worker */
	for (int i = 0; i < nthreads; i++)
		pthread_join(tid_worker3[i], NULL);		
	
	return 0;	
}

void * worker1(void *arg){

	int client;
	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];	
	int ret;
	
	if ((client = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("\nsocket() Error: ");
		exit(EXIT_FAILURE);
	}
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; //Time-out interval: 100000ms
	
	if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) == -1 ){
		perror("\nsetsockopt() Error: ");
		exit(EXIT_FAILURE);
	}
	
	if (connect(client, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
		perror("\nconnect() Error: ");
		exit(EXIT_FAILURE);
	}
	
	//Connection test
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Connect test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Thread 1: %s\n", rBuff);
	}
	
	strcpy(sBuff, "USER tungbt\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Thread 1:  %s-->%s\n", sBuff, rBuff);
	}
	else
		printf("Receive on thread 1 failed\n");
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 10000000;
	for (int i = 0; i < 5; i++) {
		nanosleep(&ts, &ts);
		strcpy(sBuff, "POST Hello. I am tungbt\r\n");
		ret = send(client, sBuff, strlen(sBuff), 0);
		ret = recv(client, rBuff, BUFF_SIZE, 0);
		if (ret > 0) {
			rBuff[ret] = 0;
			printf("Thread 1:  %s-->%s\n", sBuff, rBuff);
		}
		else
			printf("Receive on thread 1 failed\n");
	}

	strcpy(sBuff, "BYE\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Thread 1:  %s-->%s\n", sBuff, rBuff);
	}
	else
		printf("Receive on thread 1 failed\n");

	strcpy(sBuff, "USER test\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Thread 1:  %s-->%s\n", sBuff, rBuff);
	}
	else
		printf("Receive on thread 1 failed\n");

	for (int i = 0; i < 5; i++) {
		nanosleep(&ts, &ts);
		strcpy(sBuff, "POST Hello. I am test\r\n");
		ret = send(client, sBuff, strlen(sBuff), 0);
		ret = recv(client, rBuff, BUFF_SIZE, 0);
		if (ret > 0) {
			rBuff[ret] = 0;
			printf("Thread 1:  %s-->%s\n", sBuff, rBuff);
		}
		else
			printf("Receive on thread 1 failed\n");
	}

	//Step 6: Close socket
	close(client);
	printf("Thread 1 end.\n");
}

void * worker2(void *arg){

	int client;
	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];	
	int ret;
	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 1000000;
	nanosleep(&ts, &ts);
	
	if ((client = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("\nsocket() Error: ");
		exit(EXIT_FAILURE);
	}
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; //Time-out interval: 100000ms
	
	if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) == -1 ){
		perror("\nsetsockopt() Error: ");
		exit(EXIT_FAILURE);
	}
	
	if (connect(client, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
		perror("\nconnect() Error: ");
		exit(EXIT_FAILURE);
	}
	
	//Connection test
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret <= 0)
		printf("Connect test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Thread 2: %s\n", rBuff);
	}

	strcpy(sBuff, "USER admin\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Thread 2:  %s-->%s\n", sBuff, rBuff);
	}
	else
		printf("Receive on thread failed\n");

	for (int i = 0; i < 10; i++) {
		nanosleep(&ts, &ts);
		strcpy(sBuff, "POST Hello. I am admin\r\n");
		ret = send(client, sBuff, strlen(sBuff), 0);
		ret = recv(client, rBuff, BUFF_SIZE, 0);

		if (ret > 0) {
			rBuff[ret] = 0;
			printf("Thread 2:  %s-->%s\n", sBuff, rBuff);
		}
		else
			printf("Receive on thread failed\n");
	}

	strcpy(sBuff, "BYE\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Thread 2:  %s-->%s\n", sBuff, rBuff);
	}
	else
		printf("Receive on thread failed\n");

	strcpy(sBuff, "USER ductq\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret > 0) {
		rBuff[ret] = 0;
		printf("Thread 2:  %s-->%s\n", sBuff, rBuff);
	}
	else
		printf("Receive on thread failed\n");

	//Step 6: Close socket
	close(client);
	printf("Thread 2 end.\n");
	
}
void * worker3(void *arg)
{
	int ret = 0;
	int numSession = 0, numConnected = 0;
	int clients[MAX_NCONNS];
	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 20000000;
	
	
	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE + 1];
	for (int i = 0; i < MAX_NCONNS_PER_THREAD; i++) {
		clients[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(clients[i], (struct sockaddr *)&serverAddr, sizeof(serverAddr))) {
			perror("\nconnect() Error: ");
			break;
		}
		numConnected++;
		
		nanosleep(&ts, &ts);
		
		struct timeval tv2;
		tv2.tv_sec = 0;
		tv2.tv_usec = 100000; //Time-out interval: 100000ms
	
		if (setsockopt(clients[i], SOL_SOCKET, SO_RCVTIMEO, &tv2, sizeof(struct timeval)) == -1 ){
			perror("\nsetsockopt() Error: ");
			exit(EXIT_FAILURE);
		}
		ret = recv(clients[i], rBuff, BUFF_SIZE, 0);
		if (ret <= 0)
			printf("Connect test fail!\n");
		else {
			rBuff[ret] = 0;
			printf("Worker 3: %s\n", rBuff);
		}
			
		strcpy(sBuff, "USER admin\r\n");
		ret = send(clients[i], sBuff, strlen(sBuff), 0);
		ret = recv(clients[i], rBuff, BUFF_SIZE, 0);

		if (ret <= 0)
			printf("recv() fail.\n");
		else {
			rBuff[ret] = 0;
			printf("Worker 3: %s\n", rBuff);
			numSession++;
		}

	}
	
	printf("\nNumber of success connection: %d", numConnected);
	printf("\nNumber of success session: %d\n", numSession);
	
	for (int i = 0; i < MAX_NCONNS_PER_THREAD; i++) {
		int ok = 0;
		for (int k = 0; k < MAX_NREQUEST; k++) {
			strcpy(sBuff, "POST Hello. I am admin\r\n");
			ret = send(clients[i], sBuff, strlen(sBuff), 0);
			ret = recv(clients[i], rBuff, BUFF_SIZE, 0);
			if (ret <= 0)
				printf("recv() %d fail.\n", k);
			else {
				ok++;
			}
		}
		if (ok < MAX_NREQUEST) printf("Concurency test 3 failed\n");
	}

	

	for (int i = 0; i < MAX_NCONNS_PER_THREAD; i++)
		close(clients[i]);
}

