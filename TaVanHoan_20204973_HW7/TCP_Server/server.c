#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>    /* for select() */

#define MAX_USERNAME_LEN 50

typedef struct {
    char username[MAX_USERNAME_LEN];
    int status;
} Account;

Account currentUser;
int checkLogin = 0;

/**
* @function findAccount: This function finds an account with a given username in the "account.txt" file.
* @param username: A pointer to a string representing the username of the account.
* @return: If the account is found, it returns the account. If not found or an error occurs,
 it returns an Account with username as an empty string and status as -1.
**/
Account findAccount(char* username) {
    FILE* f = fopen("account.txt", "r+");
    if (f == NULL) {
        perror("Cannot open account file");
        Account a = { "", -1 };
        return a;
    }

    Account a = { "", -1 };
    while (fscanf(f, "%s %d\n", a.username, &a.status) != EOF) {
        if (strcmp(a.username, username) == 0) {
            fclose(f);
            return a;
        }
    }

    fclose(f);
    return a;
}

/**
* @function Login: This function allows a user to log into the system.
* @param connfd: An integer representing the file descriptor of the connection.
* @param username: A pointer to a string representing the username of the account.
**/
void Login(int connfd, char* username) {
    Account a = findAccount(username);
    if (strcmp(a.username, "") == 0) {
        write(connfd, "212", 4);
        printf("212\n");
    } else if (a.status == 0) {
        write(connfd, "211", 4);
        printf("211\n");
    } else {
        currentUser = a;
        checkLogin = 1;
        write(connfd, "100", 4);
        printf("110\n");
    }
}

/**
* @function Post: This function allows the user to post an article to the server.
* @param sockfd: An integer representing the socket file descriptor.
* @param article: A pointer to a string representing the article to be posted.
**/
void Post(int connfd, char* article) {
    if (!checkLogin) {
        write(connfd, "221", 4);
        printf("221\n");
    } else {
        write(connfd, "120", 4);
        printf("120\n");
    }
}

/**
* @function Logout: This function allows the user to log out from the server.
* @param sockfd: An integer representing the socket file descriptor.
**/
void Logout(int connfd) {
    if (!checkLogin) {
        write(connfd, "221", 4);
        printf("221\n");
    } else {
        checkLogin = 0;
        write(connfd, "130", 4);
        printf("BYE\n");
    }
}

/**
* @function Request: This function handles a request from a client.
* @param connfd: An integer representing the file descriptor of the connection.
**/
void Request(int connfd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesRead = read(connfd, buffer, sizeof(buffer));
    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            printf("Client disconnected\n");
        } else {
            perror("read");
        }
        close(connfd);
        exit(EXIT_SUCCESS);
    }

    char command[5];
    char rest[1024];

    if (sscanf(buffer, "%4s%[^\n]", command, rest) < 1) {
        write(connfd, "300", 4);
        printf("300\n");
        return;
    }

    if (strcmp(command, "USER") == 0) {
        char username[50];
        if (sscanf(rest, "%s", username) != 1) {
            write(connfd, "300", 4);
            printf("300\n");
            return;
        }
        Login(connfd, username);
        printf("USER %s\n", username);
   
    } else if (strcmp(command, "POST") == 0) {
        char article[50];
        if (sscanf(rest, "%s", article) != 1) {
            write(connfd, "300", 4);
            printf("300\n");
            return;
        }
        Post(connfd, article);
        printf("POST %s\n", article);
       
    } else if (strcmp(command, "BYE") == 0) {
        Logout(connfd);
        
    } else {
        write(connfd, "300", 4);
        printf("300\n");
    }
}

/**
* @function Connection: This function handles a connection from a client.
* @param connfd: An integer representing the file descriptor of the connection.
**/
void Connection(int connfd) {
    write(connfd, "100", 4);
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(connfd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(connfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (activity == 0) {
            // Timeout, no data received
            continue;
        } else {
            Request(connfd);
            if (!checkLogin) {
                break;
            }
        }
    }
}

/**
* @function main: The main function of the program. It sets up a server and listens for incoming connections.
* @param argc: An integer representing the number of command-line arguments.
* @param argv: An array of pointers to strings representing the command-line arguments.
* @return: Returns 0 on successful execution and 1 if there is an error.
**/
int main(int argc, char* argv[]) {
    // Step 1: Check if the port number is provided as a command-line argument.
    if (argc < 2) {
        printf("Please provide a port_number.\n");
        return 1;
    }

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    socklen_t len = sizeof(cli);

    // Step 2: Create a socket using the AF_INET (IPv4)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Socket successfully created.\n");
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    // Step 3: Bind the socket to the server address.
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Socket successfully binded.\n");
    }

    // Step 4: Listen for incoming connections.
    if (listen(sockfd, 5) != 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Server listening.\n");
    }

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (activity == 0) {
            // Timeout, no new connection
            continue;
        } else {
            connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
            if (connfd < 0) {
                perror("Server accept failed");
                exit(EXIT_FAILURE);
            } else {
                printf("100\n");
            }

            if (fork() == 0) {
                close(sockfd);
                Connection(connfd);
                close(connfd);
                exit(EXIT_SUCCESS);
            } else {
                close(connfd);
            }
        }
    }

    return 0;
}

