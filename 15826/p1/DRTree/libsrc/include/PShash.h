/* PShash.h
|
|	Prototypes of Page Store (PS) hash table functions.
+-----------------------------------------------------------------------------*/
extern void
PShashInit(void);

extern PSbpage*
PShashFind(
	int	pfd,
	int	pageno);

extern int
PShashInsert(
	int	pfd,
	int	pageno,
	PSbpage	*bpage);

extern int
PShashDelete(
	int	pfd,
	int	pageno);

extern void
PShashPrint(void);


