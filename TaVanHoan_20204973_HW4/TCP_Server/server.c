#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

#define BACKLOG   3 
#define BUFF_SIZE 8192
#define SEND_SIZE 1024

#define WELCOME_MESSAGE "+OK Welcome to file server"
#define PLEASE_SEND_FILE_MESSAGE "+OK Please send file"
#define SUCCESSFUL_UPLOAD_MESSAGE "+OK Successful upload"

/**
* @function create_socket_and_bind: This function creates a socket and binds it to an address.
* @param server: pointer to the server address structure.
* @param argv: command line arguments.
* @return listen_sock: the socket descriptor.
**/
int create_socket_and_bind(struct sockaddr_in *server, char *argv[]) {
    int listen_sock;

    // Construct TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("\nError: ");
        return 0;
    }

    // Bind address to socket
    memset(server, 0, sizeof(*server));
    server->sin_family = AF_INET;
    server->sin_port = htons(atoi(argv[1]));     
    server->sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY puts your IP address automatically 
    if (bind(listen_sock, (struct sockaddr *) server, sizeof(*server)) ==
        -1) { 
        perror("\nError: ");
        return 0;
    }

    return listen_sock;
}

/**
* @function write_log: This function writes a log entry to a file.
* @param client_addr: client's IP address.
* @param client_port: client's port number.
* @param request: client's request.
* @param response: server's response.
**/
void write_log(const char *client_addr, int client_port, const char *request, const char *response) {
    FILE *fileptr;
    char buffer[100];
    time_t rawtime;
    struct tm *info;

    time(&rawtime);
    info = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "[%d/%m/%Y %H:%M:%S]", info);

    fileptr = fopen("log_20204973.txt", "a");
    if (fileptr == NULL) {
        perror("Error: ");
        return;
    }

    fprintf(fileptr, "%s$%s:%d$%s$%s\n", buffer, client_addr, client_port, request, response);

    fclose(fileptr);
}


/**
* @function handle_client_communication: This function handles communication with a client.
* It sends a welcome message, receives a file from the client, and writes the file to disk.
* @param conn_sock: the connection socket.
* @param argv: command line arguments.
* @param client: the client's address structure.
**/
void handle_client_communication(int conn_sock, char *argv[], struct sockaddr_in client) {
    FILE *fileptr;
    char recv_data[BUFF_SIZE]="";
    char request[BUFF_SIZE];

    send(conn_sock, WELCOME_MESSAGE "\n", strlen(WELCOME_MESSAGE) + 1, 0);
    write_log(inet_ntoa(client.sin_addr), ntohs(client.sin_port), WELCOME_MESSAGE,"");

    memset(recv_data, '\0', BUFF_SIZE);
    if (recv(conn_sock, recv_data, BUFF_SIZE, 0) <= 0) {
        printf("Error in receiving file infomation");
    write_log(inet_ntoa(client.sin_addr), ntohs(client.sin_port), request, PLEASE_SEND_FILE_MESSAGE);
        return;
    }
    strcpy(request, recv_data);
    

    char file_name[BUFF_SIZE], file_size_str[BUFF_SIZE];
    sscanf(recv_data, "UPLD %s %s\n", file_name, file_size_str);

    printf("Received file infomation: %s %s\n", file_name, file_size_str);

    char file_path[BUFF_SIZE];
    strcpy(file_path, argv[2]);
    strcat(file_path, "/");
    strcat(file_path, file_name);

    send(conn_sock, PLEASE_SEND_FILE_MESSAGE "\n", strlen(PLEASE_SEND_FILE_MESSAGE) + 1, 0);
    write_log(inet_ntoa(client.sin_addr), ntohs(client.sin_port), PLEASE_SEND_FILE_MESSAGE,"");

    //receive and write file data
    fileptr = fopen(file_path, "wb");
    int total_bytes_received = 0;
    int file_size = atoi(file_size_str);
    while (total_bytes_received < file_size) {

        memset(recv_data, '\0', BUFF_SIZE);
        int bytes_received = recv(conn_sock, request, BUFF_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Error in receiving file data");

            break;
        }
        fwrite(recv_data, bytes_received, 1, fileptr);
        total_bytes_received += bytes_received;
    }

    // send successful upload message
    send(conn_sock, SUCCESSFUL_UPLOAD_MESSAGE "\n", strlen(SUCCESSFUL_UPLOAD_MESSAGE) + 1, 0);
    write_log(inet_ntoa(client.sin_addr), ntohs(client.sin_port),request, SUCCESSFUL_UPLOAD_MESSAGE);
    fclose(fileptr);
}

/**
* @function main: The main function of the program.
* It creates a socket, binds it to an address, listens for connections, and handles communication with clients.
* @param argc: number of command line arguments.
* @param argv: array containing the command line arguments.
**/
int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <Port_Number> <Directory_name>\n", argv[0]);
        return 1;
    }

    int listen_sock, conn_sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size;

    listen_sock = create_socket_and_bind(&server, argv);

   
    if (listen(listen_sock, BACKLOG) == -1) { 
        perror("\nError: ");
        return 0;
    }

    struct stat st = {0};

    if (stat(argv[2], &st) == -1) {   
        mkdir(argv[2], 0755);
    }


    while (true) {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *) &client, &sin_size)) == -1)
            perror("\nError: ");

        printf("Connected to  %s\n",
               inet_ntoa(client.sin_addr)); 

        handle_client_communication(conn_sock, argv, client);

        // end connection
        close(conn_sock);
        fflush(stdout);
    }

    close(listen_sock);
    return 0;
}


