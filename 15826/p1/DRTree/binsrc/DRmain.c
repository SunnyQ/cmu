/* DRmain.c
|
|	An example showing how to use DR interface functions.
+-----------------------------------------------------------------------------*/
#include <stdio.h>	/* for scanf() */
/* christos, 3/5/2011 */
#include <stdlib.h>
#include <unistd.h>	/* for access() */
#include <math.h>
#include "DR.h"




/* defaults
+-----------------------------------------------------------------------------*/
/* index file name */
#define	DEFILE	"default.idx"

/* dimension */
#define	DIM	2

#define MINFLOAT -1.0e+30
#define MAXFLOAT 1.0e+30




/* command line variables
+-----------------------------------------------------------------------------*/
int	dim = DIM;
coord_t	ctype = DR_INT;
char	*filename = DEFILE;

typedef struct irect {
        DRrect          *rect;
        int             index;
}IRect;

typedef struct nntype {
        DRrect          *rect;
        float           dist;
} Nntype;

static DRrect *qrect;
static int rrfd;

int DR_NearNeighbor(int, DRrect *);	/* prototype */


/* supporting function for command line processing
+-----------------------------------------------------------------------------*/
void command_line(
	int	argc,
	char	*argv[])
{
	int	i;

	/* start command line processing */
	printf("%s: Welcome!\n\n", argv[0]);
	for (i = 1; i < argc; i++)
	{
		/* it's a command line option */
		if (argv[i][0] == '-')
		switch (argv[i][1])
		{
			/* help */
			case 'h':
			printf("Usage: %s [ -help ] [ -dim n ] [ -float ] [ filename ]\n", argv[0]);
			exit(0);
			break;

			/* dimension */
			case 'd':
			i++;
			sscanf(argv[i], "%d", &dim);
			break;

			/* float */
			case 'f':
			ctype = DR_FLOAT;
			break;

			/* default */
			default:
			printf("%s: Unknown option '%c', ignored.\n\n", argv[0], argv[i][1]);
			break;
		}
		/* it's a file name */
		else
			filename = argv[i];
	}
}




/* main function
+-----------------------------------------------------------------------------*/
main(
	int	argc,
	char	*argv[])
{
	char	line[80];
	int	rfd;
	int	errno;
	int	recno;
	DRrect	*rect;
	int	level;
	int	count;

	/* process command line arguments */
	command_line(argc, argv);

	/* initialize DR */
	DR_Init();

	/* create a new index if necessary */
	if (access(filename, F_OK) < 0 &&
		/* file doesn't exist, create it */
		DR_CreateIndex(filename, dim, ctype) != DRE_OK)
	{
		DRperror("main");
		exit(1);
	}

	/* open index */
	if ((rfd = DR_OpenIndex(filename)) < 0)
	{
		DRperror("main");
		exit(2);
	}
	rect = (DRrect*)DRrectNew(rfd);

	/* start trans loop */
	line[0] = 'q' + 1;
	while (line[0] != 'q')
	{
		printf("(k)nn (s)earch (i)nsert (d)elete (p)rint d(r)op (q)uit\n");
		scanf("%s",line);
		switch (line[0])
		{
			/* knn query */
			case 'k':
			DRrectGet(rfd, rect, FALSE);
			errno = DR_NearNeighbor(rfd, rect);
			if (errno < 0)
				DRperror("knn");
			else
				printf("knn: %d record%s found\n", errno,
						(errno>1)?"s":"");
			break;

			/* search */
			case 's':
			DRrectGet2(rfd, rect, FALSE);
			errno = DR_SearchEntry(rfd, rect);
			if (errno < 0)
				DRperror("search");
			else
				printf("search: %d record%s found\n", errno,
						(errno>1)?"s":"");
			break;

			/* insert */
			case 'i':
			recno = DRrectGet(rfd, rect, TRUE);
			errno = DR_InsertEntry(rfd, recno, rect);
			if (errno < 0)
				DRperror("insert");
			else
				printf("insert: record %d insert OK\n", recno);
			break;

			/* delete */
			case 'd':
			recno = DRrectGet(rfd, rect, TRUE);
			errno = DR_DeleteEntry(rfd, recno, rect);
			if (errno < 0)
				DRperror("delete");
			else if (errno == 0)
				printf("delete: record %d not found\n", recno);
			else
				printf("delete: record %d delete OK\n", recno);
			break;

			/* print */
			case 'p':
			DR_PrintIndex(rfd);
			break;

			/* destroy */
			case 'r':
			DR_CloseIndex(rfd);
			DR_DestroyIndex(filename);
			exit(0);
			break;

			/* Print out MBRs at level */
			case 'l':
			scanf("%d",&level);
			//Please do not remove this comment for HW1.
			DR_PrintLevel(rfd, level);
			break;

			/* Count all data rectangles */
			case 'c':
			//Please do not remove this comment for HW1.
			count = DR_CountRec(rfd);
			printf("The number of data rectangles is %d\n", count);
			break;
			
			/* random */
			case 'a':
			break;
		}
		printf("\n");
	}

	/* close index */
	if (DR_CloseIndex(rfd) != DRE_OK)
	{
		DRperror("main");
		exit(3);
	}

	DRrectFree(rfd, rect);

	printf("%s: Goodbye!\n\n", argv[0]);
}
