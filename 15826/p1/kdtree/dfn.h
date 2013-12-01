/* $Revision: 1.4 $ */
/* $Author: christos $ */
/* $Id: dfn.h,v 1.4 2011/09/11 07:33:48 christos Exp christos $ */

#ifndef HUGE
    #define HUGE 10000000
#endif


#ifndef _dfn_h
    #define _dfn_h

   #ifndef NULL
   #define NULL 0
   #endif

   #ifndef __NUMBER
      typedef double NUMBER;
   #endif
   #ifndef __BOOLEAN
      typedef int BOOLEAN;
      #define TRUE 1
      #define FALSE 0
   #endif

    #define MAXLEN 200
    #define EOS '\0'
    #define MYHUGE HUGE

    #include <stdio.h>
    #include <string.h>
    #include <assert.h>
    #include <math.h>
#endif

