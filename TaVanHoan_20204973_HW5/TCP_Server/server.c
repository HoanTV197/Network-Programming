
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

typedef struct {
    char username[50];
    int status;
} Account;

Account currentUser;
int checkLogin = 0;

Account findAccount(char* username) {
    FILE* f = fopen("account.txt", "r+");
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


void Post(int connfd, char* article) {
    if (!checkLogin) {
        write(connfd, "221", 4);
        printf("221\n");
    } else {
        write(connfd, "120", 4);
        printf("120\n");
    }
}


void Logout(int connfd) {
    if (!checkLogin) {
        write(connfd, "221", 4);
        printf("221\n");
    } else {
        checkLogin = 0;
        write(connfd, "130", 4);
        printf("130\n");
    }
}


void Request(int connfd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(connfd, buffer, sizeof(buffer));

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


void Connection(int connfd) {
    write(connfd, "100", 4);
    while (1) {
        Request(connfd);
        if (!checkLogin) {
            break;
        }
    }
}


int main(int argc, char* argv[]) {

    // Step 1: Check if the port number is provided as a command-line argument.
    if (argc < 2) {
        printf("Please provide a port_number.\n");
        return 1;
    }

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    //Step 2: Create a socket using the AF_INET (IPv4)
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
    servaddr.sin_port = htons(atoi(argv[1]));

    //Step 3: Bind the socket to the server address.
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed.\n");
        exit(0);
    } else {
        printf("Socket successful binded.\n");
    }

    //Step 4: Listen for incoming connections.
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed.\n");
        exit(0);
    } else {
        printf("Server listenning.\n");
    }

    socklen_t len = sizeof(cli);

    //*Step 5: Accept incoming connections in a loop.
    while (1) {
        connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (connfd < 0) {
            printf("Server accept failed.\n");
            exit(0);
        } else {
            printf("100\n");
        }


        if (fork() == 0) {
            close(sockfd);
            Connection(connfd);
            close(connfd);
            exit(0);
        } else {
            close(connfd);
        }
    }


    return 0;
}
// end server.c


