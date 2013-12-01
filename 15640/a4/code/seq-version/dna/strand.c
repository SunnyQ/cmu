#include <stdio.h>
#include <string.h>

#include "strand.h"

static int min(int a, int b ){
  return a < b ? a : b;
}

int edit_dist(Strand str1, Strand str2, int strLen) {
  int dist [strLen+1][strLen+1], i, j;

  for (i = 0; i <= strLen; i++) {
    dist[i][0] = i;
    dist[0][i] = i;
  }

  for (i = 1; i <= strLen; i++) {
    for (j = 1; j <= strLen; j++) {
      if (str1[i-1] == str2[j-1]) {
        dist[i][j] = dist[i-1][j-1];
      } else {
        dist[i][j] = min(dist[i-1][j-1], min(dist[i-1][j], dist[i][j-1])) + 1;
      }
    }
  }

  return dist[strLen][strLen];
}

void print_strand(Strand str, int lenStrand) {
  int i;
  for (i = 0; i < lenStrand; i++)
    printf("%c", str[i]);
  printf("\n");
}
