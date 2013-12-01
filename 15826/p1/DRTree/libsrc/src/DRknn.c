/* DRknn.c
|
|	Deferred-split R-tree (DR) k-nearest neighbor search
+-----------------------------------------------------------------------------*/
#include <math.h>
#include "DRdef.h"

#define DRrectIntSide(r, s)     (*(((int*)r)+s))
#define DRrectFloatSide(r, s)   (*(((float*)r)+s))
#define max(X,Y) ((X) > (Y) ? (X) : (Y))
#define MINFLOAT -1.0e+30
#define MAXFLOAT 1.0e+30

typedef union { int ival; float fval; } intfloat;

typedef struct irect {
	intfloat *rectl;
	intfloat *recth;
        int      page;
}IRect;

typedef struct nntype {
	intfloat *rectl;
	intfloat *recth;
	int      childno;
        double   dist;
} Nntype;


static int rrfd;		/* file descriptor */
static DRrect *qrect;		/* query rectangle */
Nntype *nnarr;			/* array of k nearest neighbors */
static int lastNNinArr;		/* number of nnarr elements assigned */

/* DR file table */
DRftab_entry	DRftab[DR_FTAB_SIZE];


void
rectToArr(DRrect *r, intfloat *a, intfloat *b)
{
	int l;		/* low-side number of rect */
	int h;		/* high-side number of rect */

	        /* for each dimension... */
        for (l = 0, h = DRdim(rrfd); l < DRdim(rrfd); l++, h++)
        {
                /* for each case of coordinate type */
                switch (DRcoordType(rrfd))
                {
                        case DR_INT:
			a[l].ival = DRrectIntSide(r, l);
			b[l].ival = DRrectIntSide(r, h);
                        break;

                        case DR_FLOAT:
			a[l].fval = DRrectFloatSide(r, l);
			b[l].fval = DRrectFloatSide(r, h);
                        break;
		}
	}
}


/* supporting functions */

/*----------------------------------------------------------------------
| containInQuery()
|       if rectangle "r" is completely in "rectp" return MINFLOAT
|       else return 0
+----------------------------------------------------------------------*/
double
containInQuery(DRrect *rectp, intfloat *rl, intfloat *rh)
{
        int             l = 0;		/* lower-left corner of rect */
	int		dim=DRdim(rrfd);/* dimensionality of hyper-rectangle */
	int		h = dim;	/* upper-right corner of rect */

	switch (DRcoordType(rrfd))
	{
		case DR_INT:
		while ((DRrectIntSide(rectp,l) <= rl[l].ival) &&
        	(DRrectIntSide(rectp,h) >= rh[l].ival) &&
        	(l < dim )) {
			l++; h++; }
		break;

		case DR_FLOAT:
		while ((DRrectFloatSide(rectp,l) <= rl[l].fval) &&
        	(DRrectFloatSide(rectp,h) >= rh[l].fval) &&
        	(l < dim )) {
			l++; h++; }
		break;
	}

        if (l == dim)
                { return 0.0; }
        return 0;

}

/*----------------------------------------------------------------------
| assignNegDist()
|       return negative distance to rectangle "r" which overlap query point
|       "rectp". The most negative distance is assigned to rectangle whose
|       center is nearest to query point.
+----------------------------------------------------------------------*/

double
assignNegDist(DRrect *rectp, intfloat *rl, intfloat *rh)
{
        int             l,h;
        double          total=0.0, d, d1, d2;
	int		dim=DRdim(rrfd);

	for (l = 0, h = dim; l < dim; l++, h++)
	{
		switch (DRcoordType(rrfd))
		{
			case DR_INT:
        		d1 = max(DRrectIntSide(rectp,l) - rl[l].ival, 0);
        		d2 = max(rh[l].ival - DRrectIntSide(rectp,h), 0);
        		d =  max(d1,d2);
        		total += d*d;
			break;

			case DR_FLOAT:
        		d1 = max(DRrectFloatSide(rectp,l) - rl[l].fval, 0);
        		d2 = max(rh[l].fval - DRrectFloatSide(rectp,h), 0);
        		d =  max(d1,d2);
        		total += d*d;
			break;
		}
	}

        total = pow( (double) total, 1/(double) dim);
        return  - 1/(1+total);
}


/*----------------------------------------------------------------------
| minMaxDist()
|       returns the distance from rectangle "r" to query point "rectp"
+----------------------------------------------------------------------*/
double
minMaxDist(DRrect *rectp, intfloat *rl, intfloat *rh)
/* rectp is pointer to the point in space */
{
        float           b, rm;
        int             l, h;		/* low, high indices of rect diag */
        double          sum = 0.0;
	double		minaddon = MAXFLOAT;
	double		addon;
	int		dim = DRdim(rrfd);

	for (l = 0, h = dim; l < dim; l++, h++)
	{
                switch (DRcoordType(rrfd))
                {
			case DR_INT:
			if (DRrectIntSide(rectp,h) >= ((rl[l].ival+rh[l].ival)/2))
				b = DRrectIntSide(rectp,h)-rl[l].ival;
			else  b = DRrectIntSide(rectp,l)-rh[l].ival;
			if (DRrectIntSide(rectp,h) <= ((rl[l].ival+rh[l].ival)/2))
				rm = DRrectIntSide(rectp,h)-rl[l].ival;
			else  rm = DRrectIntSide(rectp,l)-rh[l].ival;
			addon = rm*rm - b*b;
			if (addon < minaddon) minaddon = addon;
                        break;
 
                        case DR_FLOAT:
			if (DRrectFloatSide(rectp,h) >= ((rl[l].fval+rh[l].fval)/2))
				b = DRrectFloatSide(rectp,h)-rl[l].fval;
			else  b = DRrectFloatSide(rectp,l)-rh[l].fval;
			if (DRrectFloatSide(rectp,h) <= ((rl[l].fval+rh[l].fval)/2))
				rm = DRrectFloatSide(rectp,h)-rl[l].fval;
			else  rm = DRrectFloatSide(rectp,l)-rh[l].fval;
			addon = rm*rm - b*b;
			if (addon < minaddon) minaddon = addon;
                        break;
                }
		sum += pow( (double) b, (double) 2 );
        }
	sum += minaddon;
        if (sum == 0.0) {         /* rect r overlap query rectp */
                if ((sum = containInQuery(rectp, rl,rh)) == 0.0)
                        return assignNegDist(rectp, rl,rh);
                return sum;
        }
	return sum;
}


/*----------------------------------------------------------------------
| minDist()
|       returns the distance from rectangle "r" to query point "rectp"
+----------------------------------------------------------------------*/
double
minDist(DRrect *rectp, intfloat *rl, intfloat *rh)
/* rectp is pointer to the query point in space */
{

        float           b;
        int             l, h;
        double          sum = 0.0;
	int		dim = DRdim(rrfd);

	for (l = 0, h = dim; l < dim; l++, h++)
	{
                switch (DRcoordType(rrfd))
                {
                        case DR_INT:
			if (DRrectIntSide(rectp,h) < rl[l].ival)
				b = DRrectIntSide(rectp,h) - rl[l].ival;
			else if (DRrectIntSide(rectp,h) > rh[l].ival)
				b = DRrectIntSide(rectp,l) - rh[l].ival;
			else	b = 0.0;
                        break;
 
                        case DR_FLOAT:
			if (DRrectFloatSide(rectp,h) < rl[l].fval)
				b = DRrectFloatSide(rectp,h) - rl[l].fval;
			else if (DRrectFloatSide(rectp,h) > rh[l].fval)
				b = DRrectFloatSide(rectp,l) - rh[l].fval;
			else	b = 0.0;
                        break;
                }
		sum += pow( (double) b, (double) 2 );
        }

        if (sum == 0.0) {         /* rectangle overlaps query rectp */

                if ((sum = containInQuery(rectp, rl,rh)) == 0.0)
                        return assignNegDist(rectp, rl,rh);
                return sum;
        }
        else
        return sum;
        /* return pow((double) sum, 1/(double) DRdim(rrfd) ); */


        /* domain error occurs if first argument is zero and second argu <=0 */
        /* or when first argu < 0 and second argu is not an integer */
}


/*----------------------------------------------------------------------
| cmp()
|       function to compare 2 rectangles for distance to query point.
|
|       return 0 if both the same
|               -1 if "v" is "nearer" than "w"
|               1 if "v" is farther than "w"
+----------------------------------------------------------------------*/
int
cmp(IRect * v, IRect * w)
{
        double          dv, dw;

        dv = minDist(qrect,(*v).rectl,(*v).recth);
        dw = minDist(qrect,(*w).rectl,(*w).recth);
        if (dv < dw)
                return -1;
        else if (dv == dw)
                return 0;

        return 1;
}


/*----------------------------------------------------------------------
| getKdist()
|       returns the longest distance for any rectangle in nnarr[] to
|       the query point. If there is less than K element , return MAXFLOAT
+----------------------------------------------------------------------*/

double
getKdist(int knn)
{

        return (lastNNinArr < (knn-1)) ? MAXFLOAT : nnarr[lastNNinArr].dist;
}


/*----------------------------------------------------------------------
| insertnnArr()
|       Insert rect "r" into the proper position within  `array' nnarr.
|       Rect within nnarr are sorted based on distance to query point.
|       Update the size of nnarr.
+----------------------------------------------------------------------*/

void
insertnnArr(intfloat *rl, intfloat *rh, int c, int knn)
{
        int             i, j, k;
        double          d;

        d = minDist(qrect, rl, rh);

        i = 0;
        while ((i <= lastNNinArr) && (d >= nnarr[i].dist))
                i++;
        if ((i > lastNNinArr)
            && (lastNNinArr < (knn-1))) {
                lastNNinArr++;

                nnarr[i].childno = c;
                nnarr[i].dist = d;
                switch (DRcoordType(rrfd))
                {
                        case DR_INT:
			for (k = 0; k < DRdim(rrfd); k++) {
				nnarr[i].rectl[k].ival = rl[k].ival;
				nnarr[i].recth[k].ival = rh[k].ival;
			}
			break;
                        case DR_FLOAT:
			for (k = 0; k < DRdim(rrfd); k++) {
				nnarr[i].rectl[k].fval = rl[k].fval;
				nnarr[i].recth[k].fval = rh[k].fval;
			}
			break;
		}
        } else {
                if (lastNNinArr < (knn-1))
                        lastNNinArr++;
                j = lastNNinArr;
                while (j != i) {
			nnarr[j].childno = nnarr[j-1].childno;
                        nnarr[j].dist = nnarr[j-1].dist;
                	switch (DRcoordType(rrfd))
			{
       	                	case DR_INT:
				for (k = 0; k < DRdim(rrfd); k++) {
					nnarr[j].rectl[k].ival = nnarr[j-1].rectl[k].ival;
					nnarr[j].recth[k].ival = nnarr[j-1].recth[k].ival;
				}
				break;

				case DR_FLOAT:
				for (k = 0; k < DRdim(rrfd); k++) {
					nnarr[j].rectl[k].fval = nnarr[j-1].rectl[k].fval;
					nnarr[j].recth[k].fval = nnarr[j-1].recth[k].fval;
				}
				break;
			}
                        j--;
                }
                nnarr[i].childno = c;
                nnarr[i].dist = d;
                switch (DRcoordType(rrfd))
                {
                        case DR_INT:
			for (k = 0; k < DRdim(rrfd); k++) {
				nnarr[i].rectl[k].ival = rl[k].ival;
				nnarr[i].recth[k].ival = rh[k].ival;
			}
			break;
                        case DR_FLOAT:
			for (k = 0; k < DRdim(rrfd); k++) {
				nnarr[i].rectl[k].fval = rl[k].fval;
				nnarr[i].recth[k].fval = rh[k].fval;
			}
			break;
		}
        }
}


/*----------------------------------------------------------------------
| findNN()
|       first pass of the algorithm . Choose the most promising node to visit
|       that may potentially contain the nearest neigbhor(s).
|       Store the approximate nearest neighbor(s) in an array.
|
+----------------------------------------------------------------------*/

void
findNN(int knn, int page)
{
	char	*pagebuf;		/* pointer to page buffer of node */
	int	level;			/* tree level */
	IRect	*d;			/* structure to store list of merits */
	int	i,j;
	int	count=0;		/* number of children at next level */
	int last;			/* last ABR element to visit after pruning */
	int	newpage;		/* page number of child MBR */
	double	minmax;			/* MinMaxDist(P,M) */
	double	mind;			/* MinDist (P,M) */
	DRrect	*obj;			/* leaf node */
	intfloat objl[DRdim(rrfd)],
		objh[DRdim(rrfd)];
	int	childno;		/* id number of leaf child */


	if (PS_GetThisPage(DRftab[rrfd].pfd, page, &pagebuf) != 0)
               	printf("GetThisPage didn't work!\n");

	level = DRnodeLevel(rrfd, pagebuf);
	if (level) {		/* if non-leaf node */
		d = (IRect *) malloc (sizeof(IRect)*DRnodeCount(rrfd,pagebuf));
		for (i = 0; i < DRnodeCount(rrfd,pagebuf); i++) {
		d[i].rectl = (intfloat *) malloc (sizeof(intfloat)*DRdim(rrfd));
		d[i].recth = (intfloat *) malloc (sizeof(intfloat)*DRdim(rrfd));
		}
		for (i = DRnodeFirstEntry(rrfd, pagebuf);
                       i >= 0 && i < DRnodeCard(rrfd);
                       i = DRnodeNextEntry(rrfd, i, pagebuf)) {
			rectToArr(DRnodeRect(rrfd,i,pagebuf),d[count].rectl,d[count].recth);
			d[count].page = DRnodeChild(rrfd,i,pagebuf);
			count++;
		}
		/* sort the possible routes according to merit */
		switch (DRcoordType(rrfd))
		{
			case DR_INT:
			qsort(d,count,sizeof(IRect),(int (*) (void *, void *)) cmp);
			break;
			case DR_FLOAT:
			qsort(d,count,sizeof(IRect),(float (*) (void *, void *)) cmp);
			break;
		}

		/* downward pruning */
		last = count;

		for (i = 0; i < last; i++) {
			if (lastNNinArr < 0) minmax = MAXFLOAT;
			else minmax = minMaxDist(qrect,
					nnarr[lastNNinArr].rectl,
					nnarr[lastNNinArr].recth);
			if (minDist(qrect,d[i].rectl,d[i].recth) > minmax) {
				last = i;
				break;
			}
			if ((lastNNinArr == knn-1) &&
			(nnarr[lastNNinArr-1].dist <=
			minMaxDist(qrect,d[i].rectl,d[i].recth)) &&
			(nnarr[lastNNinArr].dist >
			minMaxDist(qrect,d[i].rectl,d[i].recth)))
			{
				int dummy;
				printf("heuristic #2 used\n");
				scanf("%d",&dummy);
				nnarr[lastNNinArr].dist =
					minMaxDist(qrect,d[i].rectl,d[i].recth);
			}

			/* recursively visit child nodes */
			findNN(knn,d[i].page);

			/* upward pruning */
			if (lastNNinArr < 0) mind = MAXFLOAT;
			else mind = nnarr[lastNNinArr].dist;
			if (minDist(qrect,d[i].rectl,d[i].recth) > mind)
			{
				int dummy;
				last = i;
				printf("heuristic #3 used\n");
				scanf("%d",&dummy);
				break;
			}
		}
		free(d);

	} else {	/* leaf node*/
		for (i = DRnodeFirstEntry(rrfd, pagebuf);
                     i >= 0 && i < DRnodeCard(rrfd);
                     i = DRnodeNextEntry(rrfd, i, pagebuf)) {
			childno = DRnodeChild(rrfd,i,pagebuf);
			obj = DRnodeRect(rrfd,i,pagebuf);
			rectToArr(obj,objl,objh);
			if (minDist(qrect,objl,objh) < getKdist(knn))
				insertnnArr(objl,objh,childno,knn);
		}
	}

	if (PS_UnfixPage(DRftab[rrfd].pfd, page, FALSE) != PSE_OK)
		printf("Couldn't unfix!\n");
}


/*----------------------------------------------------------------------
| DR_NearNeighbor()
|       Prompts user for k, the number of nearest neighbors to search
|	for, then calls findNN to perform a branch-and-bound algorithm
|	to determine the k-nearest neighbors, which returns them in an
|	array.  DR_NearNeighbor then prints the results to the screen.
+----------------------------------------------------------------------*/
int DR_NearNeighbor(int rfd, DRrect *qr)
/* rrfd is the rfd */
/* qrect is the query rectangle */
{
	int knn;	/* number of nearest neighbors to look for */
	int i,j;
	char *pagebuf; int child;

	rrfd = rfd;
	qrect = qr;	/* transfer parameters to static variables */
	lastNNinArr = -1;

	printf("Enter the number of nearest neighbors: ");
	scanf("%d", &knn);

	nnarr = (Nntype *) malloc (sizeof(Nntype)*knn);
	for (i = 0; i < knn; i++) {
		nnarr[i].rectl = (intfloat *) malloc (sizeof(intfloat)*DRdim(rrfd));
		nnarr[i].recth = (intfloat *) malloc (sizeof(intfloat)*DRdim(rrfd));
	}

	findNN(knn,DRrootPage(rrfd));

	for (i = 0; i <= lastNNinArr; i++) {
		printf("Neighbor #%d:\n",i+1);
		printf("\tchildno=%d\n",nnarr[i].childno);
		printf("\tchildcov=\n");
		switch (DRcoordType(rrfd))
		{
			case DR_INT:
			for (j = 0; j < DRdim(rrfd); j++)
				printf("\t%d\t%d\n",nnarr[i].rectl[j].ival,nnarr[i].recth[j].ival);
			break;
			case DR_FLOAT:
			for (j = 0; j < DRdim(rrfd); j++)
				printf("\t%f\t%f\n",nnarr[i].rectl[j].fval,nnarr[i].recth[j].fval);
			break;
		}
		printf("\tdist=%f\n",nnarr[i].dist);
		printf("\n");
	}
	
	free(nnarr);

	return (lastNNinArr+1);
}
