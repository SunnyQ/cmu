/* DRdef.h
|
|	Deferred-split R-tree (DR) data type definitions.
+-----------------------------------------------------------------------------*/
/* added stdlib.h , christos march 5, 2011 */
#include <stdio.h>
#include <stdlib.h>    
#include "PS.h"




/* system default constant
+-----------------------------------------------------------------------------*/
/* the maximum number of index files that can be opened */
#define	DR_MAXFILES	PS_MAXFILES




/* definitions of supporting types
+-----------------------------------------------------------------------------*/
/* coordinate type */
typedef enum { DR_INT, DR_FLOAT }	coord_t;

/* rectangle */
typedef	char	DRrect;




/* DR error numbers
+-----------------------------------------------------------------------------*/
#define	DRE_OK			0
#define	DRE_PSERROR		-1
#define	DRE_NOMEM		-2
#define	DRE_FILEEXIST		-3
#define	DRE_FTABFULL		-4
#define	DRE_DIMINVALID		-5
#define	DRE_CTYPEINVALID	-6
#define	DRE_RFDINVALID		-7
#define	DRE_PAGEINVALID		-8
#define	DRE_LEVELINVALID	-9
#define	DRE_CHILDINVALID	-10
#define	DRE_RECTINVALID		-11
#define	DRE_ENTRYINVALID	-12
#define	DRE_NODEINVALID		-13
#define	DRE_NODEEMPTY		-14
#define	DRE_NODEFULL		-15


/* definitions of DR index header
+-----------------------------------------------------------------------------*/
/* structure of DR index header */
typedef struct DRhdr
{
	int	dim;		/* dimension of the indexed space */
	coord_t	ctype;		/* coordinate type of indexed objects */
	int	rectsize;	/* size of a rectangle */
	int	entrysize;	/* size of an entry */
	int	nodecard;	/* number of entry slots in a node */
	int	bmpsize;	/* size of the bitmap used to indicate */
				/* the usage of entry slots in a node */
	int	rootpage;	/* PS page number of the root node */
	int	rootlevel;	/* level of the root node in the index tree */
				/* with leaf node at level 0 */
}	DRhdr;			/* DR index header */

/* size of DR index header */
#define	DR_HDR_SIZE	sizeof(DRhdr)




/* definitions of DR index node
+-----------------------------------------------------------------------------*/
/* structure of DR node header */
typedef struct DRnode_hdr
{
	int	level;		/* level of the node in the index tree */
				/* with leaf node at level 0 */
	int	count;		/* number of entries in the node */

}	DRnode_hdr;		/* DR node header */

/* size of DR node header */
#define	DR_NODEHDR_SIZE	sizeof(DRnode_hdr)


/* the structure of a DR node is as the follows:
	1. the DR node header;
	2. an array of entry slots with the array size to be "nodecard";
	3. an array of bytes(bitmap) with each bit to indicate the usage
	   of a slot, there may be some unused bits in the last byte; */
/* a DR node fits exactly in the data buffer of a PS page */

/* the structure of an entry is as the follows:
	1. an child number, if the entry is in a leaf node the number is
	   the id number of an indexed object, otherwise it's the page
	   number of the sub-tree root node;
	2. a rectangle, it's the cover rectangle of an indexed object or
	   the sub-tree root node; */
/* an entry fits exactly in an entry slot in a DR node */

/* the structure of a rectangle is as the follows:
	1. an array of coordinates of all the lower sides in each dimension;
	2. an array of coordinates of all the higher sides in each dimension; */
/* a rectangle fits exactly in an array of "char"s of size "rectsize" */
/* the coordinates of a rectangle can be "int" or "float" */




/* definitions of DR split facility (for quadratic splitting)
+-----------------------------------------------------------------------------*/
/* DR partition structure */
typedef struct DRpart
{
	int	*taken;		/* an array of size "nodecard+1" indicating */
				/* which partition takes an entry in a split */
				/* -1 for untaken, 0 for part0, 1 for part1 */
	double	*area;		/* an array of size "nodecard+1" of cover */
				/* rectangle area of each entry in a split */
	DRrect	*splitcov;	/* rectangle covering all entries in a split */
	double	splitarea;	/* area of "splitcov" */
	int	partcount[2];	/* number of entries in each partition */
	DRrect	*partcov[2];	/* cover rectangle of each partition */
	double	partarea[2];	/* area of "partcov" */

}	DRpart;			/* DR partition structure */




/* definitions of DR index table
+-----------------------------------------------------------------------------*/
/* structure of DR index table entry */
typedef struct DRftab_entry
{
	int	pfd;		/* PS file descriptor of DR index */
	DRhdr	hdr;		/* DR index header */
	bool_t	hdrChanged;	/* TRUE if header has changed */
	DRrect	*coverall;	/* rectangle covering the whole index */
	DRrect	*newrect;	/* rect buffer used when root has splitted */
	DRrect	*temprect;	/* temporary rectangle */
	DRpart	part;		/* DR partition structure */
	bool_t	valid;		/* TRUE if entry is valid */

}	DRftab_entry;		/* DR index table entry */

/* size of DR index table */
#define	DR_FTAB_SIZE	DR_MAXFILES

/* DR index table as a global variable */
extern DRftab_entry	DRftab[];




/* supporting macros
+-----------------------------------------------------------------------------*/
/* macro to test if a DR file descriptor is invalid */
#define	DRfdInvalid(fd) \
	((fd) < 0 || (fd) >= DR_FTAB_SIZE || DRftab[(fd)].valid == FALSE)

/* macro to access "dim" */
#define	DRdim(fd)		(DRftab[(fd)].hdr.dim)

/* macro to access "ctype" */
#define DRcoordType(fd)		(DRftab[(fd)].hdr.ctype)

/* macro to access "rectsize" */
#define DRrectSize(fd)		(DRftab[(fd)].hdr.rectsize)

/* macro to access "entrysize" */
#define DRnodeEntrySize(fd)	(DRftab[(fd)].hdr.entrysize)

/* macro to access "nodecard" */
#define	DRnodeCard(fd)		(DRftab[(fd)].hdr.nodecard)

/* macro to access "rootpage" */
#define	DRrootPage(fd)		(DRftab[(fd)].hdr.rootpage)

/* macro to access "rootlevel" */
#define	DRrootLevel(fd)		(DRftab[(fd)].hdr.rootlevel)

/* macro to access the level of a DR node */
#define DRnodeLevel(fd, pagebuf) \
	(((DRnode_hdr*)(pagebuf))->level)

/* macro to access the entry count of a DR node */
#define DRnodeCount(fd, pagebuf) \
	(((DRnode_hdr*)(pagebuf))->count)

/* macro to access the bitmap of a DR node */
#define	DRnodeBitmap(fd, pagebuf) \
	((pagebuf) + DR_NODEHDR_SIZE + \
	DRftab[(fd)].hdr.nodecard * DRftab[(fd)].hdr.entrysize)

/* macro to access the n-th child of a DR node */
#define DRnodeChild(fd, n, pagebuf) \
	(*(int*)((pagebuf) + DR_NODEHDR_SIZE + (n)* DRftab[(fd)].hdr.entrysize))

/* macro to access the n-th rect of a DR node */
#define	DRnodeRect(fd, n, pagebuf) \
	((DRrect*)((pagebuf) + DR_NODEHDR_SIZE + \
	(n)* DRftab[(fd)].hdr.entrysize + sizeof(int)))




/* prototypes of supporting functions
+-----------------------------------------------------------------------------*/
#include "DRrect.h"
#include "DRnode.h"
#include "DRtree.h"
#include "DRerror.h"




