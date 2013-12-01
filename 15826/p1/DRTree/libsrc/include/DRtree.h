/* DRtree.h
|
|	Function prototypes for DRtree.
+-----------------------------------------------------------------------------*/
extern int
DRtreeSearchEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	rootpage,	/* root node page number */

	DRrect	*testrect,	/* rect for comparison */
	bool_t	(*testfn)(),	/* comparison function */
	int	(*outputfn)());	/* rect output function */

extern int
DRtreeInsertEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	rootpage,	/* root node page number */
	DRrect	*rootcov,	/* cover rect for root node after insertion */

	int	childno,	/* child number to insert */
	DRrect	*childcov,	/* cover rect of the child */
	int	level,		/* tree level to insert at */

	int	*newpage,	/* page number of a new node if root splitted */
	DRrect	*newcov);	/* cover rect for the new node */

extern int
DRtreeDeleteEntry(
	int	rfd,		/* R-tree index file descriptor */
	int	rootpage,	/* root node page number */
	DRrect	*rootcov,	/* cover rect for root node after deletion */

	int	childno,	/* record number to delete */
	DRrect	*testrect);	/* rect for comparison */

extern int
DRtreePrint(
	int	rfd,		/* R-tree index file descriptor */
	int	rootpage);	/* root node page number */

extern int
DRtreePrintLevel(
	int	rfd,		/* R-tree index file descriptor */
	int 	level,
	int	rootpage);		/* root node page number */

extern int
DRtreeCountRec(
	int	rfd,
	int	rootpage,
	int	count);

/*----------------------------------------------------------------------------*/
