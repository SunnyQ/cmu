/* $Revision: 1.4 $ */
/* $Author: christos $ */
/* $Id: kdtree.h,v 1.4 2011/09/11 07:33:48 christos Exp christos $ */

#ifndef __kdtree_h
  #define __kdtree_h
  #include <stdlib.h>
  #include <assert.h>
  #include "vector.h"
  #include "dfn.h"
  
  
  /* tree node */
  typedef struct TreeNode { 
     struct TreeNode *left;
     struct TreeNode *right;
     VECTOR *pvec;
  } TREENODE;
  
  /* insert a vector into the tree, and return the new root */
  TREENODE *insert(TREENODE *subroot, VECTOR *vp);

  /* recursive insert, using the level info */
  TREENODE *rinsert(TREENODE *subroot, VECTOR *vp, int level);

  TREENODE *talloc();
  void tfree();

  /* prints the whole tree - for debugging purposes */
  void tprint(TREENODE *subroot);
  void rtprint(TREENODE *subroot, int level);

  /* returns the 'count' nearest neighbors in the subtree */
  void nnsearch(TREENODE *subroot, VECTOR *vp, int count);
  VECTOR *rnnsearch(TREENODE *subroot, VECTOR *vp, VECTOR *best, int level);

  void rangesearch(TREENODE *subroot, VECTOR *vpLow, VECTOR *vpHigh);
  void rrangesearch(TREENODE *subroot, VECTOR *vpLow, VECTOR *vpHigh, int level);

   BOOLEAN contains( VECTOR *vpLow, VECTOR *vpHigh, VECTOR *vp);

   NUMBER myvecdist2( VECTOR *vp1, VECTOR *vp2);
   VECTOR *rminbox(TREENODE *subroot, VECTOR *vp, int high);

#endif
