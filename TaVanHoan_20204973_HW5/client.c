
// Client code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_USERNAME_LEN 50
#define MAX_LOG_LEN 500

int isLoggedIn = 0;

void handleLogin(int sockfd, char* username) {
    char buffer[1024];
    sprintf(buffer, "USER %s", username);
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "110") == 0) {
        isLoggedIn = 1;
        printf("Logged in successfully.\n");
    } else if (strcmp(buffer, "211") == 0) {
        printf("Account is locked.\n");
    } else if (strcmp(buffer, "212") == 0) {
        printf("Account does not exist.\n");
    } else {
        printf("Unknown response from server.\n");
    }
}

void handleLogout(int sockfd) {
    char buffer[1024];
    sprintf(buffer, "BYE");
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "130") == 0) {
        isLoggedIn = 0;
        printf("Logged out successfully.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You are not logged in.\n");
    } else {
        printf("Unknown response from server.\n");
    }
}

void handlePost(int sockfd) {
    char buffer[1024];
    printf("Enter your message: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
char postBuffer[1024 + 5]; // Increase the size of postBuffer
snprintf(postBuffer, sizeof(postBuffer), "POST %s", buffer);


    write(sockfd, postBuffer, strlen(postBuffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "120") == 0) {
        printf("Message posted successfully.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You need to log in first.\n");
    } else {
        printf("Unknown response from server.\n");
    }
}

void handleMenu(int sockfd) {
    while (1) {
        printf("1. Log in\n");
        printf("2. Post message\n");
        printf("3. Logout\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        getchar(); // consume newline character
        switch (choice) {
            case 1:
                if (isLoggedIn) {
                    printf("You are already logged in.\n");
                } else {
                    char username[MAX_USERNAME_LEN];
                    printf("Enter your username: ");
                    fgets(username, sizeof(username), stdin);
                    username[strcspn(username, "\n")] = 0;
                    handleLogin(sockfd, username);
                }
                break;
            case 2:
                if (!isLoggedIn) {
                    printf("You need to log in first.\n");
                } else {
                    handlePost(sockfd);
                }
                break;
            case 3:
                if (!isLoggedIn) {
                    printf("You are not logged in.\n");
                } else {
                    handleLogout(sockfd);
                }
                break;
            case 4:
                return;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Please provide the server IP address and port number.\n");
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    } else {
        printf("Socket successfully created.\n");
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection to the server failed.\n");
        exit(0);
    } else {
                printf("Connected to the server.\n");
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "100") == 0) {
        handleMenu(sockfd);
    } else {
        printf("Unknown response from server.\n");
    }

    close(sockfd);

    return 0;
}
