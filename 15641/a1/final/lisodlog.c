/*
 * This serves as the lisod log engine.It simply dumps
 * the system DEBUG, ERROR and WARNING messages to a
 * log file.
 *
 *      Author: Yang Sun (yksun@cs.cmu.edu)
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "lisodlog.h"
#include "errno.h"

/* return the current system time in GMT format */
char *current_time()
{
    char *datetime;
    time_t t;

    datetime = malloc(200);
    t = time(NULL);
    strftime(datetime, 200, "%a, %d %b %Y %T GMT", localtime(&t));
    return datetime;
}

/* write to the log directly without optional params */
int write_log(char *status)
{
    char line[500];
    char *datetime;
    int length;

    if (!(log_fp = fopen(log_file, "a")))
        return E_FILEOPEN;

    datetime = current_time();
    strncpy(line, datetime, strlen(datetime));
    strncat(line, "\t", 1);
    length = (strlen(status) > 50) ? 50 : strlen(status);
    strncat(line, status, length);
    length = (strlen(log_msg) > 350) ? 350 : strlen(log_msg);
    strncat(line, log_msg, length);
    strncat(line, "\n", 1);
    length = fwrite(line, 1, strlen(line), log_fp);
    memset(log_msg, 0, MAX_MSG);
    memset(line, 0, 500);
    fclose(log_fp);
    return length;
}

