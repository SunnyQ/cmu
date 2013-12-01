/* DRrect.h
|
|	Prototypes for DRrect functions.
+-----------------------------------------------------------------------------*/
extern DRrect*
DRrectNew(
	int	rfd);		/* R-tree index file descriptor */

extern void
DRrectFree(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect);		/* rect to free */

extern int
DRrectGet(
	int	rfd,
	DRrect	*rect,
	bool_t	getrec);

extern int
DRrectGet2(
	int	rfd,
	DRrect	*rect,
	bool_t	getrec);

extern void
DRrectRand(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect);		/* random rectangle */

extern void
DRrectCopy(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect1,		/* target rectangle */
	DRrect	*rect2);	/* source rectangle */

extern void
DRrectCombine(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect1,		/* target rectangle */
	DRrect	*rect2);	/* source rectangle */

extern double
DRrectArea(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect);		/* rectangle */

extern float
DRrectWidth(
	int	rfd,		/* R-tree index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect);		/* rectangle */

extern float
DRrectGap(
	int	rfd,		/* R-tree index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2);	/* second rectangle */

extern bool_t
DRrectInvalid(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect);		/* rect to test */

extern bool_t
DRrectSame(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2);	/* second rectangle */

extern bool_t
DRrectOverlap(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2);	/* second rectangle */

extern bool_t
DRrectCover(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2);	/* second rectangle */

extern bool_t
DRrectHigherThan(
	int	rfd,		/* R-tree index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2);	/* second rectangle */

extern bool_t
DRrectLowerThan(
	int	rfd,		/* R-tree index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2);	/* second rectangle */

extern void
DRrectPrint(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect);		/* rect to print */


extern void
DRrectPrintLevel(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect);
