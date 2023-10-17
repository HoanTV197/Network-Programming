#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#define USAGE_MSG     "Usage: ./dns_resolver <domain.name> or <ip>\n"
#define NOT_FOUND_MSG "No information found\n"

// Function declaration.
void resolve_ip_to_hostname(char*);
void resolve_hostname_to_ip(char*);

/**
* @function resolve_hostname_to_ip: Converts a hostname to an IP address.
* @param hostname: A pointer to the hostname to be converted.
**/
void resolve_hostname_to_ip(char* hostname) {
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET; // Only use IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo (hostname, NULL, &hints, &res);
    if (errcode != 0) {
        printf(NOT_FOUND_MSG);
        return;
    }

    printf ("Results: ");
    while (res) {
        inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family) {
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
        }
        inet_ntop (res->ai_family, ptr, addrstr, 100);
        printf ("%s\n", addrstr);
        res = res->ai_next;
    }
}

/**
* @function resolve_ip_to_hostname: Converts an IP address to a hostname.
* @param ip: A pointer to the IP address to be converted.
**/
void resolve_ip_to_hostname(char* ip) {
    struct sockaddr_in sa;    
    char hbuf[NI_MAXHOST];

    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &sa.sin_addr);

    if (getnameinfo((struct sockaddr*) &sa, sizeof(struct sockaddr_in), hbuf, sizeof(hbuf), 
        NULL, 0, NI_NAMEREQD)) {
        printf(NOT_FOUND_MSG);
        return;
    }

    printf("Result: %s\n", hbuf);
}

/**
* @function main:Checks input parameters and calls the appropriate conversion function.
* @param argc: The number of parameters passed into the program.
* @param argv: An array containing the parameters passed into the program.
* @return: 0 if successful, 1 if an error occurs.
**/
int main(int argc, char** argv) {
    if (argc != 2) {
        printf(USAGE_MSG);
        return 1;
    }

    char* input = argv[1];
    struct sockaddr_in sa;

    if (!inet_pton(AF_INET, input, &(sa.sin_addr))) {
        resolve_hostname_to_ip(input);
    } else
        resolve_ip_to_hostname(input);

    return 0;
}
