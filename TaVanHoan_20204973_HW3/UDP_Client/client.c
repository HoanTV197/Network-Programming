#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // Include for the close function

#define BUFF_SIZE 1024
/**
* @function main: Creates a UDP socket, sends a message to a server, and receives a reply.
* @param argc: The number of parameters passed into the program.
* @param argv: An array containing the parameters passed into the program.
* @return: 0 if successful, 1 if an error occurs.
**/
int main(int argc, char *argv[]) {
    // Define variables
    struct sockaddr_in si_other;
    int sockfd;
    socklen_t slen = sizeof(si_other);
    char buffer[BUFF_SIZE];
    char message[BUFF_SIZE];

    /**
    * @check: If the number of parameters is not equal to 3, print usage message and return 1.
    **/
    if (argc != 3) {
        printf("Usage: ./client IPAddress PortNumber\n");
        return 1;
    }

    /**
    * @create: A UDP socket. If socket creation fails, print error message and return 1.
    **/
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        return 1;
    }

    // Initialize si_other to zero
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(atoi(argv[2]));

    /**
    * @convert: The IP address from text to binary form. If conversion fails, print error message and return 1.
    **/
    if (inet_aton(argv[1], &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        return 1;
    }

    while (1) {
        printf("Enter a domain name or IP address: ");
        if (fgets(message, BUFF_SIZE - 1, stdin) == NULL) {
            perror("fgets");
            return 1;
        }
        // Remove the trailing newline character from fgets input
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
            message[--len] = '\0';
        }

        /**
        * @check: If the string is empty, print a message and break the loop.
        **/
        if (strlen(message) == 0) {
            printf("Empty string detected. Exiting...\n");
            break;
        }

        /**
        * @send: The message to the server. If sending fails, print error message and return 1.
        **/
        if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == -1) {
            perror("sendto()");
            return 1;
        }

        /**
        * @receive: A reply from the server and print it. If receiving fails, print error message and return 1.
        **/
        memset(buffer, '\0', BUFF_SIZE);
        if (recvfrom(sockfd, buffer, BUFF_SIZE - 1, 0, (struct sockaddr *) &si_other, &slen) == -1) {
            perror("recvfrom()");
            return 1;
        }

        puts(buffer);
    }

    close(sockfd);
    return 0;
}
