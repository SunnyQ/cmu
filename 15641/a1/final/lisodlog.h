/*
 * This serves as the header file of the lisod
 * log engine.
 *
 *      Author: Yang Sun (yksun@cs.cmu.edu)
 */

#ifndef __LISODLOG_H__
#define __LISODLOG_H__

/******** MACRO variable **********/

#define DEBUG "DEBUG: "
#define WARNING "WARNING: "
#define ERROR "ERROR: "
#define MAX_MSG 350

/******** MACRO variable **********/

/******** global variable **********/
char *log_file;
char log_msg[MAX_MSG];
FILE *log_fp;
/******** global variable **********/

/******** function header **********/
char *current_time();
int write_log(char *status);
/******** function header **********/

#endif /* __LISODLOG_H__ */
