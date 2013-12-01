/* PSbuf.c
|
|	Page Store (PS) buffer management functions.
+-----------------------------------------------------------------------------*/
#include "PSdef.h"




/* global variables
+-----------------------------------------------------------------------------*/
static PSbpage	*PSfirstbpage = NULL;	/* pointer to first buffer page */
static PSbpage	*PSlastbpage = NULL;	/* pointer to last buffer page */
static PSbpage	*PSfreebpage = NULL;	/* list of free buffer pages */
static int	PSnumbpages = 0;	/* # of buffer pages in memory */




/* supporting functions
+-----------------------------------------------------------------------------*/


/* PSbufInsertFree() -
|
|	Insert the buffer page pointed by "bpage" into the free list.
+-----------------------------------------------------------------------------*/
static void PSbufInsertFree(
	PSbpage	*bpage)
{
	bpage->nextpage = PSfreebpage;
	PSfreebpage = bpage;
}


/* PSbufLinkHead() -
|
|	Link the buffer page pointed by "bpage" as the head
|	of the used buffer list.
+-----------------------------------------------------------------------------*/
static void PSbufLinkHead(
	PSbpage	*bpage)
{
	bpage->nextpage = PSfirstbpage;
	bpage->prevpage = NULL;

	if (PSfirstbpage != NULL)
		PSfirstbpage->prevpage = bpage;
	PSfirstbpage = bpage;

	if (PSlastbpage == NULL)
		PSlastbpage = bpage;
}
	
	
/* PSbufUnlink() -
|
|	Unlink the page pointed by bpage from the buffer list. Assume
|	that bpage is a valid pointer.  Set the "prevpage" and "nextpage"
|	fields to NULL. The caller is responsible to either place
|	the unlinked page into the free list, or insert it back
|	into the used list.
+-----------------------------------------------------------------------------*/
static void PSbufUnlink(
	PSbpage	*bpage)
{
	if (PSfirstbpage == bpage)
		PSfirstbpage = bpage->nextpage;

	if (PSlastbpage == bpage)
		PSlastbpage = bpage->prevpage;
	
	if (bpage->nextpage != NULL)
		bpage->nextpage->prevpage = bpage->prevpage;
	
	if (bpage->prevpage != NULL)
		bpage->prevpage->nextpage = bpage->nextpage;

	bpage->prevpage = bpage->nextpage = NULL;
}


/* PSbufInternalAlloc() -
|
|	Allocate a buffer page and set *bpage to point to it. *bpage
|	is set to NULL if one can not be allocated.
|	The "nextpage" and "prevpage" fields of *bpage are linked as
|	the head of the list of used buffers. All the other fields are
|	undefined. writefn() is used to write pages.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
static int PSbufInternalAlloc(
	PSbpage	**bpage,
	int	(*writefn)())
{
	PSbpage	*tbpage;	/* temporary pointer to buffer page */
	int	errno;		/* error value returned*/

	/* Set *bpage to the buffer page to be returned */
	if (PSfreebpage != NULL)
	{
		/* Free list not empty, use the one from the free list. */
		*bpage = PSfreebpage;
		PSfreebpage = (*bpage)->nextpage;
	}
	else if (PSnumbpages < PS_MAXBUFS)
	{
		/* We have not reached max buffer limit, so malloc a new one */
		if ((*bpage = (PSbpage*)malloc(PS_BPAGE_SIZE)) == NULL)
			/* no more memory */
			return PSerrSave(PSE_NOMEM);

		/* increment # of pages allocated */
		PSnumbpages++;
	}
	else
	{
		/* we have reached max buffer limit */
		/* choose a victim from the buffer*/

		*bpage = NULL;		/* set initial return value */

		for (tbpage=PSlastbpage; tbpage!=NULL; tbpage=tbpage->prevpage)
			if (tbpage->fixes == 0)
				/* found a page that can be swapped out */
				break;

		if (tbpage == NULL)
			/* couldn't find a free page */
			return PSerrSave(PSE_NOBUF);

		/* write out the dirty page */
		if (tbpage->dirty)
		{
			if ((errno = (*writefn)(tbpage->pfd, tbpage->pageno,
					&tbpage->fpage)) != PSE_OK)
				return(errno);
		}
		tbpage->dirty = FALSE;

		/* unlink from hash table */
		if ((errno = PShashDelete(tbpage->pfd, tbpage->pageno)) != PSE_OK)
			return(errno);
		
		/* unlink from buffer list */
		PSbufUnlink(tbpage);

		*bpage = tbpage;
	}

	/* Link the page as the head of the used list */
	PSbufLinkHead(*bpage);
	return(PSE_OK);
}




/* interface functions
+-----------------------------------------------------------------------------*/


/* PSbufGet() -
|
|	Get the address of the page with "pageno" of file "pfd",
|	allows multiple fixes of a page.
+-----------------------------------------------------------------------------*/
int PSbufGet(
	int	pfd,
	int	pageno,
	PSfpage	**fpage,
	int	(*readfn)(),
	int	(*writefn)())
{
	return PSbufGet1(pfd, pageno, fpage, readfn, writefn, TRUE);
}


/* PSbufGet1() -
|
|	Get a page whose number is "pageno" from the file pointed
|	by "pfd". Set *fpage to point to the data for that page.
|	This function requires two functions:
|		readfn(pfd,pageno,fpage) 
|		int pfd;
|		int pageno;
|		PSfpage *fpage;
|	which will read one page whose number is "pageno" from the file "pfd"
|	into the buffer area pointed by "fpage".
|		writefn(pfd,pageno,fpage)
|		int pfd;
|		in pageno;
|		PSfpage *fpage;
|	which will write one page into the file.
|
|	If multiple_fixes is TRUE, then it is OK to get a page which
|	is alreay fixed in the buffer.  We simply increment `fixes.'
|	Otherwise, it is an error to read a page already fixed in the buffer.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PSbufGet1(
	int	pfd,
	int	pageno,
	PSfpage	**fpage,
	int	(*readfn)(),
	int	(*writefn)(),
	bool_t	multiple_fixes)
{
	PSbpage	*bpage;
	int	errno;
	
	/* if page is not in buffer... */
	if ((bpage = PShashFind(pfd, pageno)) == NULL)
	{
		/* init *fpage */
		*fpage = NULL;

		/* allocate an empty page */
		if ((errno = PSbufInternalAlloc(&bpage, writefn)) != PSE_OK)
			return(errno);
	
		/* read the page */
		if ((errno = (*readfn)(pfd, pageno, &bpage->fpage)) != PSE_OK)
		{
			/* error reading the page. put buffer back into */
			/* the free list, and return gracefully */
			PSbufUnlink(bpage);
			PSbufInsertFree(bpage);
			return(errno);
		}
	
		/* insert new page into hash table */
		if ((errno = PShashInsert(pfd, pageno, bpage)) != PSE_OK)
		{
			/* failed to insert into hash table */
			/* put page into free list */
			PSbufUnlink(bpage);
			PSbufInsertFree(bpage);
			return(errno);
		}
		
		/* set the fields for this page*/
		bpage->pfd = pfd;
		bpage->pageno = pageno;
		bpage->dirty = FALSE;
		bpage->fixes = 1;
	}
	else if (!multiple_fixes && bpage->fixes > 0)
	{
		/* otherwise if we do not allow to get the page multiple times */
		/* and the page is already fixed, then error occurs */

		/* *fpage = &bpage->fpage; */
		*fpage = NULL;
		return PSerrSave(PSE_PAGEFIXED);
	}
	else
		/* otherwise, page is in memory; increment count */
		bpage->fixes++;

	*fpage = &bpage->fpage;
	return(PSE_OK);
}


/* PSbufUnfix() -
|
|	Unfix the file page whose number is "pageno" from the buffer.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PSbufUnfix(
	int	pfd,
	int	pageno)
{
	PSbpage	*bpage;

	if ((bpage = PShashFind(pfd, pageno)) == NULL)
		/* page not in buffer */
		return PSerrSave(PSE_PAGENOTINBUF);

	if (bpage->fixes == 0)
		/* page not fixed */
		return PSerrSave(PSE_PAGEUNFIXED);

	/* if this is the last reference to the page, unfix the page */
        bpage->fixes--;
        if (bpage->fixes == 0)
	{
		/* unlink this page */
		PSbufUnlink(bpage);

		/* insert it as head of linked list to make it most recently used*/
		PSbufLinkHead(bpage);
	}

	return(PSE_OK);
}


/* PSbufAlloc() -
|
|	Allocate a buffer and mark it belonging to page "pageno"
|	of file "fd".  Set *fpage to point to the buffer data.
|	The function "writefcn" is used to write out pages.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PSbufAlloc(
	int	pfd,
	int	pageno,
	PSfpage	**fpage,
	int	(*writefn)())
{
	PSbpage	*bpage;
	int	errno;

	*fpage = NULL;	/* initial value of fpage */

	if ((bpage = PShashFind(pfd, pageno)) != NULL)
		/* page already in buffer*/
		return PSerrSave(PSE_PAGEINBUF);

	if ((errno = PSbufInternalAlloc(&bpage, writefn)) != PSE_OK)
		/* can't get any buffer */
		return(errno);
	
	/* put ourselves into the hash table */
	if ((errno = PShashInsert(pfd, pageno, bpage)) != PSE_OK)
	{
		/* can't insert into the hash table */
		/* unlink bpage, and put it into the free list */
		PSbufUnlink(bpage);
		PSbufInsertFree(bpage);
		return(errno);
	}

	/* init the fields of bpage and return */
	bpage->pfd = pfd;
	bpage->pageno = pageno;
	bpage->fixes = 1;
	bpage->dirty = FALSE;

	*fpage = &bpage->fpage;

	return(PSE_OK);
}


/* PSbufReleaseFile() -
|
|	Do a linear scan of all the buffer pages, write out all the ones
|	belong to the file refered to by "pfd" and put the page frames into
|	the free list.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PSbufReleaseFile(
	int	pfd,
	int	(*writefn)())
{
	PSbpage	*bpage, *temppage;
	int	errno;

	/* Do linear scan of the buffer to find pages belonging to the file */
	bpage = PSfirstbpage;
	while (bpage != NULL)
	{
		if (bpage->pfd == pfd)
		{
			/* The file descriptor matches*/
			if (bpage->fixes > 0)
				/* the page is currently fixed */
				return PSerrSave(PSE_PAGEFIXED);

			/* write out dirty page */
			if (bpage->dirty)
			{
				if ((errno = (*writefn)(pfd, bpage->pageno,
					&bpage->fpage)) != PSE_OK)
					/* error writing file */
					return errno;
			}
			bpage->dirty = FALSE;

			/* get rid of it from the hash table */
			if ((errno = PShashDelete(pfd, bpage->pageno))
				!= PSE_OK)
			{
				/* internal error */
				PSperror("PSbufReleaseFile");
				exit(1);
			}

			/* put the page into free list */
			temppage = bpage;
			bpage = bpage->nextpage;
			PSbufUnlink(temppage);
			PSbufInsertFree(temppage);
		}
		else	bpage = bpage->nextpage;
	}
	return(PSE_OK);
}


/* PSbufUsed() -
|
|	Mark page numbered "pagenum" of file descriptor "fd" as used.
|	The page must be fixed in the buffer. Make this page most
|	recently used.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PSbufUsed(
	int	pfd,
	int	pageno)
{
	PSbpage	*bpage;

	/* Find page in the buffer */
	if ((bpage = PShashFind(pfd, pageno)) == NULL)
		/* page not in the buffer */
		return PSerrSave(PSE_PAGENOTINBUF);

	if (bpage->fixes == 0)
		/* page not fixed */
		return PSerrSave(PSE_PAGEUNFIXED);

	/* mark this page dirty */
	bpage->dirty = TRUE;

	/* make this page head of the list of buffers*/
	PSbufUnlink(bpage);
	PSbufLinkHead(bpage);

	return(PSE_OK);
}


/* PSbufPrint() -
|
|	Print the current page buffers.
+-----------------------------------------------------------------------------*/
void PSbufPrint(void)
{
	PSbpage	*bpage;

	printf("buffer content:\n");
	if (PSfirstbpage == NULL)
		printf("\tempty\n");
	else
	{
		printf("\tpfd\tpageno\tfixes\tdirty\tfpage\n");
		for (bpage = PSfirstbpage; bpage != NULL; bpage = bpage->nextpage)
			printf("\t%d\t%d\t%d\t%d\t%d\n",
				bpage->pfd, bpage->pageno, (int)bpage->fixes,
				(int)bpage->dirty, (int)&bpage->fpage);
	}
}


