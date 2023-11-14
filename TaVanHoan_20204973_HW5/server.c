
// Server code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MAX_USERNAME_LEN 50
#define MAX_LOG_LEN 500

typedef struct {
    char username[MAX_USERNAME_LEN];
    int status;
} Account;

Account currentUser;
int isLoggedIn = 0;

Account findAccount(char* username) {
    FILE *f = fopen("account.txt", "r");
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

void handleLogin(int connfd, char* username) {
    Account a = findAccount(username);
    if (strcmp(a.username, "") == 0) {
        write(connfd, "212", 4);
    } else if (a.status == 0) {
        write(connfd, "211", 4);
    } else {
        currentUser = a;
        isLoggedIn = 1;
        write(connfd, "110", 4);
    }
}

void handlePost(int connfd, char* article) {
    if (!isLoggedIn) {
        write(connfd, "221", 4);
    } else {
        // Here you should handle the post request, for example by saving the article to a database or a file.
        // Since this is not specified in your question, I will just send a success response.
        write(connfd, "120", 4);
    }
}

void handleLogout(int connfd) {
    if (!isLoggedIn) {
        write(connfd, "221", 4);
    } else {
        isLoggedIn = 0;
        write(connfd, "130", 4);
    }
}

void handleRequest(int connfd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(connfd, buffer, sizeof(buffer));
    char* command = strtok(buffer, " ");
    if (strcmp(command, "USER") == 0) {
        char* username = strtok(NULL, " ");
        handleLogin(connfd, username);
    } else if (strcmp(command, "POST") == 0) {
        char* article = strtok(NULL, " ");
        handlePost(connfd, article);
    } else if (strcmp(command, "BYE") == 0) {
        handleLogout(connfd);
    } else {
        write(connfd, "300", 4);
    }
}

void handleConnection(int connfd) {
    write(connfd, "100", 4);
    while (1) {
        handleRequest(connfd);
        if (!isLoggedIn) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please provide a port number.\n");
        return 1;
    }

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    } else {
        printf("Socket successfully created.\n");
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed.\n");
        exit(0);
    } else {
        printf("Socket successfully binded.\n");
    }

    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed.\n");
        exit(0);
    } else {
        printf("Server listening.\n");
    }

    socklen_t len = sizeof(cli);

    while (1) {
        connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (connfd < 0) {
            printf("Server accept failed.\n");
            exit(0);
        } else {
            printf("Server accepted the client.\n");
        }

        if (fork() == 0) {
            close(sockfd);
            handleConnection(connfd);
            close(connfd);
            exit(0);
        } else {
            close(connfd);
        }
    }

    return 0;
}
