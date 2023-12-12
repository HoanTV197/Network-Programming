

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_USERNAME_LEN 50

typedef struct {
    char username[MAX_USERNAME_LEN];
    int status;
} Account;

int checkLogin = 0;

void Login(int sockfd, char* username) {
    char buffer[1024];
    sprintf(buffer, "USER %s", username);
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "100") == 0) {
        checkLogin = 1;
        printf("Login successful.\n");
    } else if (strcmp(buffer, "211") == 0) {
        printf("Account is locked.\n");
    } else if (strcmp(buffer, "212") == 0) {
        printf("Account not exist.\n");
    } else {
        printf("Not response from server.\n");
    }
}

void Post(int sockfd) {
    char buffer[1024];
    printf("Enter your message: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "120") == 0) {
        printf("Message posted successful.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You need to log in first.\n");
    } else {
        printf("Unknown response from server.\n");
    }
}

void Logout(int sockfd) {
    char buffer[1024];
    sprintf(buffer, "BYE");
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));

    if (strcmp(buffer, "130") == 0) {
        checkLogin = 0;
        printf("Log out successful.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You are not logged in.\n");
    } else {
        printf("Unknown response from server.\n");
    }
}

void Menu(int sockfd) {
    while (1) {
        printf("1. Log in\n");
        printf("2. Post message\n");
        printf("3. Logout\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        getchar(); 

        switch (choice) {
            case 1:
                if (checkLogin) {
                    printf("You are already logged in.\n");
                } else {
                    char username[MAX_USERNAME_LEN];
                    printf("Enter your username: ");
                    fgets(username, sizeof(username), stdin);
                    username[strcspn(username, "\n")] = 0;
                    Login(sockfd, username);
                }
                break;

            case 2:
                if (!checkLogin) {
                    printf("You need to log in first.\n");
                } else {
                    Post(sockfd);
                }
                break;

            case 3:
                if (!checkLogin) {
                    printf("You are not logged in.\n");
                } else {
                    Logout(sockfd);
                }
                break;
                
            case 4:
                return;
            default:
                printf("Try again.\n");
                break;
        }
    }
}


int main(int argc, char* argv[]) {

    //Step 1: Check if the server IP address and port number
    if (argc < 3) {
        printf("Please provide the server IP address and port number.\n");
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr;

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
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    //Step 3: Connect the socket to the server address.
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection to the server failed.\n");
        exit(0);
    } else {
        printf("Connected to the server.\n");
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));

    //Step 4: Read the response from the server.
    if (strcmp(buffer, "100") == 0) {
        Menu(sockfd);
    } else {
        printf("Unknown response from server.\n");
    }

    close(sockfd);

    return 0;
}

