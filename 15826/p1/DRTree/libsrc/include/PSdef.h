/* PSdef.h
|
|	Page Store (PS) data type definitions.
+-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include "PS.h"




/* definitions of PS file header
+-----------------------------------------------------------------------------*/
/* structure of PS file header */
typedef struct PShdr
{
	int	pagesize;		/* size of each page */
					/* NOTE: later we'll be able to */
					/* specify the page size when we */
					/* create a PS file */
	int	numpages;		/* number of pages in the file */
	int	firstfree;		/* first page in the free list */

}	PShdr;				/* PS file header */

/* size of PS file header */
#define	PS_HDR_SIZE	sizeof(PShdr)




/* definitions of PS file page
+-----------------------------------------------------------------------------*/
/* structure of PS file page */
typedef struct PSfpage
{
	int	nextfree;		/* next page in the free list */
					/* NOTE: see below for valid value */
	char	pagebuf			/* page buffer */
		[PS_PAGEBUF_SIZE];	/* NOTE: see the above NOTE */

}	PSfpage;			/* PS file page */

/* size of PS file page */
#define	PS_FPAGE_SIZE	sizeof(PSfpage)

/* constants used in "nextfree" */
#define	PS_FPAGE_LIST_END	-1	/* end of the free list */
#define	PS_FPAGE_USED		-2	/* used page */




/* definitions of PS buffer page
+-----------------------------------------------------------------------------*/
/* structure of PS buffer page */
typedef struct PSbpage
{
struct	PSbpage	*nextpage;	/* next in the list of buffer pages */
struct	PSbpage	*prevpage;	/* prev in the list of buffer pages */
	PSfpage	fpage;		/* PS file page */
	int	pfd;		/* PS file desciptor */
	int	pageno;		/* page number */
	bool_t	dirty;		/* TRUE if page is modified */
	short	fixes;		/* fix count */

}	PSbpage;		/* PS buffer page */

/* size of PS buffer page */
#define	PS_BPAGE_SIZE	sizeof(PSbpage)




/* definitions of PS file table entry
+-----------------------------------------------------------------------------*/
/* structure of PS file table entry */
typedef struct PSftab_entry
{
	ino_t	inode;		/* inode number of PS file */
	int	unixfd;		/* unix file descriptor	of PS file */
	PShdr	hdr;		/* PS file header */
	bool_t	hdrchanged;	/* TRUE if file header has changed */
	bool_t	valid;		/* TRUE if entry is valid */

}	PSftab_entry;		/* PS file table entry */

/* size of PS file table */
#define PS_FTAB_SIZE	PS_MAXFILES




/* definitions of PS hash facility
+-----------------------------------------------------------------------------*/
/* structure of PS hash table bucket entry */
typedef struct PShash_entry
{
struct	PShash_entry	*next;	/* next hash table bucket */
struct	PShash_entry	*prev;	/* prev hash table bucket */
	PSbpage		*bpage;	/* pointer to PS buffer page */
	int		pfd;	/* PS file descriptor */
	int		pageno;	/* PS page number */

}	PShash_entry;		/* PS hash table bucket entry */

/* size of PS hash table bucket entry */
#define	PS_HBUCKET_SIZE	sizeof(PShash_entry)

/* size of PS hash table */
#define	PS_HTAB_SIZE	20

/* PS hash function */
#define	PShash(pfd, pageno) \
	(((pfd)+(pageno)) % PS_HTAB_SIZE)




/* prototypes of PSbuf and PShash functions
+-----------------------------------------------------------------------------*/
#include "PSbuf.h"
#include "PShash.h"




