/* PSerror.c
|
|	Page Store (PS) error handling functions.
+-----------------------------------------------------------------------------*/
#include "PSdef.h"




/* global variable
+-----------------------------------------------------------------------------*/
/* PS error number */
static int	PSerrno = PSE_OK;




/* PS error messages
+-----------------------------------------------------------------------------*/
/* error messages */
static char	*PSerrmsg[] =
{
    "no error",
    "out of memory",
    "out of buffer space",
    "page fixed in buffer",
    "page is not in buffer",
    "incomplete read of page from file",
    "incomplete write of page to file",
    "incomplete read of header from file",
    "incomplete write of header to file",
    "invalid page number",
    "file still open",
    "file table full",
    "invalid file descriptor",
    "end of file",
    "page already free",
    "page already unfixed",
    "new page to be allocated already in buffer",
    "hash table entry not found",
    "page already in hash table"
};

/* number of messages */
#define PS_NERRORS	sizeof(PSerrmsg)/sizeof(PSerrmsg[0])




/* error handling functions
+-----------------------------------------------------------------------------*/


/* PSerrSave() -
|
|	Save the last error number.
+-----------------------------------------------------------------------------*/
int PSerrSave(int errno)
{
	return PSerrno = errno;
}


/* PSperror() -
|
|	Print the last error message.
+-----------------------------------------------------------------------------*/
void PSperror(char *msg)
{
	if (PSerrno == PSE_UNIX)
		perror(msg);
	else if (PSerrno <= 0 && PSerrno > -PS_NERRORS)
		fprintf(stderr, "%s: %s\n", msg, PSerrmsg[-PSerrno]);
	else
		fprintf(stderr, "%s: PSerrno corrupted!\n", msg);
}


