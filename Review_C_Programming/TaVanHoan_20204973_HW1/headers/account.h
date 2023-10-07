#ifndef ACCOUNT_H
#define ACCOUNT_H

#define MAX_USERNAME_LEN 50

typedef struct {
    char username[MAX_USERNAME_LEN];
    int status;
} Account;

Account findAccount(char* username);

#endif 
