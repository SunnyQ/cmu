/* PS.h
|
|	Page Store (PS) external header file.
+-----------------------------------------------------------------------------*/




/* system default constants
+-----------------------------------------------------------------------------*/
#define	PS_MAXFILES	20	/* maximum open files */
#define	PS_MAXBUFS	40	/* maximum buffer pages allowed in memory */
#define	PS_PAGEBUF_SIZE	1024 	/* size of each page buffer */




/* definitions of supporting types
+-----------------------------------------------------------------------------*/
/* boolean type */
typedef enum { FALSE, TRUE }	bool_t;




/* PS error numbers
+-----------------------------------------------------------------------------*/
#define PSE_OK			0
#define PSE_UNIX		(-1)
#define PSE_NOMEM		(-2)
#define PSE_NOBUF		(-3)
#define PSE_PAGEFIXED		(-4)
#define PSE_PAGENOTINBUF	(-5)
#define PSE_INCOMPLETEREAD	(-6)
#define PSE_INCOMPLETEWRITE	(-7)
#define PSE_HDRREAD		(-8)
#define PSE_HDRWRITE		(-9)
#define PSE_INVALIDPAGE		(-10)
#define PSE_FILEOPEN		(-11)
#define	PSE_FTABFULL		(-12)
#define PSE_FD			(-13)
#define PSE_EOF			(-14)
#define PSE_PAGEFREE		(-15)
#define PSE_PAGEUNFIXED		(-16)
#define PSE_PAGEINBUF		(-17)
#define PSE_HASHNOTFOUND	(-18)
#define PSE_HASHPAGEEXIST	(-19)




/* prototypes of PS interface functions
+-----------------------------------------------------------------------------*/
extern void
PS_Init(void);

extern int
PS_CreateFile(
	char	*filename);

extern int
PS_DestroyFile(
	char	*filename);

extern int
PS_OpenFile(
	char	*filename);

extern int
PS_CloseFile(
	int	pfd);

extern int
PS_RenameFile(
	char	*oldfile,
	char	*newfile);

extern int
PS_GetFirstPage(
	int	pfd,
	int	*pageno,
	char	**pagebuf);

extern int
PS_GetNextPage(
	int	pfd,
	int	*pageno,
	char	**pagebuf);

extern int
PS_GetThisPage(
	int	pfd,
	int	pageno,
	char	**pagebuf);

extern int
PS_AllocPage(
	int	pfd,
	int	*pageno,
	char	**pagebuf);

extern int
PS_DisposePage(
	int	pfd,
	int	pageno);

extern int
PS_UnfixPage(
	int	pfd,
	int	pageno,
	bool_t	dirty);

extern int
PS_DirtyPage(
	int	pfd,
	int	pageno);

extern void
PSperror(
	char	*s);


