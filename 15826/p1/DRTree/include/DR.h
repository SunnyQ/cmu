/* DR.h
|
|	Deferred-split R-tree (DR) external header file.
+-----------------------------------------------------------------------------*/




/* definitions of supporting types
+-----------------------------------------------------------------------------*/
/* boolean type */
typedef enum { FALSE, TRUE }		bool_t;

/* coordinate type */
typedef enum { DR_INT, DR_FLOAT }	coord_t;

/* rectangle */
typedef	char	DRrect;




/* DR error numbers
+-----------------------------------------------------------------------------*/
#define	DRE_OK			0
#define	DRE_PSERROR		-1
#define	DRE_NOMEM		-2
#define	DRE_FILEEXIST		-3
#define	DRE_FTABFULL		-4
#define	DRE_DIMINVALID		-5
#define	DRE_CTYPEINVALID	-6
#define	DRE_RFDINVALID		-7
#define	DRE_PAGEINVALID		-8
#define	DRE_LEVELINVALID	-9
#define	DRE_CHILDINVALID	-10
#define	DRE_RECTINVALID		-11
#define	DRE_ENTRYINVALID	-12
#define	DRE_NODEINVALID		-13
#define	DRE_NODEEMPTY		-14
#define	DRE_NODEFULL		-15




/* prototypes of DR interface functions
+-----------------------------------------------------------------------------*/
extern void
DR_Init(void);

extern int
DR_CreateIndex(
	char	*filename,
	int	dim,
	coord_t	ctype);

extern int
DR_DestroyIndex(
	char	*filename);

extern int
DR_OpenIndex(
	char	*filename);

extern int
DR_CloseIndex(
	int	fd);

extern int
DR_SearchEntry(
	int	fd,
	DRrect	*testrect);

extern int
DR_InsertEntry(
	int	fd,
	int	childno,
	DRrect	*childcov);

extern int
DR_DeleteEntry(
	int	fd,
	int	childno,
	DRrect	*testrect);

extern int
DR_PrintIndex(
	int	fd);

extern int
DR_PrintLevel(
	int	rfd,
	int	level);

extern int DR_CountRec(
	int	rfd);



/* prototypes of DR rectangle functions
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

extern void
DRrectPrint(
	int	rfd,		/* R-tree index file descriptor */
	DRrect	*rect);		/* rect to print */




/* prototypes of DR error handling functions
+-----------------------------------------------------------------------------*/
extern void
DRperror(
	char	*msg);


