#include "../headers/user_actions.h"
#include "../headers/account.h"
#include "../headers/log.h"
#include <stdio.h>
#include <string.h>

Account currentUser;
int isLoggedIn = 0;

/**
* @function login: This function handles the login process.
* It asks for a username, checks if the user is already logged in, checks if the account exists and if it's not banned.
* If OK, it sets the "currentUser" and ""isLoggedIn" variables.
**/
void login() {
    char username[MAX_USERNAME_LEN];
    printf("Username: ");
    scanf("%s", username);
    getchar();

    if (isLoggedIn) {
        printf("You have already logged in.\n");
        writeLog(1, username, 0);
        return;
    }

    Account a = findAccount(username);
    if (a.status == -1) {
        printf("Account is not exist.\n");
        writeLog(1, username, 0);
        return;
    }

    if (a.status == 0) {
        printf("Account is banned.\n");
        writeLog(1, username, 0);
        return;
    }

    currentUser = a;
    isLoggedIn = 1;
    printf("Hello %s\n", currentUser.username);
    writeLog(1, username, 1);
}

/**
* @function postMessage: This function allows the user to post a message.
* It checks if the user is logged in. If so, it asks for a message and posts it.
**/
void postMessage() {
    char message[MAX_LOG_LEN];
    printf("Post message: ");
    fflush(stdin); 
    fgets(message, MAX_LOG_LEN, stdin);
    message[strcspn(message, "\n")] = 0;

    if (!isLoggedIn) {
        printf("You have not logged in.\n");
        writeLog(2, message, 0);
        return;
    }

    printf("Successful post\n");
    writeLog(2, message, 1);
}


/**
* @function logout: This function handles the logout process.
* It checks if the user is logged in. If so, it logs out the user.
**/
void logout() {
    if (!isLoggedIn) {
        printf("You have not logged in.\n");
        writeLog(3, "", 0);
        return;
    }

    isLoggedIn = 0;
    printf("Successful log out\n");
    writeLog(3, "", 1);
}
