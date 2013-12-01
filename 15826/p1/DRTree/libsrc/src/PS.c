/* PS.c
|
|	Page Store (PS) interface functions.
+-----------------------------------------------------------------------------*/
#include <sys/stat.h>
#include <unistd.h>	/* for lseek() */
#include "PSdef.h"




/* global variable
+-----------------------------------------------------------------------------*/
/* PS file table */
static PSftab_entry	PSftab[PS_FTAB_SIZE];




/* supporting macros
+-----------------------------------------------------------------------------*/
/* macro to test a PS file descriptor */
#define PSpfdInvalid(pfd) \
	((pfd) < 0 || (pfd) >= PS_FTAB_SIZE || PSftab[(pfd)].valid == FALSE)

/* macro to test a PS page number */
#define PSpagenoInvalid(pfd, pageno) \
	((pageno) < 0 || (pageno) >= PSftab[(pfd)].hdr.numpages)

/* macro to test if a PS file page is used */
#define PSfpageUsed(fpage) \
	((fpage)->nextfree == PS_FPAGE_USED)

/* macro to test if a page is free */
#define PSfpageFree(fpage) \
	((fpage)->nextfree != PS_FPAGE_USED)

/* macro to test if an opened file has free page */
#define PSpfdHasFreePage(pfd) \
	(PSftab[(pfd)].hdr.firstfree != PS_FPAGE_LIST_END)




/* supporting functions
+-----------------------------------------------------------------------------*/


/* PSftabFindFile() -
|
|	Find the PS file table entry whose inode number is the same
|	as the parameter inode number.
|
|	Return Value:
|	>= 0	success;
|	< 0	fail;
+-----------------------------------------------------------------------------*/
static int PSftabFindFile(
	ino_t	inode)
{
	int	i;

	/* for each file table entry */
	for (i = 0; i < PS_FTAB_SIZE && (PSftab[i].valid == FALSE
				|| PSftab[i].inode != inode); ++i);

	/* if there is no such entry */
	if (i == PS_FTAB_SIZE)
		return -1;

	/* find an entry */
	return i;
}


/* PSftabFindFree() -
|
|	Find a free entry in the PS file table.
|
|	Return Value:
|	>= 0	index number for a free entry in the file table;
|	< 0	file table is full;
+-----------------------------------------------------------------------------*/
static int PSftabFindFree(void)
{
	int	i;

	/* scan through the file table to find a free entry */
	for (i = 0; i < PS_FTAB_SIZE && PSftab[i].valid != FALSE; ++i);

	/* if there is no such entry */
	if (i == PS_FTAB_SIZE)
		return -1;

	/* find an entry */
	return i;
}


/* PSfpageRead() -
|
|	Read the page numbered "pageno" of the file "pfd"
|	into the page buffer "buf".
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
static int PSfpageRead(
	int	pfd,		/* PS file descriptor */
	int	pageno,		/* PS file page number */
	PSfpage *buf)		/* pointer to PS file page buffer */
{
	int	offset, nbytes;

	/* get the offset of the page in the file */
	offset = pageno * PS_FPAGE_SIZE + PS_HDR_SIZE;

	/* seek to the appropriate position */
	if (lseek(PSftab[pfd].unixfd, (off_t)offset, SEEK_SET) < 0)
		return PSerrSave(PSE_UNIX);

	/* read the data */
	nbytes = read(PSftab[pfd].unixfd, (char*)buf, PS_FPAGE_SIZE);
	if(nbytes < 0)
		return PSerrSave(PSE_UNIX);

	if (nbytes != PS_FPAGE_SIZE)
		return PSerrSave(PSE_INCOMPLETEREAD);

	return PSE_OK;
}


/* PSfpageWrite() -
|
|	Write the page numbered "pageno" from the buffer "buf"
|	into the file indexed by "pfd".
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
static int PSfpageWrite(
	int	pfd,		/* PS file descriptor */
	int	pageno,		/* PS file page number */
	PSfpage	*buf)		/* pointer to PS file page buffer */
{
	int	offset, nbytes;

	/* get the offset of the page in the file */
	offset = pageno * PS_FPAGE_SIZE + PS_HDR_SIZE;

	/* seek to the right position */
	if (lseek(PSftab[pfd].unixfd, (off_t)offset, SEEK_SET) < 0)
		return PSerrSave(PSE_UNIX);

	/* write out the page */
	nbytes = write(PSftab[pfd].unixfd, (char*)buf, PS_FPAGE_SIZE);
	if (nbytes < 0)
		return PSerrSave(PSE_UNIX);

	if (nbytes != PS_FPAGE_SIZE)
		return PSerrSave(PSE_INCOMPLETEWRITE);

	return PSE_OK;    
}




/* interface functions
+-----------------------------------------------------------------------------*/


/* PS_Init() -
|
|	Initialize the PS layer, including the PS hash table and file table.
|
|	Return Value:
|	none;
+-----------------------------------------------------------------------------*/
void PS_Init(void)
{
	int	i;

	/* initialize the hash table */
	PShashInit();

	/* initialize the PS file table */
	for (i = 0; i < PS_FTAB_SIZE; ++i)
		PSftab[i].valid = FALSE;
}


/* PS_CreateFile() -
|
|	Create a PS file called "filename". The file should not have
|	already existed before.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
/* default file creation mask */
#define	DEF_CREATE_MASK		0664

int PS_CreateFile(
	char	*filename)	/* PS file name */
{
	int	fd;		/* unix file descriptor */
	int	nbytes;		/* return code from write() */
	PShdr	hdr;		/* file header */

	/* create file for exclusive use */
	fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, DEF_CREATE_MASK);
	if (fd < 0)
		return PSerrSave(PSE_UNIX);

	/* initialize the file header */
	hdr.firstfree = PS_FPAGE_LIST_END;
	hdr.numpages = 0;

	/* write header to file */
	nbytes = write(fd, (char*)&hdr, PS_HDR_SIZE);
	if (nbytes != PS_HDR_SIZE)
	{
		/* error while writing: close and remove file */
		close(fd);
		unlink(filename);

		if(nbytes < 0)
	    		return PSerrSave(PSE_UNIX);

		return PSerrSave(PSE_HDRWRITE);
	}

	/* close file */
	if (close(fd) < 0)
		return PSerrSave(PSE_UNIX);

	return PSE_OK;
}


/* PS_DestroyFile() -
|
|	Destroy the PS file whose name is "filename".
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_DestroyFile(
	char	*filename)	/* PS file name */
{
	struct stat	statbuf;

	if (stat(filename, &statbuf) != 0)
		return PSerrSave(PSE_UNIX);

	/* if file is open then error */
	if (PSftabFindFile(statbuf.st_ino) >= 0)
		return PSerrSave(PSE_FILEOPEN);

	/* remove the file */
	if(unlink(filename) < 0)
		return PSerrSave(PSE_UNIX);
    
	return PSE_OK;
}


/* PS_OpenFile() -
|
|	Open the PS file whose name is "filename". It is possible to open
|	a file more than once, however, it will be treated as 2 separate
|	files (different file descriptors; different buffers).
|	Thus, opening a file more than once for writing may corrupt the file,
|	and can, in certain circumstances, crash the PS layer.  Note that
|	even if only one instance of a file is for writing, problems may
|	occur because some writes may not be seen by a reader of another
|	instance of the file.
|
|	Return Value:
|	>= 0	PS file descriptor;
|	< 0	errors;
+-----------------------------------------------------------------------------*/
int PS_OpenFile(
	char	*filename)		/* PS file name */
{
	int		pfd;		/* PS file descriptor */
	int		fd;		/* unix file descriptor */
	int		nbytes;		/* # of bytes in read */
	struct stat	statbuf;	/* used for fstat() */

	/* find a free entry in the PS file table */
	if ((pfd = PSftabFindFree()) < 0)
		return PSerrSave(PSE_FTABFULL);

	/* open the file */
	if ((fd = open(filename, O_RDWR)) < 0)
		return PSerrSave(PSE_UNIX);

	/* get the inode of the file */
	if (fstat(fd, &statbuf) != 0)
	{
		close(fd);
		return PSerrSave(PSE_UNIX);
	}

	/* Read the file header */
	nbytes = read(fd, (char*)&PSftab[pfd].hdr, PS_HDR_SIZE);
	if (nbytes < PS_HDR_SIZE)
	{
		close(fd);
		if(nbytes < 0)
			return PSerrSave(PSE_UNIX);

		return PSerrSave(PSE_HDRREAD);
	}

	/* set file header to be not changed */
	PSftab[pfd].hdrchanged = FALSE;

	/* save the file descriptor and inode of the file */
	PSftab[pfd].unixfd = fd;
	PSftab[pfd].inode = statbuf.st_ino;

	/* mark the entry valid */
	PSftab[pfd].valid = TRUE;

	/* success */
	return pfd;
}


/* PS_CloseFile() -
|
|	Close the file indexed by file descriptor "pfd". It is an error
|	to close a file with pages still fixed in the buffer.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_CloseFile(
	int	pfd)		/* PS file descriptor */
{
	int	errno, nbytes;

	/* check "pfd" */
	if (PSpfdInvalid(pfd))
		return PSerrSave(PSE_FD);

	/* Flush all buffers for this file */
	if ((errno = PSbufReleaseFile(pfd, PSfpageWrite)) != PSE_OK)
		return  errno ;

	/* if the file header has changed,, write it back to file */
	if (PSftab[pfd].hdrchanged)
	{
		/* First seek to the appropriate place */
		if (lseek(PSftab[pfd].unixfd, (off_t)0, SEEK_SET) < 0)
			return PSerrSave(PSE_UNIX);
	
		/* write header*/
		nbytes = write(PSftab[pfd].unixfd, (char*)&PSftab[pfd].hdr,
						PS_HDR_SIZE);
		if(nbytes < 0)
			return PSerrSave(PSE_UNIX);

		if(nbytes != PS_HDR_SIZE)
			return PSerrSave(PSE_HDRWRITE);

		PSftab[pfd].hdrchanged = FALSE;
	}

	/* close the file */
	if(close(PSftab[pfd].unixfd) < 0)
		return PSerrSave(PSE_UNIX);

	/* mark the entry invalid */
	PSftab[pfd].valid = FALSE;

	/* success */
	return PSE_OK;
}


/* PS_GetFirstPage() -
|
|	Read the first page into memory and set *pagebuf to point to it.
|	Set *pagenum to the page number of the page read.
|	The page read is fixed in the buffer until it is unfixed.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_GetFirstPage(
	int	pfd,		/* PS file descriptor */
	int	*pageno,	/* PS file page number */
	char	**pagebuf)	/* pointer to poiter of page buffer */
{
	*pageno = -1;
	return PS_GetNextPage(pfd, pageno, pagebuf);
}


/* PS_GetNextPage() -
|
|	Read the next valid page after *pagenum, the current page number,
|	and set *pagebuf to point to the page data. Set *pagenum
|	to be the new page number. The new page is fixed in memory
|	until it is unfixed.
|
|	Note that PS_GetNextPage() with *pagenum == -1 will return the
|	first valid page.
|
|	Returns:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_GetNextPage(
	int	pfd,		/* PS file descriptor */
	int	*pageno,	/* PS file page number */
	char	**pagebuf)	/* pointer to poiter of page buffer */
{
	PSfpage	*fpage;		/* pointer to file page */
	int	temppage;	/* page number to scan for next valid page */
	int	errno;		/* error code */

	/* validate pfd */
	if (PSpfdInvalid(pfd))
		return PSerrSave(PSE_FD);

	/* validate pageno */
	if (*pageno != -1 && PSpagenoInvalid(pfd, *pageno))
		return PSerrSave(PSE_INVALIDPAGE);

	/* scan the file until a valid used page is found */
	for(temppage=*pageno+1; temppage<PSftab[pfd].hdr.numpages; temppage++)
	{
		/* get the next page into the buffer */
		if ((errno = PSbufGet(pfd, temppage, &fpage,
				PSfpageRead, PSfpageWrite)) != PSE_OK)
			return errno;

		/* if the page is being used, then done */
		if (PSfpageUsed(fpage))
		{
			*pageno = temppage;
			*pagebuf = fpage->pagebuf;
			return PSE_OK;
		}

		/* otherwise, unfix the page (since it's not being used) */
		if ((errno = PSbufUnfix(pfd, temppage)) != PSE_OK)
		{
			/* internal error */
			PSperror("PS_GetNextPage");
			exit(1);
		}
	}

	/* No valid used page found */
	return PSerrSave(PSE_EOF);
}


/* PS_GetThisPage() -
|
|	Read the page specified by "pagenum" and set *pagebuf to point
|	to the page data. The page number should be valid.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_GetThisPage(
	int	pfd,		/* PS file descriptor */
	int	pageno,		/* PS file page number */
	char	**pagebuf)	/* pointer to poiter of page buffer */
{
	PSfpage	*fpage;
	int	errno;

	/* validate pfd */
	if (PSpfdInvalid(pfd))
		return PSerrSave(PSE_FD);

	/* validate pageno */
	if (PSpagenoInvalid(pfd, pageno))
		return PSerrSave(PSE_INVALIDPAGE);

	/* see if we've already got the page */
	if ((errno = PSbufGet(pfd, pageno, &fpage,
			PSfpageRead, PSfpageWrite)) != PSE_OK)
		return errno;

	/* if page is valid, then done */
	if (PSfpageUsed(fpage))
	{
		*pagebuf = fpage->pagebuf;
		return PSE_OK;
	}

	/* otherwise */
	if ((errno = PSbufUnfix(pfd, pageno)) != PSE_OK)
	{
		PSperror("PS_GetThisPage");
		exit(1);
	}

	return PSerrSave(PSE_INVALIDPAGE);
}


/* PS_AllocPage() -
|
|	Allocate a new, empty page for file "fd". Set *pagenum to the
|	new page number. Set *pagebuf to point to the buffer for that page.
|	The page allocated is fixed in the buffer.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_AllocPage(
	int	pfd,		/* PS file descriptor */
	int	*pageno,	/* PS file page number */
	char	**pagebuf)	/* pointer to poiter of page buffer */
{
	PSfpage	*fpage;
	int	errno;
	
	if (PSpfdInvalid(pfd))
		return PSerrSave(PSE_FD);
	
	/* if the free list isn't empty... */
	if (PSpfdHasFreePage(pfd))
	{
		*pageno = PSftab[pfd].hdr.firstfree;

		/* get the page into the bufffer to get the nextfree number */
		if ((errno = PSbufGet(pfd, *pageno, &fpage,
				 PSfpageRead, PSfpageWrite)) != PSE_OK)
			return(errno);

		PSftab[pfd].hdr.firstfree = fpage->nextfree;
		PSftab[pfd].hdrchanged = TRUE;
	}
	else
	{
		/* the free list is empty... */
		*pageno = PSftab[pfd].hdr.numpages;

		/* allocate a page */
		if ((errno = PSbufAlloc(pfd, *pageno, &fpage,
				PSfpageWrite)) != PSE_OK)
			return(errno);
	
		/* increment number of pages for this file */
		PSftab[pfd].hdr.numpages++;
		PSftab[pfd].hdrchanged = TRUE;

		/* mark this page dirty */
		if ((errno = PSbufUsed(pfd, *pageno)) != PSE_OK)
		{
			PSperror("PS_AllocPage");
			exit(1);
		}
	}

	/* zero out the page */
	memset(fpage->pagebuf, 0, PS_PAGEBUF_SIZE);

	/* Mark the new page used */
	fpage->nextfree = PS_FPAGE_USED;

	/* set return value */
	*pagebuf = fpage->pagebuf;

	return PSE_OK;
}


/* PS_DisposePage()
|
|	Dispose the page numbered "pageno" of the file "pfd".
|	Only a page that is not fixed in the buffer can be disposed.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_DisposePage(
	int	pfd,		/* PS file descriptor */
	int	pageno)		/* PS file page number */
{
	PSfpage	*fpage;
	int	errno;

	/* validate pfd */
	if (PSpfdInvalid(pfd))
		return PSerrSave(PSE_FD);

	/* validate pageno */
	if (PSpagenoInvalid(pfd, pageno))
		return PSerrSave(PSE_INVALIDPAGE);

	/* get the page (but don't re-get it if it's already gotten) */
	if ((errno = PSbufGet1(pfd, pageno, &fpage,
			  PSfpageRead, PSfpageWrite, FALSE)) != PSE_OK)
		return errno;

	/* if the page is already freed... */
	if (PSfpageFree(fpage))
	{
		/* unfix the page */
		if ((errno = PSbufUnfix(pfd, pageno)) != PSE_OK)
		{
			PSperror("PS_DisposePage");
			exit(1);
		}

		return PSerrSave(PSE_PAGEFREE);
	}

	/* we should check if the page is the last page of the file */
	/* then remove this page from the file */
	/* get to this later */
	
	/* put this page onto the free list */
	fpage->nextfree = PSftab[pfd].hdr.firstfree;
	PSftab[pfd].hdr.firstfree = pageno;
	PSftab[pfd].hdrchanged = TRUE;

	/* mark the page dirty (because we changed its `next' ptr) */
	if (PSbufUsed(pfd, pageno) != PSE_OK)
	{
		PSperror("PS_DisposePage");
		exit(1);
	}

	/* unfix the page */
	return PSbufUnfix(pfd, pageno);
}


/* PS_DirtyPage() -
|
|	Marks page "pageno" of the file referenced by "pfd" dirty.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_DirtyPage(
	int	pfd,		/* PS file descriptor */
	int	pageno)		/* PS file page number */
{
	/* validate fd */
	if (PSpfdInvalid(pfd))
		return PSerrSave(PSE_FD);

	/* validate pageno */
	if (PSpagenoInvalid(pfd, pageno))
		return PSerrSave(PSE_INVALIDPAGE);

	/* mark the page dirty */
	return PSbufUsed(pfd, pageno);
}


/* PS_UnfixPage() -
|
|	Unfix the page numbered "pageno" of the file "pfd" in buffer.
|	Mark the page if "dirty" is TRUE.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_UnfixPage(
	int	pfd,		/* PS file descriptor */
	int	pageno,		/* PS file page number */
	bool_t	dirty)		/* TRUE if page has been changed */
{
	int	errno;

	if (PSpfdInvalid(pfd))
		return PSerrSave(PSE_FD);

	if (PSpagenoInvalid(pfd, pageno))
		return PSerrSave(PSE_INVALIDPAGE);

	/* mark the page dirty */
	if (dirty == TRUE)
	{
		if ((errno = PSbufUsed(pfd, pageno)) != PSE_OK)
			return errno;
	}

	return PSbufUnfix(pfd, pageno);
}


/* PS_RenameFile() -
|
|	Rename "oldfile" to "newfile".
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PS_RenameFile(
	char	*oldfile,
	char	*newfile)
{
	struct stat	statbuf;

	if (stat(oldfile, &statbuf) != 0)
		return PSerrSave(PSE_UNIX);

	if (PSftabFindFile(statbuf.st_ino) >= 0)
		return PSerrSave(PSE_FILEOPEN);

	if (stat(newfile, &statbuf) == 0 &&
			PSftabFindFile(statbuf.st_ino) >= 0)
		return PSerrSave(PSE_FILEOPEN);

	if (rename(oldfile, newfile) != 0)
		return PSerrSave(PSE_UNIX);

	return PSE_OK;
}


