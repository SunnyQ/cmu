#ifndef __LISODLOG_H__
#define __LISODLOG_H__

#define DEBUG "DEBUG: "
#define WARNING "WARNING: "
#define ERROR "ERROR: "
#define MAX_MSG 350

char *log_file;
FILE *log_fp;
char *log_msg;

char *current_time();
int write_log(char *status, char *msg);

#endif /* __LISODLOG_H__ */
