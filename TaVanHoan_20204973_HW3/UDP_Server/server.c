#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <time.h>

#define BUFF_SIZE 1024
#define NOT_FOUND_MSG "–Not found information\n"

// Function declaration.
void resolve_ip_to_hostname(char*, int, char*);
void resolve_hostname_to_ip(char*, char*);

/**
* @function resolve_hostname_to_ip: Resolves a hostname to its corresponding IP address.
* @param hostname: The hostname to resolve to an IP address.
* @param result: The buffer to store the resolved IP address or an error message.
**/

void resolve_hostname_to_ip(char* hostname, char* result) {
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC; // Use both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo (hostname, NULL, &hints, &res);
    if (errcode != 0) {
        strcpy(result, NOT_FOUND_MSG);
        return;
    }

    strcpy(result, "+");
    while (res) {
        inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family) {
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
        }
        inet_ntop (res->ai_family, ptr, addrstr, 100);
        strcat(result, addrstr);
        strcat(result, " ");
        res = res->ai_next;
    }
}

/**
* @function resolve_ip_to_hostname: Resolves an IP address to its corresponding hostname.
* @param ip: The IP address to resolve to a hostname.
* @param is_ipv6: Indicates if the IP address is in IPv6 format (1 for IPv6, 0 for IPv4).
* @param result: The buffer to store the resolved hostname or an error message.
**/
void resolve_ip_to_hostname(char* ip, int is_ipv6, char* result) {
    char hbuf[NI_MAXHOST];

    if(is_ipv6) {
        struct sockaddr_in6 sa;    
        memset(&sa, 0, sizeof(struct sockaddr_in6));
        sa.sin6_family = AF_INET6;
        inet_pton(AF_INET6, ip, &sa.sin6_addr);

        if (getnameinfo((struct sockaddr*) &sa, sizeof(struct sockaddr_in6), hbuf, sizeof(hbuf), 
            NULL, 0, NI_NAMEREQD)) {
            strcpy(result, NOT_FOUND_MSG);
            return;
        }
    } else {
        struct sockaddr_in sa;    
        memset(&sa, 0, sizeof(struct sockaddr_in));
        sa.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &sa.sin_addr);

        if (getnameinfo((struct sockaddr*) &sa, sizeof(struct sockaddr_in), hbuf, sizeof(hbuf), 
            NULL, 0, NI_NAMEREQD)) {
            strcpy(result, NOT_FOUND_MSG);
            return;
        }
    }

    strcpy(result,"+");
    strcat(result,hbuf);
}

/**
* @function main: Creates a UDP socket, receives messages from a client, resolves the hostname or IP address, logs the result, and sends a reply to the client.
* @param argc: The number of parameters passed into the program.
* @param argv: An array containing the parameters passed into the program.
* @return: 0 if successful, 1 if an error occurs.
**/
int main(int argc , char *argv[]) {
    // Define variables
    int sockfd;
    struct sockaddr_in si_me, si_other;
    char buffer[BUFF_SIZE];
    char message[BUFF_SIZE];
    socklen_t slen = sizeof(si_other) , recv_len;

    /**
    * @check: If the number of parameters is not equal to 2, print usage message and return 1.
    **/
    if(argc != 2) {
        printf("Usage: ./server PortNumber\n");
        return 1;
    }

    /**
    * @create: A UDP socket. If socket creation fails, print error message and return 1.
    **/
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        return 1;
    }

    // Zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(atoi(argv[1]));
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    /**
    * @bind: Socket to port. If binding fails, print error message and return 1.
    **/
    if( bind(sockfd , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
        perror("bind");
        return 1;
    }

    // Keep listening for data
    while(1) {
        printf("Waiting for data...");
        fflush(stdout);

        /**
        * @receive: Some data. If receiving fails, print error message and return 1.
        **/
        if ((recv_len = recvfrom(sockfd, buffer, BUFF_SIZE-1 , 0 , (struct sockaddr *) &si_other,
                                 &slen)) == -1) {
            perror("recvfrom()");
            return 1;
        }

        buffer[recv_len] = '\0';

        struct sockaddr_in sa;
        struct sockaddr_in6 sa6;

        /**
        * @resolve: The hostname or IP address and store the result in message.
        **/
        if (!inet_pton(AF_INET6, buffer, &(sa6.sin6_addr))) {
            if (!inet_pton(AF_INET, buffer, &(sa.sin_addr))) {
                resolve_hostname_to_ip(buffer, message);
            } else
                resolve_ip_to_hostname(buffer, 0, message);
        } else
            resolve_ip_to_hostname(buffer, 1, message);

        // Log the result
        FILE *f = fopen("log_20204973.txt", "a+");
        
        /**
        * @check: If file opening fails, print error message and return 1.
        **/
        if (f == NULL) {
            printf("Error opening file!\n");
            return 1;
        }

        // Get current time
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        fprintf(f, "[%02d/%02d/%d %02d:%02d:%02d]$%s$%s\n", tm.tm_mday, tm.tm_mon + 1,
                tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, buffer, message);

        fclose(f);

        /**
        * @reply: The client with the result. If sending fails, print error message and return 1.
        **/
        if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr*) &si_other,
                   slen) == -1) {
            perror("sendto()");
            return 1;
        }
    }
    close(sockfd);
    return 0;
}

