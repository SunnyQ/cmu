/* $Revision: 1.4 $ */
/* $Author: christos $ */
/* $Id: vector.c,v 1.4 2011/09/11 07:33:48 christos Exp christos $ */

#include "vector.h"

VECTOR *vecalloc( int n){
    VECTOR *pvec;
    int i;

    assert( n > 0 );
    pvec = (VECTOR *) malloc( sizeof(VECTOR) );
    assert( pvec != NULL );
    pvec-> vec = (NUMBER *) malloc ( sizeof(NUMBER) * n );
    assert( (pvec->vec) != NULL );
    pvec->len = n;

    /* initialize vector to zeros - 2/7/97 */
    for(i=0; i<n; i++){
	(pvec->vec)[i] = (NUMBER) 0.0;
    }

    return(pvec);
}

void vecfree( VECTOR *pvec){
    free( (char *) pvec->vec);
    pvec->len =0;
    free( (char *) pvec);
}

NUMBER vecdist2( VECTOR *p1, VECTOR *p2){
    NUMBER res2; /* squared result */
    NUMBER v1, v2;
    int pos, length;

    assert( (p1->len) == (p2->len) );
    length = p1->len;
    res2=0.0;
    for(pos=0; pos< length; pos++){
	v1 = (p1->vec)[pos];
	v2 = (p2->vec)[pos];
	res2 += (v1-v2)*(v1-v2);
    }

    return( res2);
}

void vecput( VECTOR *pvec, int pos, NUMBER val){
    assert( pos >=0) ;
    assert( pos < (pvec->len)  );
    (pvec->vec)[pos] =  val;
}

NUMBER vecget( VECTOR *pvec, int pos){
    assert( pos >=0) ;
    assert( pos < (pvec->len)  );
    return( (pvec->vec)[pos] );

}

VECTOR *veccopy( VECTOR *psource){
    VECTOR *ptarget;
    int pos, length;

    assert( (psource->len) >0 );
    length = psource->len;
    ptarget = vecalloc(length);
    /* printf( "%d\n" , ptarget->len); */
    for(pos=0; pos<length; pos++){
	(ptarget->vec)[pos] = (psource->vec)[pos];
    }
    return( ptarget);
}

void vecprint( VECTOR *p){
    int i, length;

    /*  assert( (p->len) > 0 ); */
    length = p->len;

    for(i=0; i< length; i++){
	printf("%g\t", vecget(p, i));
    }
    printf("\n");
}
