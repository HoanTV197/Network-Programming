#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFF_SIZE 8192
#define SEND_SIZE 1024


/**
* @function uploadFile: This function handles the file upload process.
* It opens the file, reads data from the file and sends the data over the socket.
* @param sock: socket to send data.
* @param file_path: path to the file to be uploaded.
* @param num_bytes: number of bytes to send.
**/
void uploadFile(int sock, const char *file_path, int num_bytes) {
    FILE *fileptr;
    struct stat st;
    
    if (stat(file_path, &st) == -1) {
        perror("Error: ");
        return;
    }
    
    // Initialize send data
    char send_data[BUFF_SIZE];
    sprintf(send_data, "UPLD %s %d\r\n", file_path, num_bytes);
    send(sock, send_data, strlen(send_data), 0);

    // Receive response from server
    memset(send_data, '\0', BUFF_SIZE);
    recv(sock, send_data, BUFF_SIZE, 0);
    printf("%s\n", send_data);

    fileptr = fopen(file_path, "rb");
    if (fileptr == NULL) {
        perror("Error: ");
        return;
    }
    
    // Send data from file over socket
    int total_bytes_sent = 0;
    while (total_bytes_sent < num_bytes) {
        size_t bytesRead = fread(send_data, 1, SEND_SIZE, fileptr);
        if (bytesRead == 0) {
            break;
        }

        if (send(sock, send_data, bytesRead, 0) == -1) {
            perror("Error in sending file data");
            break;
        }
        total_bytes_sent += bytesRead;
    }

    fclose(fileptr);

    // Receive successful upload message from server
    memset(send_data, '\0', BUFF_SIZE);
    recv(sock, send_data, BUFF_SIZE, 0);
    printf("%s\n", send_data);
}

/**
* @function main: The main function of the program.
* It initializes the socket, connects to the server and sends data over the socket.
* @param argc: number of command line arguments.
* @param argv: array containing the command line arguments.
**/
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <Port_Number> <Directory_name>\n", argv[0]);
        return 1;
    }

    int sock;
    char send_data[BUFF_SIZE];
    struct sockaddr_in server;

    // Step 1: Construct a TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation error");
        return 1;
    }

    // Step 2: Define the server address
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) <= 0) {
        perror("Invalid address !");
        close(sock);
        return 1;
    }

    // Step 3: Connect to the server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Step 4: Receive and print welcome message from server
    memset(send_data, '\0', BUFF_SIZE);
    recv(sock, send_data, BUFF_SIZE, 0);
    printf("%s\n", send_data);

    // Step 5: Communicate with server
    char file_path[BUFF_SIZE];

    while (true) {
        printf("Input 'UPLD <file_path> <file_size>' : \n");
        fgets(file_path, BUFF_SIZE, stdin);
        printf("%s\n", file_path);
        file_path[strcspn(file_path, "\n")] = '\0';
        char filename[BUFF_SIZE];
         sscanf(file_path, "UPLD %s", filename);
        int file_size;
        sscanf(file_path, "UPLD %*s %d", &file_size);
         printf("%s\n", filename);
        if (strlen(file_path) == 0) {
            break;
        }

        uploadFile(sock, filename, file_size);
    }

    // Step 6: Close the socket
    close(sock);
    return 0;
}

