/* $Revision: 1.2 $ */
/* $Author: christos $ */
/* $Id: main.c,v 1.2 2011/09/11 07:33:48 christos Exp christos $ */


#include "dfn.h"
#include "vector.h"
#include "kdtree.h"

#define CMDLINE

#ifdef CMDLINE
   #define ERRACT	fprintf(stderr, \
   "usage: %s [-d numberOfDimensions]\n", pgname) 
#endif

void checkword(char *s);

main(argc, argv)
int argc;
char *argv[];
{
    char *pgname;
    char inp[MAXLEN];
    char cmd[MAXLEN];
    VECTOR *inputVectorPtr;
    VECTOR *vpLow;
    VECTOR *vpHigh;
    int numdims;
    int i;
    NUMBER val;
    TREENODE *root;
    int count;


    void prhelp();

    count = 0; /* number of nodes touched - not used, yet */

    numdims = 4; /* to be changed, from the command-line interface */



    vpLow = vecalloc(numdims);
    vpHigh = vecalloc(numdims);
    root = NULL;

    /* initialize */
    pgname = argv[0];

#ifdef CMDLINE
    while( argc > 1 && argv[1][0] == '-' ) {
	switch( argv[1][1]) {

	    case 'd':	/* set number of dimensions */
		if( argc>2){
		    checkword(argv[2]);
		    strcpy(inp, argv[2]);
		    numdims = atoi(inp);
		    printf("num. dimensions = %d\n", numdims);
		    vecfree( vpLow);
		    vecfree( vpHigh);
		    vpLow = vecalloc(numdims);
		    vpHigh = vecalloc(numdims);
		    argc--; argv++;
		} else {
		    ERRACT;
		    exit(1);
		}
		break;


	    #ifdef JUNK
	    case 's':	/* set fname for file to show */
		if( argc>2){
		    checkword(argv[2]);
		    strcpy(fname,  argv[2]);
		    argc--; argv++;
		} else {
		    ERRACT;
		    exit(1);
		}
		break;
	    #endif

	    default:
		fprintf(stderr, "%s: unknown arg %s\n", pgname, argv[1]);
		exit(1);
	}
	argc--;
	argv++;
    }

    if( argc != 1){ 
	ERRACT;
	exit(1);
    }
#endif

    inputVectorPtr = vecalloc(numdims);

    prhelp();
    printf("kdtree> ");
    /* printf("kdtree1> "); */
    gets(inp); checkword(inp);
    while( strlen(inp)==0){ 
        /* printf("kdtree2> "); */
	gets(inp); checkword(inp);
    }

    while( inp[0] != 'x' ){
	#ifdef DEBUG
	printf("********Testing: input line is: %s\n", inp);
	#endif

	switch( inp[0] ){
	   case 'i':		 /* insert a vector */
	       printf("inserting ...\n");

	       for(i=0;i<numdims;i++){
		   printf("%d-th attr. value= ", i);
		   scanf("%lf", &val);
		   #ifdef DEBUG
		   printf("%f \n", val);
		   #endif
		   vecput( inputVectorPtr, i, val);
	       }
	       #ifdef DEBUG
		printf("\n");
	       #endif
	       printf("   inserting point: ");
	       vecprint( inputVectorPtr);
	       root = insert(root, inputVectorPtr);

	   break;

	   case 'p':		 /* print the whole tree */
	       tprint(root);
	   break;

	   case 'r':		 /* range search */
	       printf("range searching ...\n");

	       for(i=0;i<numdims;i++){
		   printf("%d-th attr. low value= ", i);
		   scanf("%lf", &val);
		   #ifdef DEBUG
		   printf("%f \n", val);
		   #endif
		   vecput( vpLow, i, val);
	       }
	       #ifdef DEBUG
		printf("\n");
	       #endif

	       for(i=0;i<numdims;i++){
		   printf("%d-th attr. high value= ", i);
		   scanf("%lf", &val);
		   #ifdef DEBUG
		   printf("%f ", val);
		   #endif
		   vecput( vpHigh, i, val);
	       }
	       #ifdef DEBUG
		printf("\n");
	       #endif

	       printf("   searching - low values: ");
	       vecprint( vpLow);
	       printf("   searching - high values: ");
	       vecprint( vpHigh);
	       rangesearch(root, vpLow, vpHigh);
	   break;


	   case 'n':		 /* nn search for a similar vector */
	       printf("nn searching ...\n");
	       /* printf("# of neighbors= ");
	       scanf("%d", &count);
	       */
	       for(i=0;i<numdims;i++){
		   printf("%d-th attr. value= ", i);
		   scanf("%lf", &val);
		   #ifdef DEBUG
		   printf("%f \n", val);
		   #endif
		   vecput( inputVectorPtr, i, val);
	       }
	       #ifdef DEBUG
		printf("\t\t for %d nn\n", count);
	       #endif
	       printf("   nn searching - query point: ");
	       vecprint( inputVectorPtr);
	       nnsearch(root, inputVectorPtr, count);
	   break;
	   
	   case 'b':		 /* Minimum bounding box. Insert HW1 code here */
               printf("Calculating minimum bounding box ...\n");
               vpLow = veccopy(root->pvec);
               vpHigh = veccopy(root->pvec);

               printf("The low end is: ");
               vpLow = rminbox(root, vpLow, 0);
               vecprint(vpLow);

               printf("The high end is: ");
               vpHigh = rminbox(root, vpHigh, 1);
               vecprint(vpHigh);
	   break;

	   case 'x':		 /* exit */
	   break;

	   case 'h':		 /* print help file */
	       prhelp();
	   break;

	   default:
	       printf("illegal command: |%s| \n", inp);
	}

    	prhelp();
	printf("kdtree> ");
	/* printf("kdtree3> "); */
    	gets(inp); checkword(inp);
        while( strlen(inp)==0){ 
	    /* printf("kdtree4> "); */
	    gets(inp); checkword(inp);
        }
    }

}

/********************************************/
/** check whether the arg. is a valid word **/
/********************************************/
void checkword(char *s){
   assert(strlen(s) < MAXLEN);
}

/********************************************/
/** print the help message                 **/
/********************************************/
void prhelp(){
    printf("\t i \t\t for insertion\n");
    printf("\t n \t\t for nn search \n");
    printf("\t r \t\t for range search \n");
    printf("\t p \t\t to print the tree\n");
    printf("\t b \t\t to print out the minimum bounding box\n");
    printf("\t x \t\t to exit\n");
    printf("\t h \t\t to print this help message\n");
}
