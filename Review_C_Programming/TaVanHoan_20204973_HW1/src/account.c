#include "../headers/account.h"
#include <stdio.h>
#include <string.h>

/**
* @function findAccount: This function finds an account with a given username in the "account.txt" file.
* @param username: A pointer to a string representing the username of the account.
* @return: If the account is found, it returns the account. If not found or an error occurs,
 it returns an Account with username as an empty string and status as -1.
**/
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