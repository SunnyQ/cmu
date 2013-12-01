/* DRrect.c
|
|	Deferred-split R-tree (DR) supporting functions to manipulate
|	rectangles.
+-----------------------------------------------------------------------------*/
#include "DRdef.h"




/* supporting macros
+-----------------------------------------------------------------------------*/
#define	min(x, y)		(((x) < (y)) ? (x) : (y))
#define	max(x, y)		(((x) > (y)) ? (x) : (y))

/* "r" has to start at word-boundaries */
#define	DRrectIntSide(r, s)	(*(((int*)r)+s))
#define	DRrectFloatSide(r, s)	(*(((float*)r)+s))




/* DRrectNew() -
|
|	Make a new rectangle.
|
|	Return Value:
|	> 0	pointer to a rectangle;
|	= 0	otherwise;
+-----------------------------------------------------------------------------*/
DRrect *DRrectNew(
	int	rfd)		/* DR index file descriptor */
{
	return (DRrect*)malloc(DRrectSize(rfd));
}


/* DRrectFree() -
|
|	Free a rect.
+-----------------------------------------------------------------------------*/
void DRrectFree(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect)		/* rect to free */
{
	free(rect);
}


/* DRrectGet() -
|
|	Get a rect from stdin. If "getrec" is TRUE, also get a record number.
+-----------------------------------------------------------------------------*/
int DRrectGet(
	int	rfd,
	DRrect	*rect,
	bool_t	getrec)
{
	int	recno;
	int	l, h;

	recno = -1;
	if (getrec == TRUE)
	{
		printf("Input record number: ");
		scanf("%d", &recno);
	}

	printf("Input rectangle coord:\n");
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			printf("dim[%d] low-side:  ", l);
			scanf("%d", &DRrectIntSide(rect, l));
			printf("dim[%d] high-side: ", l);
			scanf("%d", &DRrectIntSide(rect, h));
			break;

			case DR_FLOAT:
			printf("dim[%d] low-side:  ", l);
			scanf("%f", &DRrectFloatSide(rect, l));
			printf("dim[%d] high-side: ", l);
			scanf("%f", &DRrectFloatSide(rect, h));
			break;
		}
	}

	return recno;
}

/* DRrectGet2() -
|
|	Get a rect from stdin with a given epsilon 'radius'.
+-----------------------------------------------------------------------------*/
int DRrectGet2(
	int	rfd,
	DRrect	*rect,
	bool_t	getrec)
{
	int	recno;
	int	l, h;
	int	iepsilon;
	float	fepsilon;

	recno = -1;
	if (getrec == TRUE)
	{
		printf("Input record number: ");
		scanf("%d", &recno);
	}

	printf("Input tolerance:\n");
	switch (DRcoordType(rfd))
	{
		case DR_INT:
		scanf("%d",&iepsilon);
		break;
		case DR_FLOAT:
		scanf("%f",&fepsilon);
		break;
	}
	printf("Input center coord:\n");
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			printf("dim[%d] low-side:  ", l);
			scanf("%d", &DRrectIntSide(rect, l));
			DRrectIntSide(rect, l)=DRrectIntSide(rect,l)-iepsilon;
			printf("dim[%d] high-side: ", l);
			scanf("%d", &DRrectIntSide(rect, h));
			DRrectIntSide(rect, h)=DRrectIntSide(rect,h)+iepsilon;
			break;

			case DR_FLOAT:
			printf("dim[%d] low-side:  ", l);
			scanf("%f", &DRrectFloatSide(rect, l));
			DRrectFloatSide(rect, l)=DRrectFloatSide(rect,l)-fepsilon;
			printf("dim[%d] high-side: ", l);
			scanf("%f", &DRrectFloatSide(rect, h));
			DRrectFloatSide(rect, h)=DRrectFloatSide(rect,h)+fepsilon;
			break;
		}
	}

	return recno;
}
	

/* DRrectRand() -
|
|	Set "rect" with random coordinates.
+-----------------------------------------------------------------------------*/
void DRrectRand(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect)		/* random rectangle */
{
	int	l;		/* low-side number of rect */
	int	h;		/* high-side number of rect */
	int	width;		/* width along one dimension of rect */

	/* for each dimension... */
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		/* get the width along this dimension */
		width = (int)(random() % (1000/4) + 1);

		/* for each case of coordinate type */
		/* set the low-side coordinates and */
		/* the high-side coordinates */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			DRrectIntSide(rect, l) = random() % (1000-width);
			DRrectIntSide(rect, h) = DRrectIntSide(rect, l) + width;
			break;

			case DR_FLOAT:
			DRrectFloatSide(rect, l) = random() % (1000-width);
			DRrectFloatSide(rect, h) = DRrectFloatSide(rect, l) + width;
			break;

			default:
			printf("DRrectRand(): unknown coord type\n");
			exit(1);
			break;
		}
	}
}


/* DRrectCopy() -
|
|	Copy "rect2" to "rect1".
+-----------------------------------------------------------------------------*/
void DRrectCopy(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect1,		/* target rectangle */
	DRrect	*rect2)		/* source rectangle */
{
	/* it's easy */
	memcpy(rect1, rect2, DRrectSize(rfd));
}


/* DRrectCombine() -
|
|	Combine "rect1" and "rect2", make "rect1" to include both.
+-----------------------------------------------------------------------------*/
void DRrectCombine(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect1,		/* target rectangle */
	DRrect	*rect2)		/* source rectangle */
{
	int	l;		/* low-side number of rect */
	int	h;		/* high-side number of rect */

	/* for each dimension... */
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		/* for each case of coordinate type */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			DRrectIntSide(rect1, l) = min(DRrectIntSide(rect1, l),
						      DRrectIntSide(rect2, l));
			DRrectIntSide(rect1, h) = max(DRrectIntSide(rect1, h),
						      DRrectIntSide(rect2, h));
			break;

			case DR_FLOAT:
			DRrectFloatSide(rect1, l) = min(DRrectFloatSide(rect1, l),
							DRrectFloatSide(rect2, l));
			DRrectFloatSide(rect1, h) = max(DRrectFloatSide(rect1, h),
							DRrectFloatSide(rect2, h));
			break;

			default:
			printf("DRrectCombine(): unknown coord type\n");
			exit(1);
			break;
		}
	}
}


/* DRrectArea() -
|
|	Calculate the area of "rect".
+-----------------------------------------------------------------------------*/
double DRrectArea(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect)		/* rectangle */
{
	double	area;		/* area of rect */
	int	dim;		/* dimension number */

	/* initialize "area" */
	area = 1;

	/* for each dimension... */
	for (dim = 0; dim < DRdim(rfd); dim++)
	{
		/* for each case of coordinate type */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			area *= DRrectIntSide(rect, dim+DRdim(rfd)) -
				DRrectIntSide(rect, dim);
			break;

			case DR_FLOAT:
			area *= DRrectFloatSide(rect, dim+DRdim(rfd)) -
				DRrectFloatSide(rect, dim);
			break;

			default:
			printf("DRrectArea(): unknown coord type\n");
			exit(1);
			break;
		}
	}

	return area;
}


/* DRrectWidth() -
|
|	Calculate the width of "rect" along one dimension.
+-----------------------------------------------------------------------------*/
float DRrectWidth(
	int	rfd,		/* DR index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect)		/* rectangle */
{
	/* for each case of coordinate type */
	switch (DRcoordType(rfd))
	{
		case DR_INT:
		return DRrectIntSide(rect, dim+DRdim(rfd)) -
			DRrectIntSide(rect, dim);

		case DR_FLOAT:
		return DRrectFloatSide(rect, dim+DRdim(rfd)) -
			DRrectFloatSide(rect, dim);

		default:
		printf("DRrectWidth(): unknown coord type\n");
		exit(1);
	}
}


/* DRrectGap() -
|
|	Calculate the gap between two rects along one dimension.
+-----------------------------------------------------------------------------*/
float DRrectGap(
	int	rfd,		/* DR index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2)		/* second rectangle */
{
	/* for each case of coordinate type */
	switch (DRcoordType(rfd))
	{
		case DR_INT:
		return DRrectIntSide(rect1, dim) -
			DRrectIntSide(rect2, dim+DRdim(rfd));

		case DR_FLOAT:
		return DRrectFloatSide(rect1, dim) -
			DRrectFloatSide(rect2, dim+DRdim(rfd));

		default:
		printf("DRrectGap(): unknown coord type\n");
		exit(1);
	}
}


/* DRrectInvalid() -
|
|	Test if "rect" is NOT valid.
+-----------------------------------------------------------------------------*/
bool_t DRrectInvalid(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect)		/* rect to test */
{
	int	l;		/* low-side number of rect */
	int	h;		/* high-side number of rect */

	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		switch(DRcoordType(rfd))
		{
			case DR_INT:
			if (DRrectIntSide(rect, l) > DRrectIntSide(rect, h))
				return TRUE;	/* NOT valid */
			break;

			case DR_FLOAT:
			if (DRrectFloatSide(rect, l) > DRrectFloatSide(rect, h))
				return TRUE;	/* NOT valid */
			break;

			default:
			printf("DRrectInvalid(): unknown coord type\n");
			exit(1);
			break;
		}
	}

	return FALSE;	/* valid */
}


/* DRrectSame() -
|
|	Test if two rects are the same.
+-----------------------------------------------------------------------------*/
bool_t DRrectSame(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2)		/* second rectangle */
{
	int	sides;		/* number of sides */
	int	i;		/* loop control variable */

	/* initialize "sides" */
	sides = 2 * DRdim(rfd);

	/* for each side... */
	for (i = 0; i < sides; i++)
	{
		/* for each coordinate type */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			if (DRrectIntSide(rect1, i) != DRrectIntSide(rect2, i))
				return FALSE;
			break;

			case DR_FLOAT:
			if (DRrectFloatSide(rect1, i) != DRrectFloatSide(rect2, i))
				return FALSE;
			break;

			default:
			printf("DRrectSame(): unknown coord type\n");
			exit(1);
			break;
		}
	}

	return TRUE;
}


/* DRrectOverlap() -
|
|	Test if two rects overlap.
+-----------------------------------------------------------------------------*/
bool_t DRrectOverlap(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2)		/* second rectangle */
{
	int	l;		/* low-side number of rect */
	int	h;		/* high-side number of rect */

	/* for each dimension... */
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		/* for each case of coordinate type */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			if (DRrectIntSide(rect1, l) > DRrectIntSide(rect2, h) ||
			    DRrectIntSide(rect1, h) < DRrectIntSide(rect2, l))
				return FALSE;
			break;

			case DR_FLOAT:
			if (DRrectFloatSide(rect1, l) > DRrectFloatSide(rect2, h) ||
			    DRrectFloatSide(rect1, h) < DRrectFloatSide(rect2, l))
				return FALSE;
			break;

			default:
			printf("DRrectOverlap(): unknown coord type\n");
			exit(1);
			break;
		}
	}

	return TRUE;
}


/* DRrectCover() -
|
|	Test if one rect covers another.
+-----------------------------------------------------------------------------*/
bool_t DRrectCover(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2)		/* second rectangle */
{
	int	l;		/* low-side number of rect */
	int	h;		/* high-side number of rect */

	/* for each dimension... */
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		/* for each case of coordinate type */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			if (DRrectIntSide(rect1, l) > DRrectIntSide(rect2, l) ||
			    DRrectIntSide(rect1, h) < DRrectIntSide(rect2, h))
				return FALSE;
			break;

			case DR_FLOAT:
			if (DRrectFloatSide(rect1, l) > DRrectFloatSide(rect2, l) ||
			    DRrectFloatSide(rect1, h) < DRrectFloatSide(rect2, h))
				return FALSE;
			break;

			default:
			printf("DRrectCover(): unknown coord type\n");
			exit(1);
			break;
		}
	}

	return TRUE;
}


/* DRrectHigherThan() -
|
|	Test if one rect is higher than another along one dimension.
+-----------------------------------------------------------------------------*/
bool_t DRrectHigherThan(
	int	rfd,		/* DR index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2)		/* second rectangle */
{
	/* for each case of coordinate type */
	switch (DRcoordType(rfd))
	{
		case DR_INT:
		if (DRrectIntSide(rect1, dim) <= DRrectIntSide(rect2, dim))
			return FALSE;
		break;

		case DR_FLOAT:
		if (DRrectFloatSide(rect1, dim) <= DRrectFloatSide(rect2, dim))
			return FALSE;
		break;

		default:
		printf("DRrectHigherThan(): unknown coord type\n");
		exit(1);
		break;
	}

	return TRUE;
}


/* DRrectLowerThan() -
|
|	Test if one rect is lower than another along one dimension.
+-----------------------------------------------------------------------------*/
bool_t DRrectLowerThan(
	int	rfd,		/* DR index file descriptor */
	int	dim,		/* dimension number */
	DRrect	*rect1,		/* first rectangle */
	DRrect	*rect2)		/* second rectangle */
{
	/* for each case of coordinate type */
	switch (DRcoordType(rfd))
	{
		case DR_INT:
		if (DRrectIntSide(rect1, dim+DRdim(rfd)) >=
		    DRrectIntSide(rect2, dim+DRdim(rfd)))
			return FALSE;
		break;

		case DR_FLOAT:
		if (DRrectFloatSide(rect1, dim+DRdim(rfd)) >=
		    DRrectFloatSide(rect2, dim+DRdim(rfd)))
			return FALSE;
		break;

		default:
		printf("DRrectLowerThan(): unknown coord type\n");
		exit(1);
		break;
	}

	return TRUE;
}


/* DRrectPrint() -
|
|	Print a rect.
+-----------------------------------------------------------------------------*/
void DRrectPrint(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect)		/* rect to print */
{
	int	l;		/* low-side number of rect */
	int	h;		/* high-side number of rect */

	/* for each dimension */
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		/* for each case of coordinate type */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			printf("\t%d\t%d\n",DRrectIntSide(rect, l),
					    DRrectIntSide(rect, h));
			break;

			case DR_FLOAT:
			printf("\t%f\t%f\n",DRrectFloatSide(rect, l),
					    DRrectFloatSide(rect, h));
			break;

			default:
			printf("DRrectPrint(): unknown coord type\n");
			exit(1);
			break;
		}
	}
}


void DRrectPrintLevel(
	int	rfd,		/* DR index file descriptor */
	DRrect	*rect)		/* rect to print */
{
	int	l;		/* low-side number of rect */
	int	h;		/* high-side number of rect */
	char	*format;

	/* for each dimension */
	printf("(");
	for (l = 0, h = DRdim(rfd); l < DRdim(rfd); l++, h++)
	{
		format = (l == DRdim(rfd) - 1) ? "" : ", ";
		/* for each case of coordinate type */
		switch (DRcoordType(rfd))
		{
			case DR_INT:
			printf("%d, %d%s",DRrectIntSide(rect, l),
					    DRrectIntSide(rect, h), format);
			break;

			case DR_FLOAT:
			printf("%f, %f%s",DRrectFloatSide(rect, l),
					    DRrectFloatSide(rect, h), format);
			break;

			default:
			printf("DRrectPrint(): unknown coord type\n");
			exit(1);
			break;
		}
	}
	printf(")\n");
}


