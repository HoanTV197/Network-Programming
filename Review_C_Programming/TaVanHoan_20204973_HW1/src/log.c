#include "../headers/log.h"
#include <stdio.h>
#include <time.h>

/**
* @function writeLog: This function writes a log entry to a file.
* @param function: An integer representing the function that is being logged.
* @param value: A pointer to a string representing the value that is being logged.
* @param result: 1 success (+OK),
                 other value fail (-ERR).
**/

void writeLog(int function, char* value, int result) {
    FILE *f = fopen("log_20204973.txt", "a");
    if (f == NULL) {
        printf("Cannot open log file.\n");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(f, "[%02d/%02d/%d %02d:%02d:%02d] $ %d $ %s $ %s\n",
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            function, value, result == 1 ? "+OK" : "-ERR");
    fclose(f);
}