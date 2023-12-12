#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_USERNAME_LEN 50

typedef struct {
    char username[MAX_USERNAME_LEN];
    int status;
} Account;

int checkLogin = 0;

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
        // Timeout, no data received
        return;
    } else {
        int bytesRead = read(sockfd, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                printf("Server disconnected\n");
            } else {
                perror("read");
            }
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
    }

    if (strcmp(buffer, "100") == 0) {
        checkLogin = 1;
        printf("Login successful.\n");
    } else if (strcmp(buffer, "211") == 0) {
        printf("Account is locked.\n");
    } else if (strcmp(buffer, "212") == 0) {
        printf("Account does not exist.\n");
    } else {
        printf("No response from the server.\n");
    }
}

/**
* @function Post: This function allows the user to post an article to the server.
* @param sockfd: An integer representing the socket file descriptor.
* @param article: A pointer to a string representing the article to be posted.
**/
void Post(int sockfd, char* article) {
    if (!checkLogin) {
        printf("You need to log in first.\n");
        return;
    }

    char buffer[1024];
    sprintf(buffer, "POST %s", article);
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));

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
        // Timeout, no data received
        return;
    } else {
        int bytesRead = read(sockfd, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                printf("Server disconnected\n");
            } else {
                perror("read");
            }
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
    }

    if (strcmp(buffer, "120") == 0) {
        printf("Message posted successful.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You need to log in first.\n");
    } else {
        printf("Unknown response from server.\n");
    }
}

/**
* @function Logout: This function allows the user to log out from the server.
* @param sockfd: An integer representing the socket file descriptor.
**/
void Logout(int sockfd) {
    char buffer[1024];
    sprintf(buffer, "BYE");
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));

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
        // Timeout, no data received
        return;
    } else {
        int bytesRead = read(sockfd, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                printf("Server disconnected\n");
            } else {
                perror("read");
            }
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
    }

    if (strcmp(buffer, "130") == 0) {
        checkLogin = 0;
        printf("Logout successful.\n");
    } else if (strcmp(buffer, "221") == 0) {
        printf("You are not logged in.\n");
    } else {
        printf("Unknown response from the server.\n");
    }
}

/**
* @function Menu: This function displays a menu for the user to interact with the server.
* @param sockfd: An integer representing the socket file descriptor.
**/
void Menu(int sockfd) {
    char article[1024];
    int loggedIn = 0;  //
    char loggedInUsername[MAX_USERNAME_LEN]; 

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
                char username[MAX_USERNAME_LEN];
                printf("Enter your username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = 0;
                if (loggedIn) {
                    printf("You are already logged in as %s.\n", loggedInUsername);
                } else {
                    Login(sockfd, username);
                    if (checkLogin) {
                        loggedIn = 1;
                        strcpy(loggedInUsername, username);
                    }
                }
                break;

            case 2:
                if (!loggedIn) {
                    printf("You need to log in first.\n");
                } else {
                    printf("Enter your message: ");
                    fgets(article, sizeof(article), stdin);
                    article[strcspn(article, "\n")] = 0;
                    Post(sockfd, article);
                }
                break;

            case 3:
                if (!loggedIn) {
                    printf("You are not logged in.\n");
                } else {
                    Logout(sockfd);
                    loggedIn = 0;
                    loggedInUsername[0] = '\0';  
                }
                break;

            case 4:
                return;

            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }
}



/**
* @function main: The main function of the program. It establishes a connection to the server and starts the menu.
* @param argc: An integer representing the number of command-line arguments.
* @param argv: An array of pointers to strings representing the command-line arguments.
* @return: Returns 0 on successful execution and 1 if there is an error.
**/
int main(int argc, char* argv[]) {
    // Check if the server IP address and port number are provided.
    if (argc < 3) {
        printf("Please provide the server IP address and port number.\n");
        return 1;
    }

     // Create a socket.
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    // Connect to the server.
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connection to the server failed");
        exit(EXIT_FAILURE);
    }

    // Read the response from the server
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));

    if (strcmp(buffer, "100") == 0) {
        Menu(sockfd);
    } else {
        printf("Unknown response from server.\n");
    }

    close(sockfd);

    return 0;
}
