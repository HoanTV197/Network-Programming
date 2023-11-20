#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_USERNAME_LEN 50

typedef struct {
    char username[MAX_USERNAME_LEN];
    int status;
} Account;

int isLoggedIn = 0;

/**
* @function Login: This function performs the login operation with a given username.
* @param sockfd: An integer representing the socket file descriptor.
* @param username: A pointer to a string representing the username.
**/
void Login(int sockfd, char* username) {
    char buffer[1024];
    sprintf(buffer, "USER %s", username);
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "100") == 0) {
        isLoggedIn = 1;
        printf("Login successful.\n");
    } else if (strcmp(buffer, "211") == 0) {
        printf("Account is locked.\n");
    } else if (strcmp(buffer, "212") == 0) {
        printf("Account does not exist.\n");
    } else {
        printf("Not response from the server.\n");
    }
}

/**
* @function Post: This function allows the user to post a message.
* @param sockfd: An integer representing the socket file descriptor.
**/
void Post(int sockfd) {
    char buffer[1024];
    printf("Enter message: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    write(sockfd, buffer, strlen(buffer));

    // Read the server's response
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));

    if (strcmp(buffer, "120") == 0) {
        printf("Posted successfully.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You need log in first.\n");
    } else if (strcmp(buffer, "300") == 0) {
        printf("Invalid message format.\n");
    } else {
        printf("Unknown response from the server: %s\n", buffer);
    }
}

/**
* @function Logout: This function performs the logout operation.
* @param sockfd: An integer representing the socket file descriptor.
**/
void Logout(int sockfd) {
    char buffer[1024];
    sprintf(buffer, "BYE");
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "130") == 0) {
        isLoggedIn = 0;
        printf("Logout successful.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You are not logged in.\n");
    } else {
        printf("Unknown response from the server.\n");
    }
}

/**
* @function Menu: This function displays a menu to the user and handles their choices.
* @param sockfd: An integer representing the socket file descriptor.
**/
void Menu(int sockfd) {
    while (1) {
        printf("1. Log in\n");
        printf("2. Post message\n");
        printf("3. Logout\n");
        printf("4. Exit\n");
        printf("Input your choice: ");
        char choice[5];
        fgets(choice, sizeof(choice), stdin);
        choice[strcspn(choice, "\n")] = 0;

        if (strcmp(choice, "1") == 0) {
            if (isLoggedIn) {
                printf("You are already logged in.\n");
            } else {
                char username[MAX_USERNAME_LEN];
                printf("Enter username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = 0;
                Login(sockfd, username);
            }
        } else if (strcmp(choice, "2") == 0) {
            if (!isLoggedIn) {
                printf("You need log in first.\n");
            } else {
                Post(sockfd);
            }
        } else if (strcmp(choice, "3") == 0) {
            if (!isLoggedIn) {
                printf("You aren't logged in.\n");
            } else {
                Logout(sockfd);
            }
        } else if (strcmp(choice, "4") == 0) {
            return;
        } else {
            printf("Please try again.\n");
        }
    }
}

/**
* @function main: The main function of the program. It establishes a connection to the server and calls the Menu function.
* @param argc: An integer representing the number of command-line arguments.
* @param argv: An array of pointers to strings representing the command-line arguments.
* @return: Returns 0 if the program executes successfully, and 1 otherwise.
**/
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Please provide the server IP and port .\n");
        return 1;
    }

    char* server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    } else {
        printf("Socket successful created.\n");
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_ip);
    servaddr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection to the server failed.\n");
        exit(0);
    } else {
        printf("Connected to the server on port %d.\n", port);
    }

    Menu(sockfd);
    
    close(sockfd);

    return 0;
}


