/* PSbuf.c
|
|	Prototypes of PS buffer management functions.
+-----------------------------------------------------------------------------*/
extern int
PSbufGet(
	int	pfd,
	int	pageno,
	PSfpage	**fpage,
	int	(*readfn)(),
	int	(*writefn)());

extern int
PSbufGet1(
	int	pfd,
	int	pageno,
	PSfpage	**fpage,
	int	(*readfn)(),
	int	(*writefn)(),
	bool_t	multiple_fixes);

extern int
PSbufUnfix(
	int	pfd,
	int	pageno);

extern int
PSbufAlloc(
	int	pfd,
	int	pageno,
	PSfpage	**fpage,
	int	(*writefn)());

extern int
PSbufReleaseFile(
	int	pfd,
	int	(*writefn)());

extern int
PSbufUsed(
	int	pfd,
	int	pageno);

extern void
PSbufPrint(void);




