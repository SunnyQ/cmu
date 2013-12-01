/* DRnode.c
|
|	Deferred-split R-tree (DR) supporting functions to manipulate entries
|	in a DR node.
+-----------------------------------------------------------------------------*/
#include "DRdef.h"




/* supporting type
+-----------------------------------------------------------------------------*/
typedef	unsigned char	byte;




/* functions to manipulate entries in a DR node
+-----------------------------------------------------------------------------*/


/* DRnodeFirstEntry() -
|
|	Get the index number of the first entry in a node.
+-----------------------------------------------------------------------------*/
int DRnodeFirstEntry(
	int	rfd,		/* DR index file descriptor */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	return DRnodeNextEntry(rfd, -1, pagebuf);
}


/* DRnodeNextEntry() -
|
|	Get the index number of the next entry in a node.
|
|	Return Value:
|	>= 0	entry index;
|	= -1	otherwise;
+-----------------------------------------------------------------------------*/
int DRnodeNextEntry(
	int	rfd,		/* DR index file descriptor */
	int	entry,		/* current entry number */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	byte	*bmp;		/* pointer to bitmap byte */
	int	bit;		/* bit position in byte */

	/* check "entry" */
	if (++entry < 0 || entry >= DRnodeCard(rfd))
		return -1;

	/* get to the byte in bitmap */
	bmp = DRnodeBitmap(rfd, pagebuf) + (entry >> 3); /* entry/8 */

	/* get the bit position in byte */
	bit = entry & 7; /* entry%8 */

	/* check the rest of the bitmap to find the first used slot */
	for (; entry < DRnodeCard(rfd); entry+=8, bmp++)
	{
		/* if all bits in current byte are unset */
		if (((*bmp<<bit) & 0xFF) == 0x00)
		{
			/* it's possible that (bit > 0) in the first loop */
			if (bit > 0)
			{
				/* adjust them */
				entry -= bit;
				bit = 0;
			}
		}
		/* at least one of the bits is set */
		else
		{
			/* find the bit and its coresponding entry number */
			for (; (char)((*bmp<<bit)&0xFF) > 0; bit++, entry++);

			/* if the entry number is valid */
			if (entry < DRnodeCard(rfd))
				return entry;
			else
				return -1;
		}
	}

	return -1;
}


/* DRnodeFreeEntry() -
|
|	Get the index number of the first free entry slot in the node.
+-----------------------------------------------------------------------------*/
int DRnodeFreeEntry(
	int	rfd,		/* DR index file descriptor */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	int	entry;		/* entry number */
	byte	*bmp;		/* pointer to bitmap byte */
	int	bit;		/* bit position in byte */

	/* get the first byte of the bitmap */
	bmp = DRnodeBitmap(rfd, pagebuf);

	/* check the whole bitmap to find the first free entry */
	for (entry = 0; entry < DRnodeCard(rfd); entry += 8, bmp++)
	{
		/* if at least one bit in the byte is unset */
		if (((*bmp) & 0xFF) != 0xFF)
		{
			/* find the first unset bit in the byte */
			for (bit=0; (char)((*bmp<<bit)&0xFF) < 0; bit++, entry++);

			/* if the entry is valid */
			if (entry < DRnodeCard(rfd))
				return entry;
			else
				return -1;
		}
	}

	return -1;
}


/* DRnodeMarkEntry() -
|
|	Mark an entry slot in a node as used or unused.
+-----------------------------------------------------------------------------*/
bool_t DRnodeMarkEntry(
	int	rfd,		/* DR index file descriptor */
	int	entry,		/* entry number */
	bool_t	op,		/* operation */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	int	old;		/* old bit value */
	byte	*bmp;		/* pointer to bitmap byte */
	int	bit;		/* bit position in byte */

	/* Move pointer to the correct byte in bitmap and get bit position */
	bmp = DRnodeBitmap(rfd, pagebuf) + (entry >> 3);
	bit = entry & 7;
	old = *bmp & (0x80 >> bit);

	switch (op)
	{
		case TRUE:
		*bmp |= (0x80 >> bit);
		break;

		case FALSE:
		*bmp &= (0xFF ^ (0x80 >> bit));
		break;

		default:
		break;
	}

	return (old) ? TRUE : FALSE;
}



/* DRnodeNew() -
|
|	Allocate a new node.
+-----------------------------------------------------------------------------*/
int DRnodeNew(
	int	rfd,		/* DR file descriptor */
	int	level,		/* level of the new node */
	char	**pagebuf)	/* pointer to pointer of page buffer */
{
	int	pageno;		/* page number */

	/* allocate a PS page */
	if (PS_AllocPage(DRftab[rfd].pfd, &pageno, pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* initialize the node */
	DRnodeLevel(rfd, *pagebuf) = level;
	DRnodeCount(rfd, *pagebuf) = 0;
	memset(DRnodeBitmap(rfd, *pagebuf), 0, DRftab[rfd].hdr.bmpsize);

	/* success */
	return pageno;
}



/* DRnodePutEntry() -
|
|	Functionality:
|	Put "childno" and "childcov" as an entry into the page buffer
|	of a node.
|
|	Return Value:
|	>= 0	entry number;
|	< 0	errno conditions;
+-----------------------------------------------------------------------------*/
int DRnodePutEntry(
	int	rfd,		/* DR index file descriptor */
	int	childno,	/* child number to insert */
	DRrect	*childcov,	/* cover rect of the child */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	int	slot;		/* free entry slot number */

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

	/* check the node entry count */
	if (DRnodeCount(rfd, pagebuf) < 0)
		return DRerrSave(DRE_NODEINVALID);

	/* check if the node is full */
	if (DRnodeCount(rfd, pagebuf) >= DRnodeCard(rfd))
		return DRerrSave(DRE_NODEFULL);

	/*--------------------------------------------------------------------*/
	/* now it's time to put the new entry */
	/* find a free entry slot in the node */
	slot = DRnodeFreeEntry(rfd, pagebuf);

	/* if cannot get free slot */
	if (slot < 0)
	{
		printf("DRnodePutEntry(): internal error\n");
		exit(1);
	}

	/* copy "childno" and "childcov" to entry slot */
	DRnodeChild(rfd, slot, pagebuf) = childno;
	DRrectCopy(rfd, DRnodeRect(rfd, slot, pagebuf), childcov);

	/* mark the slot to be used */
	if (DRnodeMarkEntry(rfd, slot, TRUE, pagebuf) != FALSE)
	{
		printf("DRnodePutEntry(): internal error\n");
		exit(2);
	}

	/* increase the entry count */
	++DRnodeCount(rfd, pagebuf);

	/* success */
	return slot;
}


/* DRnodeCutEntry() -
|
|	Functionality:
|	Remove "entry" from a node.
|
|	Return Value:
|	>= 0	entry number;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DRnodeCutEntry(
	int	rfd,		/* DR index file descriptor */
	int	entry,		/* index of the entry in the node */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check "entry" */
	if (entry < 0 || entry >= DRnodeCard(rfd))
		return DRerrSave(DRE_ENTRYINVALID);

	/* check the node entry count */
	if (DRnodeCount(rfd, pagebuf) < 0)
		return DRerrSave(DRE_NODEINVALID);

	/* check if the node is empty */
	if (DRnodeCount(rfd, pagebuf) == 0)
		return DRerrSave(DRE_NODEEMPTY);

	/*--------------------------------------------------------------------*/
	/* mark the entry as free */
	if (DRnodeMarkEntry(rfd, entry, FALSE, pagebuf) != TRUE)
	{
		printf("DRnodeCutEntry(): internal error\n");
		exit(1);
	}

	/* decrease the entry count */
	--DRnodeCount(rfd, pagebuf);

	/* success */
	return entry;
}


/* DRnodePickEntry() -
|
|	Functionality:
|	Pick the entry whose cover rect needs the least enlargement
|	if combined with "testrect".
|
|	Return Value:
|	>= 0	picked entry number;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DRnodePickEntry(
	int	rfd,		/* DR index file descriptor */
	DRrect	*testrect,	/* rect for testing */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	double	coverarea, leastarea;
	double	areagain, leastgain;
	int	entry;
	int	i;

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check "testrect" */
	if (DRrectInvalid(rfd, testrect))
		return DRerrSave(DRE_RECTINVALID);

	/* check the node entry count */
	if (DRnodeCount(rfd, pagebuf) < 0)
		return DRerrSave(DRE_NODEINVALID);

	/* check the node entry count */
	if (DRnodeCount(rfd, pagebuf) == 0)
		return DRerrSave(DRE_NODEEMPTY);

	/* initialize "entry" and "leastgain" */
	entry = -1;
	leastgain = -1;

	/*--------------------------------------------------------------------*/
	/* for each entry in the node... */
	for (i = DRnodeFirstEntry(rfd, pagebuf);
		i >= 0 && i < DRnodeCard(rfd);
		i = DRnodeNextEntry(rfd, i, pagebuf))
	{
		/* get the old area */
		coverarea = DRrectArea(rfd, DRnodeRect(rfd, i, pagebuf));

		/* get the combined rect and area gain */
		DRrectCopy(rfd, DRftab[rfd].temprect, DRnodeRect(rfd, i, pagebuf));
		DRrectCombine(rfd, DRftab[rfd].temprect, testrect);
		areagain = DRrectArea(rfd, DRftab[rfd].temprect) - coverarea;

		if (leastgain == -1 || areagain < leastgain)
		{
			leastarea = coverarea;
			leastgain = areagain;
			entry = i;
		}
		else if (areagain == leastgain && coverarea < leastarea )
		{
			leastarea = coverarea;
			entry = i;
		}
	}

	if (entry < 0)
	{
		printf("DRnodePickEntry(): internal error\n");
		exit(1);
	}
	return entry;
}



/* DRnodeCover() -
|
|	Funtionality:
|	Find the cover rect of a node.
|
|	Return Value:
|	= 0	success;
|	< 0	error;
+-----------------------------------------------------------------------------*/
int DRnodeCover(
	int	rfd,		/* DR index file descriptor */
	DRrect	*nodecov,	/* cover rect of the node */
	char	*pagebuf)	/* pointer to page buffer of node */
{
	int	i;

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check the node entry count */
	if (DRnodeCount(rfd, pagebuf) < 0)
		return DRerrSave(DRE_NODEINVALID);

	/*--------------------------------------------------------------------*/
	/* if the node is empty */
	if (DRnodeCount(rfd, pagebuf) == 0)
	{
		memset(nodecov, 0, DRrectSize(rfd));
		return DRE_OK;
	}

	/* find the first entry */
	if ((i = DRnodeFirstEntry(rfd, pagebuf)) < 0)
	{
		printf("DRnodeCover(): internal error\n");
		exit(1);
	}
	/* copy the first rect */
	DRrectCopy(rfd, nodecov, DRnodeRect(rfd, i, pagebuf));

	/* combine with rest of the rects */
	for (; i >= 0 && i < DRnodeCard(rfd); i = DRnodeNextEntry(rfd, i, pagebuf))
		DRrectCombine(rfd, nodecov, DRnodeRect(rfd, i, pagebuf));

	/* success */
	return DRE_OK;
}


/* DRnodePrintEntry() -
+-----------------------------------------------------------------------------*/
int DRnodePrintEntry(
	int	rfd,
	int	entry,
	char	*pagebuf)
{
	printf("\tchildno=%d\n",DRnodeChild(rfd,entry,pagebuf));
	fprintf(stderr,"\t%d\n",DRnodeChild(rfd,entry,pagebuf));
	printf("\tchildcov=\n");
	DRrectPrint(rfd,DRnodeRect(rfd,entry,pagebuf));

	return 0;
}



/* DRnodePrint() -
+-----------------------------------------------------------------------------*/
int DRnodePrint(
	int	rfd,
	char	*pagebuf)
{
	int	i;

	printf("level=%d\tcount=%d\n",DRnodeLevel(rfd,pagebuf),
					DRnodeCount(rfd,pagebuf));

	for (i = DRnodeFirstEntry(rfd, pagebuf);
		i >= 0 && i < DRnodeCard(rfd);
		i = DRnodeNextEntry(rfd, i, pagebuf))
	{
		printf("entry=%d\n",i);
		DRnodePrintEntry(rfd,i,pagebuf);
	}

	return DRE_OK;
}

