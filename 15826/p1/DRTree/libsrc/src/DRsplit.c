/* DRsplit.c
|
|	Supporting functions to split entries between two nodes.
+-----------------------------------------------------------------------------*/
#include "DRdef.h"




/* DRnodeSplit() -
|
+-----------------------------------------------------------------------------*/
int DRnodeSplit(
	int	rfd,
	int	childno,
	DRrect	*childcov,
	char	*pagebuf0,
	char	*pagebuf1)
{
	DRnodeSplitInit(rfd, childcov, pagebuf0);
	DRnodeSplitPickSeeds(rfd, childcov, pagebuf0);
	DRnodeSplitDistribute(rfd, childcov, pagebuf0);
	DRnodeSplitLoadNodes(rfd, childno, childcov, pagebuf0, pagebuf1);
	return DRE_OK;
}


/* DRnodeSplitInit() -
|
|	Initialize the partition structure.
+-----------------------------------------------------------------------------*/
int DRnodeSplitInit(
	int	rfd,
	DRrect	*childcov,
	char	*pagebuf)
{
	int	i;

	/* initialize "taken" and "area" */
	for (i = 0; i < DRnodeCard(rfd); i++)
	{
		DRftab[rfd].part.taken[i] = -1;
		DRftab[rfd].part.area[i] = DRrectArea(rfd,
						DRnodeRect(rfd, i, pagebuf));
	}
	{
		DRftab[rfd].part.taken[DRnodeCard(rfd)] = 0;
		DRftab[rfd].part.taken[DRnodeCard(rfd)] = DRrectArea(rfd,
								childcov);
	}

	/* initialize "splitcov" and "splitarea" */
	DRrectCopy(rfd, DRftab[rfd].part.splitcov, childcov);
	for (i = 0; i < DRnodeCard(rfd); i++)
		DRrectCombine(rfd, DRftab[rfd].part.splitcov,
			DRnodeRect(rfd, i, pagebuf));
	DRftab[rfd].part.splitarea = DRrectArea(rfd, DRftab[rfd].part.splitcov);

	/* initialize "partcount" */
	DRftab[rfd].part.partcount[0] = DRftab[rfd].part.partcount[1] = 0;
}


/* DRnodeSplitPickSeeds() -
|
|	Using quadratic splitting algorithm to select the first elements
|	of two partition groups.
+-----------------------------------------------------------------------------*/
int DRnodeSplitPickSeeds(
	int	rfd,
	DRrect	*childcov,
	char	*pagebuf)
{
	double	wastearea, wastemost;
	int	seed0, seed1, i, j;

	/* initialize "wastemost" */
	wastemost = -(DRftab[rfd].part.splitarea + 1);

	/* for all pairs of rects */
	for (i = 0; i < DRnodeCard(rfd); i++)
	for (j = i+1; j <= DRnodeCard(rfd); j++)
	{
		/* copy the secord rect to "temprect" */
		if (j == DRnodeCard(rfd))
			DRrectCopy(rfd, DRftab[rfd].temprect, childcov);
		else
			DRrectCopy(rfd, DRftab[rfd].temprect,
					DRnodeRect(rfd, j, pagebuf));

		/* combine the first rect with "temprect" */
		DRrectCombine(rfd, DRftab[rfd].temprect, DRnodeRect(rfd,i,pagebuf));

		/* find the wasted area */
		wastearea = DRrectArea(rfd, DRftab[rfd].temprect) -
				(DRftab[rfd].part.area[i] +
				 DRftab[rfd].part.area[j]);

		/* update "wastemost" and two seeds */
		if (wastearea > wastemost)
		{
			wastemost = wastearea;
			seed0 = i;
			seed1 = j;
		}
	}

	/* update the partition group */
	DRnodeSplitClassify(rfd, seed0, 0, childcov, pagebuf);
	DRnodeSplitClassify(rfd, seed1, 1, childcov, pagebuf);
}


int DRnodeSplitDistribute(
	int	rfd,
	DRrect	*childcov,
	char	*pagebuf)
{
	double	areagain[2], gaindiff, mostdiff;
	int	test, group, entry, i, j;

	/* while there is some entry has not been distributed */
	while (DRftab[rfd].part.partcount[0] + DRftab[rfd].part.partcount[1]
			<= DRnodeCard(rfd) &&
	       DRftab[rfd].part.partcount[0] < (DRnodeCard(rfd)+1)/2 &&
	       DRftab[rfd].part.partcount[1] < (DRnodeCard(rfd)+1)/2)
	{
		/* initialize "mostdiff" */
		mostdiff = -1;

		/* scan through "taken" */
		for (i = 0; i <= DRnodeCard(rfd); i++)
		/* if this entry has not been taken */
		if (DRftab[rfd].part.taken[i] < 0)
		{
			/* for each partition group */
			for (j = 0; j < 2; j++)
			{
				/* copy the entry cover to "temprect */
				if (i == DRnodeCard(rfd))
					DRrectCopy(rfd, DRftab[rfd].temprect,
						childcov);
				else
					DRrectCopy(rfd, DRftab[rfd].temprect,
						DRnodeRect(rfd, i, pagebuf));

				/* combine partition cover with "temprect" */
				DRrectCombine(rfd, DRftab[rfd].temprect,
					DRftab[rfd].part.partcov[j]);

				/* find "areagain" of each group */
				/* IF the entry cover goes to it */
				areagain[j] = DRrectArea(rfd, DRftab[rfd].temprect) -
						DRftab[rfd].part.partarea[j];
			}

			/* compute "gaindiff" between two group */
			gaindiff = areagain[1] - areagain[0];

			/* decide the better group to go */
			if (areagain[0] <= areagain[1])
				test = 0;
			else
			{
				test = 1;
				gaindiff *= -1;
			}

			/* if the current entry has the most tendency */
			if (gaindiff > mostdiff)
			{
				mostdiff = gaindiff;
				group = test;
				entry = i;
			}
			else if (gaindiff == mostdiff &&
				(DRftab[rfd].part.partarea[test] <
				 DRftab[rfd].part.partarea[group] ||
				 DRftab[rfd].part.partcount[test] <
				 DRftab[rfd].part.partcount[group]))
			{
				group = test;
				entry = i;
			}
		}

		/* classify "entry" to "group" */
		DRnodeSplitClassify(rfd, entry, group, childcov, pagebuf);
	}

	/* if there are some unclassified entries */
	if (DRftab[rfd].part.partcount[0] + DRftab[rfd].part.partcount[1]
			<= DRnodeCard(rfd))
	{
		/* find the group with fewer entries */
		if (DRftab[rfd].part.partcount[0] <=
		    DRftab[rfd].part.partcount[1])
			group = 0;
		else
			group = 1;

		/* classify the rest entries to this group */
		/* scan through "taken" */
		for (i = 0; i <= DRnodeCard(rfd); i++)
		/* if this entry has not been taken */
		if (DRftab[rfd].part.taken[i] < 0)
			DRnodeSplitClassify(rfd, i, group, childcov, pagebuf);
	}
}

int DRnodeSplitClassify(
	int	rfd,
	int	entry,
	int	group,
	DRrect	*childcov,
	char	*pagebuf)
{
	/* mark "entry" been taken by "group" */
	DRftab[rfd].part.taken[entry] = group;

	/* update the partition cover */
	if (entry == DRnodeCard(rfd))
	{
		if (DRftab[rfd].part.partcount[group] == 0)
			DRrectCopy(rfd, DRftab[rfd].part.partcov[group], childcov);
		else
			DRrectCombine(rfd, DRftab[rfd].part.partcov[group], childcov);
	}
	else
	{
		if (DRftab[rfd].part.partcount[group] == 0)
			DRrectCopy(rfd, DRftab[rfd].part.partcov[group],
				DRnodeRect(rfd, entry, pagebuf));
		else
			DRrectCombine(rfd, DRftab[rfd].part.partcov[group],
				DRnodeRect(rfd, entry, pagebuf));
	}

	/* update "partarea" */
	DRftab[rfd].part.partarea[group] =
		DRrectArea(rfd, DRftab[rfd].part.partcov[group]);

	/* update "partcount" */
	DRftab[rfd].part.partcount[group]++;
}


int DRnodeSplitLoadNodes(
	int	rfd,
	int	childno,
	DRrect	*childcov,
	char	*pagebuf0,
	char	*pagebuf1)
{
	int	i;

	/* for each entry in node0 */
	for (i = 0; i < DRnodeCard(rfd); i++)
	{
		if (DRftab[rfd].part.taken[i] == 1)
		{
			/* put the entry in node1 */
			DRnodePutEntry(rfd, DRnodeChild(rfd, i, pagebuf0),
				DRnodeRect(rfd, i, pagebuf0), pagebuf1);

			/* delete the entry in node0 */
			DRnodeCutEntry(rfd, i, pagebuf0);
		}
	}

	/* for the last entry */
	if (DRftab[rfd].part.taken[DRnodeCard(rfd)] == 0)
		DRnodePutEntry(rfd, childno, childcov, pagebuf0);
	else
		DRnodePutEntry(rfd, childno, childcov, pagebuf1);
}
