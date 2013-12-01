/* $Revision: 1.4 $ */
/* $Author: christos $ */
/* $Id: vector.h,v 1.4 2011/09/11 07:33:48 christos Exp christos $ */

#ifndef __vector_h
  #define __vector_h
  #include <stdlib.h>
  #include <assert.h>
  #include "dfn.h"
  
  
  /* 1-d vector, with size */
  typedef struct Vector { 
     int len;
     NUMBER *vec;
  } VECTOR;
  
  
  
  VECTOR *vecalloc(int n);
  
  void vecfree( VECTOR *);
  
  NUMBER vecdist2( VECTOR *, VECTOR *); /* squared Eucl. distance */
  
  void vecput( VECTOR *pvec, int pos, NUMBER val);
  
  NUMBER vecget( VECTOR *pvec, int pos );
  
  VECTOR *veccopy( VECTOR *p);
  
  void vecprint( VECTOR *p);

#endif
