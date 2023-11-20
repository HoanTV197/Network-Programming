#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_USERNAME_LEN 100
typedef struct {
    int sockfd;
    struct sockaddr_in client_addr;
    char username[MAX_USERNAME_LEN];
    int isLoggedIn;
} Session;

typedef struct {
    char username[50];
    int status;
} Account;
Account currentUser;
int isLoggedIn = 0;

/**
* @function findAccount: This function finds an account with a given username in the "account.txt" file.
* @param username: A pointer to a string representing the username of the account.
* @return: If the account is found, it returns the account. If not found or an error occurs,
 it returns an Account with username as an empty string and status as -1.
**/
Account findAccount(char* username) {
    FILE* f = fopen("account.txt", "r");
    if (f == NULL) {
        printf("Cannot open account file.\n");
        Account a;
        strcpy(a.username, "");
        a.status = -1;
        return a;
    }

    Account a;
    while (fscanf(f, "%s %d\n", a.username, &a.status) != EOF) {
        if (strcmp(a.username, username) == 0) {
            fclose(f);
            return a;
        }
    }

    fclose(f);
    strcpy(a.username, "");
    a.status = -1;
    return a;
}

/**
* @function Login: This function performs the login operation with a given username.
* @param connfd: An integer representing the connection file descriptor.
* @param username: A pointer to a string representing the username.
**/
void Login(int connfd, char* username) {
    if (isLoggedIn) {
        write(connfd, "213", 4); // Account is already logged in
        return;
    }

    Account a = findAccount(username);
    if (strcmp(a.username, "") == 0) {
        write(connfd, "212", 4); // Account does not exist
    } else if (a.status == 0) {
        write(connfd, "211", 4); // Account is locked
    } else {
        currentUser = a;
        isLoggedIn = 1;
        write(connfd, "110", 4); // Login successful
    }
}

/**
* @function Post: This function allows the user to post a message.
* @param connfd: An integer representing the connection file descriptor.
* @param article: A pointer to a string representing the article to be posted.
**/
void Post(int connfd, char* article) {
    if (!isLoggedIn) {
        write(connfd, "221", 4);
    } else {
        printf("Received : %s\n", article);
        write(connfd, "120", 4);
    }
}

/**
* @function Logout: This function performs the logout operation.
* @param connfd: An integer representing the connection file descriptor.
**/
void Logout(int connfd) {
    if (!isLoggedIn) {
        write(connfd, "221", 4);
    } else {
        isLoggedIn = 0;
        write(connfd, "130", 4);
    }
}

/**
* @function Request: This function handles the requests from the client.
* @param arg: A pointer to the argument passed to the function. It is cast to an integer pointer representing the connection file descriptor.
* @return: This function does not return a value.
**/
void* Request(void* arg) {
    int connfd = *((int*)arg);
    free(arg);

    while (1) {
        write(connfd, "MENU", 4);
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

    if (read(connfd, buffer, sizeof(buffer)) == 0) {
        printf("Client disconnected.\n");
        break;
    }

        char command[5];
        char rest[1024];

        if (sscanf(buffer, "%4s%[^\n]", command, rest) < 1) {
            write(connfd, "300", 4);
            printf("300\n");
            continue;
        }

        if (strcmp(command, "USER") == 0) {
            char username[50];
            if (sscanf(rest, "%s", username) != 1) {
                write(connfd, "300", 4);
                printf("300\n");
                continue;
            }
            Login(connfd, username);
            printf("USER %s\n", username);
        } else if (strcmp(command, "POST") == 0) {
            char article[1024];
            if (sscanf(rest, "%[^\n]", article) != 1) {
                write(connfd, "300", 4);
                printf("300\n");
                continue;
            }
            Post(connfd, article);
            printf("POST %s\n", article);
        } else if (strcmp(command, "BYE") == 0) {
            Logout(connfd);
            printf("BYE\n");
            break;
        } else {
            write(connfd, "300", 4);
            printf("300\n");
        }
    }

    close(connfd);
    pthread_exit(NULL);
}

/**
* @function main: The main function of the program. It sets up the server and listens for connections from clients.
* @param argc: An integer representing the number of command-line arguments.
* @param argv: An array of pointers to strings representing the command-line arguments.
* @return: Returns 0 if the program executes successfully, and 1 otherwise.
**/
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Please provide a port number.\n");
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    pthread_t thread_id;

    // Create a socket and check if it was successful
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    } else {
        printf("Socket successful created.\n");
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Bind the socket to the server address and check if it was successful
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed.\n");
        exit(0);
    } else {
        printf("Socket successful binded.\n");
    }

    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed.\n");
        exit(0);
    } else {
        printf("Server port %d.\n", port);
    }

    socklen_t len = sizeof(cli);

    while (1) {
        connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (connfd < 0) {
            printf("Server failed.\n");
            exit(0);
        } else {
            printf("100\n");
        }

         write(connfd, "100", 4);
        int* new_sock = (int*)malloc(1);
        *new_sock = connfd;

        // Create a new thread for the new connection and check if it was successful
        if (pthread_create(&thread_id, NULL, Request, (void*)new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }

        pthread_detach(thread_id);
    }

    return 0;
}



