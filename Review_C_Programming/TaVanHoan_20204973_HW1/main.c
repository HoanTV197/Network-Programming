#include "headers/account.h"
#include "headers/log.h"
#include "headers/user_actions.h"
#include <stdio.h>

int main() {
   int choice;

   do {
       printf("1. Log in\n");
       printf("2. Post message\n");
       printf("3. Logout\n");
       printf("4. Exit\n");
       scanf("%d", &choice);

       switch(choice) {
           case 1:
               login();
               break;
           case 2:
               postMessage();
               break;
           case 3:
               logout();
               break;
           case 4:
               writeLog(4, "", 1);
               break;
           default:
               printf("\nPlease try again.\n");
       }
   } while(choice != 4);

   return 0;
}