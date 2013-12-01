/* DRerror.c
|
|	Deferred-split R-tree (DR) error handling functions.
+-----------------------------------------------------------------------------*/
#include "DRdef.h"




/* global variable
+-----------------------------------------------------------------------------*/
/* DR error number */
static int	DRerrno = DRE_OK;




/* DR error messages
+-----------------------------------------------------------------------------*/
/* error messages */
static char	*DRerrmsg[] =
{
	"no error",
	"error in PS layer",
	"no memory",
	"file exist",
	"file table full",
	"invalid dimension",
	"invalid coordinate type",
	"invalid DR file descriptor",
	"invalid PS page number",
	"invalid level",
	"invalid page/record number",
	"invalid rectangle",
	"invalid entry index",
	"invalid node",
	"node empty",
	"node full"
};

/* number of messages */
#define	DR_NERRORS	sizeof(DRerrmsg)/sizeof(DRerrmsg[0])




/* error handling functions
+-----------------------------------------------------------------------------*/


/* DRerrSave() -
|
|	Save the last error number.
+-----------------------------------------------------------------------------*/
int DRerrSave(int errno)
{
    return DRerrno = errno;
}


/* DRperror() -
|
|	Print the last error message.
+-----------------------------------------------------------------------------*/
void DRperror(char *msg)
{
	if (DRerrno == DRE_PSERROR)
		PSperror(msg);
	else if (DRerrno <= 0 && DRerrno > -DR_NERRORS)
		fprintf (stderr, "%s: %s\n", msg, DRerrmsg[-DRerrno]);
	else
		fprintf (stderr, "%s: DRerrno corrupted!\n", msg);
}


