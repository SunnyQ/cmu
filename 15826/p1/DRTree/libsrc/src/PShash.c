/* PShash.c
|
|	Page Store (PS) hash table functions.
+-----------------------------------------------------------------------------*/
#include "PSdef.h"




/* global variable
+-----------------------------------------------------------------------------*/
/* hash table */
static PShash_entry *PShtab[PS_HTAB_SIZE];




/* interface functions
+-----------------------------------------------------------------------------*/


/* PShashInit() -
|
|	Initialize the PS hash table.
+-----------------------------------------------------------------------------*/
void PShashInit(void)
{
	int	i;

	for (i = 0; i < PS_HTAB_SIZE; i++)
		PShtab[i] = NULL;
}


/* PShashFind() -
|
|	Given a PS file descriptor "pfd" and a page number "pageno",
|	find the buffer address of this particular page.
|
|	Return Value:
|	Non-Null	success;
|	Null		otherwise;
+-----------------------------------------------------------------------------*/
PSbpage *PShashFind(
	int	pfd,
	int	pageno)
{
	PShash_entry	*entry;		/* ptr to entries to search */
	int		bucket;		/* bucket to look for the page*/

	/* See which bucket it is in */
	bucket = PShash(pfd, pageno);

	/* go through the linked list of this bucket */
	for (entry = PShtab[bucket]; entry != NULL; entry = entry->next)
	{
		if (entry->pfd == pfd && entry->pageno == pageno)
			/* found it */
			return(entry->bpage);
	}

	/* not found */
	return(NULL);
}


/* PShashInsert() -
|
|	Insert a PS file descriptor "pfd", a page number "pageno",
|	and the buffer address "bpage" into the hash table. 
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PShashInsert(
	int	pfd,
	int	pageno,
	PSbpage	*bpage)
{
	PShash_entry	*entry;		/* pointer to new entry */
	int		bucket;		/* bucket to insert the page */

	if (PShashFind(pfd, pageno) != NULL)
		/* page already inserted */
		return PSerrSave(PSE_HASHPAGEEXIST);

	/* find the bucket for this page */
	bucket = PShash(pfd, pageno);
	
	/* allocate mem for new entry */
	if ((entry = (PShash_entry*)malloc(PS_HBUCKET_SIZE)) == NULL)
		/* no mem */
		return PSerrSave(PSE_NOMEM);

	/* Insert entry as head of list for this bucket */
	entry->pfd = pfd;
	entry->pageno = pageno;
	entry->bpage = bpage;
	entry->next = PShtab[bucket];
	entry->prev = NULL;
	if (PShtab[bucket] != NULL)
		PShtab[bucket]->prev = entry;
	PShtab[bucket] = entry;

	return(PSE_OK);
}

/* PShashDelete() -
|
|	Delete the entry whose PS file descriptor is "pfd",
|	and whose page number is "pageno" from the hash table.
|
|	Return Value:
|	= 0	success;
|	< 0	otherwise;
+-----------------------------------------------------------------------------*/
int PShashDelete(
	int	pfd,
	int	pageno)
{
	PShash_entry	*entry;		/* entry to look for */
	int		bucket;		/* bucket for this page */

	/* find the bucket */
	bucket = PShash(pfd, pageno);

	/* See if the entry is in this bucket */
	for (entry = PShtab[bucket]; entry != NULL; entry = entry->next)
		if (entry->pfd == pfd && entry->pageno == pageno)
			break;

	if (entry == NULL)
		/* not found */
		return PSerrSave(PSE_HASHNOTFOUND);

	/* get rid of this entry */
	if (entry == PShtab[bucket])
		PShtab[bucket] = entry->next;
	if (entry->prev != NULL)
		entry->prev->next = entry->next;
	if (entry->next != NULL)
		entry->next->prev = entry->prev;
	free((char*)entry);

	return(PSE_OK);
}


/* PShashPrint() -
|
|	Print the hash table entries.
+-----------------------------------------------------------------------------*/
void PShashPrint(void)
{
	PShash_entry	*entry;
	int		i;

	for (i = 0; i < PS_HTAB_SIZE; i++)
	{
		printf("bucket %d\n", i);
		if (PShtab[i] == NULL)
			printf("\tempty\n");
		else
		{
			for (entry = PShtab[i]; entry != NULL; entry = entry->next)
				printf("\tpfd: %d, pageno: %d %d\n",
					entry->pfd, entry->pageno, entry->bpage);
		}
	}
}


