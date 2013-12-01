/* DR.c
|
|	Deferred-split R-tree (DR) interface functions.
+-----------------------------------------------------------------------------*/
#include <unistd.h>	/* for access() */
#include "DRdef.h"




/* global variable
+-----------------------------------------------------------------------------*/
/* DR file table */
DRftab_entry	DRftab[DR_FTAB_SIZE];




/* supporting functions
+-----------------------------------------------------------------------------*/


/* DRftabFindFree() -
|
|	Find a free entry in the DR file table.
|
|	Return Value:
|	>= 0	index number for a free entry in the file table;
|	< 0	file table is full;
+-----------------------------------------------------------------------------*/
static int DRftabFindFree(void)
{
	int	i;

	/* scan through the file table to find a free entry */
	for (i = 0; i < DR_FTAB_SIZE && DRftab[i].valid != FALSE; i++);

	/* if there is no such entry */
	if (i == DR_FTAB_SIZE)
		return -1;

	/* find an entry */
	return i;
}




/* interface functions
+-----------------------------------------------------------------------------*/


/* DR_Init() -
|
|	Initialize the DR layer, including PS layer initialization and
|	DR file table initialization.
|
|	Return Value:
|	none;
+-----------------------------------------------------------------------------*/
void DR_Init(void)
{
	int	i;

	/* initialize the PS layer */
	PS_Init();

	/* initialize the DR file table */
	for (i = 0; i < DR_FTAB_SIZE; i++)
		DRftab[i].valid = FALSE;
}


/* DR_CreateIndex() -
|
|	Create an index file with name "filename", space dimension "dim"
|	and coordinate type "ctype".
|
|	Return Value:
|	= 0	success;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DR_CreateIndex(
	char	*filename,	/* index file name */
	int	dim,		/* dimension of indexed space */
	coord_t	ctype)		/* coordinate type */
{
	int	pfd;		/* PS file descriptor */
	int	pageno;		/* PS file page number */
	char	*pagebuf;	/* pointer to page buffer */
	DRhdr	hdr;		/* DR header structure */

	/*--------------------------------------------------------------------*/
	/* check "filename" */
	if (access(filename, F_OK) == 0)
		return DRerrSave(DRE_FILEEXIST);

	/* check "dim", it must be greater than 0 */
	if (dim < 1)
		return DRerrSave(DRE_DIMINVALID);

	/* check "ctype" */
	if (ctype != DR_INT && ctype != DR_FLOAT)
		return DRerrSave(DRE_CTYPEINVALID);

	/*--------------------------------------------------------------------*/
	/* create the PS file */
	if (PS_CreateFile(filename) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* open the PS file */
	if ((pfd = PS_OpenFile(filename)) < 0)
		return DRerrSave(DRE_PSERROR);

	/* allocate the header page */
	if (PS_AllocPage(pfd, &pageno, &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/*--------------------------------------------------------------------*/
	/* initialize the header structure */
	hdr.dim = dim;
	hdr.ctype = ctype;

	/* "rectsize" = number_of_sides * side_size */
	if (ctype == DR_FLOAT)
		hdr.rectsize = 2*dim*sizeof(float);
	else	/* ctype == DR_INT */
		hdr.rectsize = 2*dim*sizeof(int);

	/* "entrysize" = pageno_or_recno_size + rect_size */
	hdr.entrysize = sizeof(int) + hdr.rectsize;

	/* "nodecard" = byte_size * (pagebuf_size - page_header_size) */
	/*		/ (byte_size * entry_size + 1) */
	hdr.nodecard = 8 * (PS_PAGEBUF_SIZE - DR_NODEHDR_SIZE) /
			(8 * hdr.entrysize + 1);

	/* "bmpsize" = ("nodecard" + 7) / 8 */
	hdr.bmpsize = (hdr.nodecard + 7) >> 3;

	/* leaf level */
	hdr.rootlevel = 0;

	/* allocate root page later when we open the index */
	hdr.rootpage = -1;

	/*--------------------------------------------------------------------*/
	/* copy the header structure to the header page */
	memcpy(pagebuf, &hdr, DR_HDR_SIZE);

	/* mark and unfix the header page */
	if (PS_UnfixPage(pfd, pageno, TRUE) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* close the PS file */
	if (PS_CloseFile(pfd) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* success */
	return DRE_OK;
}


/* DR_DestroyIndex() -
|
|	Destroy an index file with name "filename".
|
|	Return Value:
|	= 0	success;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DR_DestroyIndex(
	char	*filename)	/* index file name */
{
	/* destroy the PS file */
	if (PS_DestroyFile(filename) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* success */
	return DRE_OK;
}


/* DR_OpenIndex() -
|
|	Open an index file with name "filename".
|
|	Return Value:
|	>= 0	DR file descriptor;
|	< 0	errors;
+-----------------------------------------------------------------------------*/
int DR_OpenIndex(
	char	*filename)	/* index file name */
{
	int	errno;		/* error number */
	int	rfd;		/* DR file descriptor */
	int	pfd;		/* PS file descriptor */
	int	pageno;		/* PS page number */
	char	*pagebuf;	/* pointer to page buffer */

	/*--------------------------------------------------------------------*/
	/* find a free entry in the DR file table */
	if ((rfd = DRftabFindFree()) < 0)
		return DRerrSave(DRE_FTABFULL);

	/* open the PS file */
	if ((pfd = PS_OpenFile(filename)) < 0)
		return DRerrSave(DRE_PSERROR);

	/* get the header page */
	if (PS_GetFirstPage(pfd, &pageno, &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* copy the header page to the header structure */
	memcpy(&DRftab[rfd].hdr, pagebuf, DR_HDR_SIZE);
	DRftab[rfd].hdrChanged = FALSE;

	/* unfix the header page */
	if (PS_UnfixPage(pfd, pageno, FALSE) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* mark the entry as valid */
	DRftab[rfd].valid = TRUE;

	/* if the index has no page */
	if (DRrootPage(rfd) < 0)
	{
		/* allocate a leaf node for the index */
		DRrootPage(rfd) = DRnodeNew(rfd, 0, &pagebuf);
		DRrootLevel(rfd) = 0;

		/* mark and unfix the node page */
		PS_UnfixPage(pfd, DRrootPage(rfd), TRUE);
		DRftab[rfd].hdrChanged = TRUE;
	}

	/*--------------------------------------------------------------------*/
	/* initialize rest fields of the file table */
	DRftab[rfd].pfd = pfd;
	if ((DRftab[rfd].temprect = DRrectNew(rfd)) == NULL)
		return DRerrSave(DRE_NOMEM);
	if ((DRftab[rfd].newrect = DRrectNew(rfd)) == NULL)
		return DRerrSave(DRE_NOMEM);
	if ((DRftab[rfd].coverall = DRrectNew(rfd)) == NULL)
		return DRerrSave(DRE_NOMEM);

	/* get the root page */
	if (PS_GetThisPage(pfd, DRrootPage(rfd), &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* get "coverall" */
	if ((errno = DRnodeCover(rfd, DRftab[rfd].coverall, pagebuf)) < 0)
		return errno;

	/* unfix the root page */
	if (PS_UnfixPage(pfd, DRrootPage(rfd), FALSE) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* initialize the partition structure */
	if ((DRftab[rfd].part.taken = (int*) malloc((DRnodeCard(rfd) + 1)
			* sizeof(int))) == NULL)
		return DRerrSave(DRE_NOMEM);
	if ((DRftab[rfd].part.area = (double*) malloc((DRnodeCard(rfd) + 1)
			* sizeof(double))) == NULL)
		return DRerrSave(DRE_NOMEM);
	if ((DRftab[rfd].part.splitcov = DRrectNew(rfd)) == NULL)
		return DRerrSave(DRE_NOMEM);
	if ((DRftab[rfd].part.partcov[0] = DRrectNew(rfd)) == NULL)
		return DRerrSave(DRE_NOMEM);
	if ((DRftab[rfd].part.partcov[1] = DRrectNew(rfd)) == NULL)
		return DRerrSave(DRE_NOMEM);

	/* success */
	return rfd;
}


/* DR_CloseIndex() -
|
|	Close the index file with given DR file descriptor "rfd".
|
|	Return Value:
|	= 0	success;
|	< 0	errors;
+-----------------------------------------------------------------------------*/
int DR_CloseIndex(
	int	rfd)		/* DR file descriptor */
{
	int	pageno;		/* PS page number */
	char	*pagebuf;	/* pointer to page buffer */

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* if the header structure has changed */
	if (DRftab[rfd].hdrChanged)
	{
		/* get the header page */
		if (PS_GetFirstPage(DRftab[rfd].pfd, &pageno, &pagebuf)
				!= PSE_OK)
			return DRerrSave(DRE_PSERROR);

		/* copy the header structure to the header page */
		memcpy(pagebuf, &DRftab[rfd].hdr, DR_HDR_SIZE);

		/* mark and unfix the header page */
		if (PS_UnfixPage(DRftab[rfd].pfd, pageno, TRUE) != PSE_OK)
			return DRerrSave(DRE_PSERROR);
		DRftab[rfd].hdrChanged = FALSE;
	}

	/*--------------------------------------------------------------------*/
	/* do some wrap up */
	DRrectFree(rfd, DRftab[rfd].temprect);
	DRrectFree(rfd, DRftab[rfd].newrect);
	DRrectFree(rfd, DRftab[rfd].coverall);
	DRrectFree(rfd, DRftab[rfd].part.splitcov);
	DRrectFree(rfd, DRftab[rfd].part.partcov[0]);
	DRrectFree(rfd, DRftab[rfd].part.partcov[1]);
	free(DRftab[rfd].part.taken);
	free(DRftab[rfd].part.area);

	/* close the PS file */
	if (PS_CloseFile(DRftab[rfd].pfd) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* mark the entry invalid */
	DRftab[rfd].valid = FALSE;

	/* success */
	return DRE_OK;
}


/* DR_SearchEntry()
|
|	Search in the R-tree index of file "rfd" for all leaf entries
|	whose retangles match "testrect" by "testfn". The entries will
|	be output by "outputfn".
|
|	Return Value:
|	>= 0	number of entries found;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DR_SearchEntry(
	int	rfd,		/* DR file descriptor */
	DRrect	*testrect)	/* rect for comparison */
{
	bool_t	(*testfn)();	/* comparison function */
	int	(*outputfn)();	/* rect output function */
	
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check "testrect" */
	if (DRrectInvalid(rfd, testrect))
		return DRerrSave(DRE_RECTINVALID);

	/* initialize "testfn" */
	testfn = DRrectOverlap;

	/* initialize "outputfn" */
	outputfn = DRnodePrintEntry;

	/* search index tree */
	return DRtreeSearchEntry(rfd, DRrootPage(rfd), testrect, testfn, outputfn);
}


/* DR_InsertEntry()
|
|	Insert "childno" and "childcov" as a leaf entry into the R-tree index.
|
|	Return Value:
|	>= 0	success;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DR_InsertEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	childno,	/* record number to insert */
	DRrect	*childcov)	/* cover rect for the record */
{
	int	errno;		/* errro number */
	int	newpage;	/* page number for a new page */
	int	mypage;		/* my page number */
	char	*mybuf;		/* pointer to my page buffer */
	int	myerrno;	/* my errro number */

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check "childno" */
	if (childno < 0)
		return DRerrSave(DRE_CHILDINVALID);

	/* check "childcov" */
	if (DRrectInvalid(rfd, childcov))
		return DRerrSave(DRE_RECTINVALID);

	/*--------------------------------------------------------------------*/
	/* insert in the index tree */
	errno = DRtreeInsertEntry(rfd, DRrootPage(rfd),
			DRftab[rfd].coverall, childno, childcov, 0,
			&newpage, DRftab[rfd].newrect);

	/* if the root node has splitted */
	if (errno > 0)
	{
		/* get a new node */
		mypage = DRnodeNew(rfd, DRrootLevel(rfd)+1, &mybuf);
		if (mypage < 0)
			return mypage;

		/* insert the first entry */
		myerrno = DRnodePutEntry(rfd, DRrootPage(rfd),
				DRftab[rfd].coverall, mybuf);
		if (myerrno < 0)
			return myerrno;

		/* insert the second entry */
		myerrno = DRnodePutEntry(rfd, newpage, DRftab[rfd].newrect, mybuf);
		if (myerrno < 0)
			return myerrno;

		/* update the header structure */
		DRrootPage(rfd) = mypage;
		DRrootLevel(rfd)++;
		DRftab[rfd].hdrChanged = TRUE;
		myerrno = DRnodeCover(rfd, DRftab[rfd].coverall, mybuf);
		if (myerrno < 0)
			return myerrno;

		/* unpin the new root page */
		if (PS_UnfixPage(DRftab[rfd].pfd, mypage, TRUE) != PSE_OK)
			return DRerrSave(DRE_PSERROR);
	}
	return errno;
}


/* DR_DeleteEntry() -
|
|	Remove a leaf entry with "childno" from the index file "rfd".
|	The cover rect of the entry must overlap with "testrect".
|
|	Return Value:
|	> 1	success, with empty root node;
|	= 1	success otherwise;
|	= 0	entry not found;
|	< 0	errors;
+-----------------------------------------------------------------------------*/
int DR_DeleteEntry(
	int	rfd,		/* DR file descriptor */
	int	childno,	/* record number to remove */
	DRrect	*testrect)	/* rect for comparison */
{
	int	errno;		/* error number */

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check "childno" */
	if (childno < 0)
		return DRerrSave(DRE_CHILDINVALID);

	/* check "testrect" */
	if (DRrectInvalid(rfd, testrect))
		return DRerrSave(DRE_RECTINVALID);

	/*--------------------------------------------------------------------*/
	/* delete in the index tree */
	errno = DRtreeDeleteEntry(rfd, DRrootPage(rfd),
			DRftab[rfd].coverall, childno, testrect);

	/* if the root page is empty */
	if (errno > 1)
	{
		DRrootLevel(rfd) = 0;
		DRftab[rfd].hdrChanged = TRUE;
	}
	return errno;
}


/* DR_PrintIndex() -
|
|	Print the data in an DR index.
|
|	Return Value:
|	= 0	success;
|	< 0	errors;
+-----------------------------------------------------------------------------*/
int DR_PrintIndex(
	int	rfd)	/* DR file descriptor */
{
	printf("nodecard=%d\n",DRnodeCard(rfd));
	return DRtreePrint(rfd, DRrootPage(rfd));
}

int DR_PrintLevel(
	int	rfd,
	int	level)	/* DR file descriptor */
{
	return DRtreePrintLevel(rfd, level, DRrootPage(rfd));
}

int DR_CountRec(
	int	rfd)	/* DR file descriptor */
{
	return DRtreeCountRec(rfd, DRrootPage(rfd), 0);
}



