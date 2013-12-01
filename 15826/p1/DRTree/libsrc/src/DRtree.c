/* DRtree.c
|
|	Deferred-split R-tree (DR) functions to manipulate index tree.
+-----------------------------------------------------------------------------*/
#include "DRdef.h"




/* DRtree functions
+-----------------------------------------------------------------------------*/


/* DRtreeSearchEntry() -
|
|	Functionality:
|	Search in an index tree rooted at "rootpage" of index file "rfd" for
|	all leaf entries whose cover rectangles match "testrect" by "testfn".
|	The leaf entries will be output by "outputfn".
|
|	Return Value:
|	>= 0	the number of leaf entries found;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DRtreeSearchEntry(
	int	rfd,		/* DR index file descriptor */
	int	rootpage,	/* root node page number */

	DRrect	*testrect,	/* rect for comparison */
	bool_t	(*testfn)(),	/* comparison function */
	int	(*outputfn)())	/* rect output function */
{
	int	errno;		/* error number */
	char	*pagebuf;	/* pointer to page buffer of node */
	int	count;		/* number of records found */
	int	i;		/* entry number */

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check "testrect" */
	if (DRrectInvalid(rfd, testrect))
		return DRerrSave(DRE_RECTINVALID);

	/* get the root node page */
	if (PS_GetThisPage(DRftab[rfd].pfd, rootpage, &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* initialize "count" and "errno" */
	errno = count = 0;

	/*--------------------------------------------------------------------*/
	/* if the node is NOT a leaf... */
	if (DRnodeLevel(rfd, pagebuf) > 0)
	{
		/* for each entry in the node... */
		for (i = DRnodeFirstEntry(rfd, pagebuf);
			i >= 0 && i < DRnodeCard(rfd);
			i = DRnodeNextEntry(rfd, i, pagebuf))
		{
			/* if "testrect" matches the rect of an entry... */
			if ((*testfn)(rfd, testrect,
				DRnodeRect(rfd, i, pagebuf)))
			{
				/* search the child of the entry */
				errno = DRtreeSearchEntry(rfd,
						DRnodeChild(rfd, i, pagebuf),
						testrect, testfn, outputfn);

				/* if bad things happened... */
				if (errno < 0)
					/* stop searching */
					break;

				/* increase counter */
				count += errno;
			}
		}
	}
	/*--------------------------------------------------------------------*/
	/* if the node IS a leaf... */
	else if (DRnodeLevel(rfd, pagebuf) == 0)
	{
		/* for each entry in the node... */
		for (i = DRnodeFirstEntry(rfd, pagebuf);
			i >= 0 && i < DRnodeCard(rfd);
			i = DRnodeNextEntry(rfd, i, pagebuf))
		{
			/* if "testrect" matches the rect of an entry... */
			if ((*testfn)(rfd, testrect,
				DRnodeRect(rfd, i, pagebuf)))
			{
				/* output the entry */
				(*outputfn)(rfd, i, pagebuf);

				/* increase counter */
				count++;
			}
		}
	}
	/*--------------------------------------------------------------------*/
	/* the level info in the root node is corrupted */
	else
		errno = DRerrSave(DRE_LEVELINVALID);

	/*--------------------------------------------------------------------*/
	/* unfix the root node page */
	if (PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* return error number if there is any */
	if (errno < 0)
		return errno;

	/* return the number of entries found */
	return count;
}


/* DRtreeInsertEntry() -
|
|	Functionality:
|	Insert an entry with "childno" and "childcov" at "level" of the index
|	tree rooted at "rootpage" of index file "rfd". New cover rectangle of
|	the root node will be returned in "rootcov". If root node splitted after
|	insertion, the page number of the new node will be returned in "newpage"
|	and its cover rect in "newcov".
|
|	Return Value:
|	= 0	success without split;
|	> 0	success with split;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DRtreeInsertEntry(
	int	rfd,		/* DR index file descriptor */
	int	rootpage,	/* root node page number */
	DRrect	*rootcov,	/* cover rect for root node after insertion */

	int	childno,	/* child number to insert */
	DRrect	*childcov,	/* cover rect of the child */
	int	level,		/* tree level to insert at */

	int	*newpage,	/* page number of a new node if root splitted */
	DRrect	*newcov)	/* cover rect for the new node */
{
	int	errno;		/* error number */
	char	*pagebuf;	/* pointer to page buffer of node */
	char	*newbuf;	/* pointer to page buffer of new node */
	int	entry;		/* entry number in a node */
	int	mypage;		/* temporary page number */

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* check "rootcov" */
	if (DRrectInvalid(rfd, rootcov))
		return DRerrSave(DRE_RECTINVALID);

	/* check "childno" */
	/* since "childno" can be a child page number or a record number */
	/* the checking should be more than this */
	if (childno < 0)
		return DRerrSave(DRE_CHILDINVALID);

	/* check "childcov" */
	if (DRrectInvalid(rfd, childcov))
		return DRerrSave(DRE_RECTINVALID);

	/* check "level" */
	if (level < 0)
		return DRerrSave(DRE_LEVELINVALID);

	/* get the root node page */
	if (PS_GetThisPage(DRftab[rfd].pfd, rootpage, &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* initialize "mypage" */
	mypage = -1;

	/*--------------------------------------------------------------------*/
	/* if it's ABOVE the level to insert... */
	if (DRnodeLevel(rfd, pagebuf) > level)
	{
		/* pick an entry whose cover rect needs the least enlargement */
		entry = DRnodePickEntry(rfd, childcov, pagebuf);

		/* if cannot find an entry */
		if (entry < 0)
			errno = entry;
		/*------------------------------------------------------------*/
		/* otherwise got the entry */
		else
		{
			/* insert in the child of the entry */
			errno = DRtreeInsertEntry(rfd,
					DRnodeChild(rfd, entry, pagebuf),
					DRnodeRect(rfd, entry, pagebuf),
					childno, childcov, level, newpage, newcov);

			/* if insertion is done... */
			if (errno >= 0)
			{
				/* adjust cover rect of root node */
				DRrectCombine(rfd, rootcov, childcov);
			}

			/* if insertion caused a split... */
			if (errno > 0)
			{
				/* insert the new node with its cover rect */
				/* at the current level */
				errno = DRtreeInsertEntry(rfd, rootpage, rootcov,
					*newpage, newcov,
					DRnodeLevel(rfd, pagebuf),
					newpage, newcov);

				/* NOTE: we have to make sure the two uses of */
				/* "newpage" and "newcov" won't cause trouble */
			}
		}
	}
	/*--------------------------------------------------------------------*/
	/* if it's the RIGHT level to insert... */
	else if (DRnodeLevel(rfd, pagebuf) == level)
	{
		/* if there is room to accommodate the new entry... */
		if (DRnodeCount(rfd, pagebuf) < DRnodeCard(rfd))
		{
			/* put the entry into the node */
			errno = DRnodePutEntry(rfd, childno, childcov, pagebuf);

			/* if the entry is settled... */
			if (errno >= 0)
			{
				/* adjust the root node cover rect */
				DRrectCombine(rfd, rootcov, childcov);

				/* insertion succeeded without a split */
				errno = 0;
			}
		}
		/*------------------------------------------------------------*/
		/* there is NO room, we have to split the node */
		else
		{
			/* allocate a page for the new node */
			mypage = DRnodeNew(rfd, DRnodeLevel(rfd, pagebuf), &newbuf);

			/* if allocation failed */
			if (mypage < 0)
				errno = mypage;
			/*----------------------------------------------------*/
			/* then allocation succeeded */
			else
			{
				/* split the entries between two nodes */
				errno = DRnodeSplit(rfd, childno, childcov,
						pagebuf, newbuf);

				/* if split succeeded */
				if (errno >= 0)
				{
					/* get cover rects for two nodes */
					DRnodeCover(rfd, rootcov, pagebuf);
					DRnodeCover(rfd, newcov, newbuf);

					/* the new page number */
					*newpage = mypage;

					/* insertion succeeded with a split */
					errno = 1;
				}
			}
		}
	}
	/*--------------------------------------------------------------------*/
	/* "level" is invalid */
	else
		errno = DRerrSave(DRE_LEVELINVALID);

	/*--------------------------------------------------------------------*/
	/* if insertion failed */
	if (errno < 0)
	{
		/* unfix the root node page */
		if (PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE) != PSE_OK)
		{
			PSperror("DRtreeInsertEntry");
			exit(1);
		}

		/* if we have a new page, dump it */
		if (mypage >= 0)
		{
			PS_UnfixPage(DRftab[rfd].pfd, mypage, FALSE);
			if (PS_DisposePage(DRftab[rfd].pfd, mypage) != PSE_OK)
			{
				PSperror("DRtreeInsertEntry");
				exit(2);
			}
		}
	}
	/*--------------------------------------------------------------------*/
	/* otherwise succeeded */
	else
	{
		/* unpin the root node page and mark it dirty */
		if (PS_UnfixPage(DRftab[rfd].pfd, rootpage, TRUE) != PSE_OK)
		{
			PSperror("DRtreeInsertEntry");
			exit(3);
		}

		/* if we have a new page, unpin it and mark it dirty */
		if (mypage >= 0 && PS_UnfixPage(DRftab[rfd].pfd, mypage, TRUE)
					!= PSE_OK)
		{
			PSperror("DRtreeInsertEntry");
			exit(4);
		}
	}

	/* return the errer number */
	return errno;
}


/* DRtreeDeleteEntry() -
|
|	Functionality:
|	Delete a leaf entry with "childno" from the index tree rooted at
|	"rootpage" of index file "rfd". The cover rect of the entry must
|	overlap with "testrect". New cover rect of the root node will be
|	returned in "rootcov".
|
|	Return Value:
|	> 1	success, with empty root node;
|	= 1	success otherwise;
|	= 0	entry not found;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DRtreeDeleteEntry(
	int	rfd,		/* DR index file descriptor */
	int	rootpage,	/* root node page number */
	DRrect	*rootcov,	/* cover rect for root node after deletion */

	int	childno,	/* record number to delete */
	DRrect	*testrect)	/* rect for comparison */
{
	int	errno;		/* error number */
	char	*pagebuf;	/* pointer to page buffer of node */
	int	mypage;		/* temporary page number */
	int	i;		/* loop control variable */

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

	/* get the root node page */
	if (PS_GetThisPage(DRftab[rfd].pfd, rootpage, &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* initialize "errno" and "mypage" */
	errno = 0;
	mypage = -1;

	/*--------------------------------------------------------------------*/
	/* if the node is NOT a leaf... */
	if (DRnodeLevel(rfd, pagebuf) > 0)
	{
		/* for each entry in the node... */
		for (i = DRnodeFirstEntry(rfd, pagebuf);
			i >= 0 && i < DRnodeCard(rfd);
			i = DRnodeNextEntry(rfd, i, pagebuf))
		{
			/* if "testrect" overlaps the rect of an entry... */
			if (DRrectOverlap(rfd, testrect,
				DRnodeRect(rfd, i, pagebuf)))
			{
				/* delete in the child of the entry */
				errno = DRtreeDeleteEntry(rfd,
						DRnodeChild(rfd, i, pagebuf),
						DRnodeRect(rfd, i, pagebuf),
						childno, testrect);

				/* if bad things happended during deletion... */
				if (errno < 0)
					/* stop deleting */
					break;

				/*--------------------------------------------*/
				/* if entry was deleted... */
				if (errno > 0)
				{
					/* if child node is empty... */
					if (errno > 1)
					{
						/* keep child page number */
						mypage = DRnodeChild(rfd, i, pagebuf);

						/* remove the entry */
						/* NOTE: test return value */
						DRnodeCutEntry(rfd, i, pagebuf);
					}

					/* if the root node is empty... */
					if (DRnodeCount(rfd, pagebuf) == 0)
					{
						/* if this is the root node */
						if (DRnodeLevel(rfd, pagebuf) ==
						    DRftab[rfd].hdr.rootlevel)
						{
							/* it's a leaf node now */
							DRnodeLevel(rfd, pagebuf) = 0;
						}
						/* set the empty signal */
						errno = 2;
					}
					/* otherwise it's not empty */
					else
					{
						/* adjust the root node cover rect */
						DRnodeCover(rfd, rootcov, pagebuf);

						/* set the non-empty signal */
						errno = 1;
					}

					/* done with deleting */
					break;
				}
			}
		}
	}
	/*--------------------------------------------------------------------*/
	/* if the node IS a leaf... */
	else if (DRnodeLevel(rfd, pagebuf) == 0)
	{
		/* for each entry in the node... */
		for (i = DRnodeFirstEntry(rfd, pagebuf);
			i >= 0 && i < DRnodeCard(rfd);
			i = DRnodeNextEntry(rfd, i, pagebuf))
		{
			/* if "testrect" overlaps the rect of an entry */
			/* and "childno" equals the childno of the entry */
			if (DRrectOverlap(rfd, testrect,
				DRnodeRect(rfd, i, pagebuf)) &&
				childno == DRnodeChild(rfd, i, pagebuf))
			{
				/* remove the entry from the node */
				errno = DRnodeCutEntry(rfd, i, pagebuf);

				/* if bad things happened... */
				if (errno < 0)
					break;

				/* if the root node is empty... */
				if (DRnodeCount(rfd, pagebuf) == 0)
				{
					/* if this is the root node */
					if (DRnodeLevel(rfd, pagebuf) ==
					    DRftab[rfd].hdr.rootlevel)
					{
						/* it's a leaf node now */
						DRnodeLevel(rfd, pagebuf) = 0;
					}
					/* set the empty signal */
					errno = 2;
				}
				/* otherwise it's not empty */
				else
				{
					/* adjust cover rect of root node */
					DRnodeCover(rfd, rootcov, pagebuf);

					/* set the non-empty signal */
					errno = 1;
				}

				/* done with deleting */
				break;
			}
		}
	}
	/*--------------------------------------------------------------------*/
	/* the level info in the root node is corrupted */
	else
		errno = DRerrSave(DRE_LEVELINVALID);

	/*--------------------------------------------------------------------*/
	/* if deletion failed or entry not found */
	if (errno <= 0)
	{
		/* unfix the root node page */
		if (PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE) != PSE_OK)
		{
			PSperror("DRtreeDeleteEntry");
			exit(1);
		}
	}
	/*--------------------------------------------------------------------*/
	/* otherwise entry removed */
	else
	{
		/* unpin the root node page and mark it dirty */
		if (PS_UnfixPage(DRftab[rfd].pfd, rootpage, TRUE) != PSE_OK)
		{
			PSperror("DRtreeDeleteEntry");
			exit(2);
		}

		/* if we have an empty child node, dump its page */
		if (mypage >= 0 && PS_DisposePage(DRftab[rfd].pfd, mypage)
					!= PSE_OK)
		{
			PSperror("DRtreeDeleteEntry");
			exit(3);
		}
	}

	/* return the error number */
	return errno;
}


/* DRtreePrint() -
|
|	Functionality:
|	Print an index tree rooted at "rootpage" of index file "rfd".
|
|	Return Value:
|	>= 0	success;
|	< 0	error conditions;
+-----------------------------------------------------------------------------*/
int DRtreePrint(
	int	rfd,		/* DR index file descriptor */
	int	rootpage)	/* root node page number */
{
	int	errno;		/* error number */
	char	*pagebuf;	/* pointer to page buffer of node */
	int	i;		/* loop control variable */

	/*--------------------------------------------------------------------*/
	/* check "rfd" */
	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	/* get the root node page */
	if (PS_GetThisPage(DRftab[rfd].pfd, rootpage, &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	/* print the root node */
	if ((errno = DRnodePrint(rfd, pagebuf)) != DRE_OK)
	{
		PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE);
		return errno;
	}

	/*--------------------------------------------------------------------*/
	/* if it's NOT a leaf node */
	if (DRnodeLevel(rfd, pagebuf) > 0)
	{
		/* for each subtree of this node */
		for (i = DRnodeFirstEntry(rfd, pagebuf);
			i >= 0 && i < DRnodeCard(rfd);
			i = DRnodeNextEntry(rfd, i, pagebuf))
		{
			/* print the subtree */
			errno = DRtreePrint(rfd, DRnodeChild(rfd, i, pagebuf));
			if (errno < 0)
			{
				PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE);
				return errno;
			}
		}
	}

	/* unfix the root node page */
	if (PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	return DRE_OK;
}



int DRtreePrintLevel(
	int	rfd,
        int     level,
	int	rootpage)
{
	int	errno;
	char	*pagebuf;	/* pointer to page buffer of node */
	int	i;		/* loop control variable */

	if (DRfdInvalid(rfd))
		return DRerrSave(DRE_RFDINVALID);

	if (PS_GetThisPage(DRftab[rfd].pfd, rootpage, &pagebuf) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	if (DRnodeLevel(rfd,pagebuf) == level) {
		for (i = DRnodeFirstEntry(rfd, pagebuf);
			i >= 0 && i < DRnodeCard(rfd);
			i = DRnodeNextEntry(rfd, i, pagebuf)) {
			DRrectPrintLevel(rfd,DRnodeRect(rfd,i,pagebuf));
			
		}
	}
	else if (level < 0 || DRnodeLevel(rfd, pagebuf) < level)
		fprintf(stderr, "The level is not a valid parameter or exceeds the depth of the tree\n");

	else if (DRnodeLevel(rfd, pagebuf) > level)
	{
		for (i = DRnodeFirstEntry(rfd, pagebuf);
			i >= 0 && i < DRnodeCard(rfd);
			i = DRnodeNextEntry(rfd, i, pagebuf))
		{
			errno = DRtreePrintLevel(rfd, level, DRnodeChild(rfd, i, pagebuf));
			if (errno < 0)
			{
				PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE);
				return errno;
			}
		}
	} 

	if (PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE) != PSE_OK)
		return DRerrSave(DRE_PSERROR);

	return DRE_OK;
}

int DRtreeCountRec(
	int	rfd,
	int	rootpage,
	int	count)
{
	char	*pagebuf;	/* pointer to page buffer of node */
	int	i;		/* loop control variable */

	PS_GetThisPage(DRftab[rfd].pfd, rootpage, &pagebuf);
	for (i = DRnodeFirstEntry(rfd, pagebuf);
		i >= 0 && i < DRnodeCard(rfd);
		i = DRnodeNextEntry(rfd, i, pagebuf)) {
		if (DRnodeLevel(rfd,pagebuf) == 0)
			count ++;
		else
			count = DRtreeCountRec(rfd, DRnodeChild(rfd, i, pagebuf), count);	
	}
	PS_UnfixPage(DRftab[rfd].pfd, rootpage, FALSE);
	return count;
}





