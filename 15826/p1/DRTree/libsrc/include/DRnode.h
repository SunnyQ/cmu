/* DRnode.h
|
|	Function prototypes for DRnode.
+-----------------------------------------------------------------------------*/
extern int
DRnodeFirstEntry(
	int	rfd,		/* R-tree index file descriptor */
	char	*pagebuf);	/* pointer to page buffer of node */

extern int
DRnodeNextEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	entry,		/* current entry number */
	char	*pagebuf);	/* pointer to page buffer of node */

extern int
DRnodeFreeEntry(
	int	rfd,		/* R-tree index file descriptor */
	char	*pagebuf);	/* pointer to page buffer of node */

extern bool_t
DRnodeMarkEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	entry,		/* entry number */
	bool_t	op,		/* operation */
	char	*pagebuf);	/* pointer to page buffer of node */

extern int
DRnodePutEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	childno,	/* child number to insert */
	DRrect	*childcov,	/* cover rect of the child */
	char	*pagebuf);	/* pointer to page buffer of node */

extern int
DRnodeCutEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	entry,		/* index of the entry in the node */
	char	*pagebuf);	/* pointer to page buffer of node */

extern int
DRnodePickEntry(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*testrect,	/* rect for testing */
	char	*pagebuf);	/* pointer to page buffer of node */

extern int
DRnodeCover(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*nodecov,	/* cover rect of the node */
	char	*pagebuf);	/* pointer to page buffer of node */

extern int
DRnodePrint(
	int	rfd,
	char	*pagebuf);

extern int
DRnodePrintEntry(
	int	rfd,
	int	entry,
	char	*pagebuf);


