#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
using namespace std;

#define BUF_SIZE 128

char buf[BUF_SIZE];

int run_dtw(int n, int m, FILE *infp1, FILE *infp2) {
  int i, j, k;
  long **dtw;
  long *q;
  long *c;
  
  dtw = (long **)malloc(n * sizeof(long *));
  q = (long *)malloc(n * sizeof(long));
  c = (long *)malloc(m * sizeof(long));
  for (i = 0; i < n; i++) {
    dtw[i] = (long *)malloc(m * sizeof(long));
    q[i] = atoi(fgets(buf, BUF_SIZE, infp1));
  }
	
  for (j = 0; j < m; j++) 
    c[j] = atoi(fgets(buf, BUF_SIZE, infp2));
	
  for (j = 0; j < m; j++)
    for (k = 0; k <= j; k++) 
      dtw[0][j] += abs(q[0] - c[k]);
	  
  for (i = 0; i < n; i++)
    for (k = 0; k <= i; k++)
      dtw[i][0] += abs(q[k] - c[0]);
  
  for (i = 1; i < n; i ++)
    for (j = 1; j < m; j++)
      dtw[i][j] = abs(q[i] - c[j]) + min(min(dtw[i-1][j-1], dtw[i][j-1]), dtw[i-1][j]);

  return dtw[n-1][m-1];
}

int main(int nargs, char* args[]) {
  FILE *infp1;
  FILE *infp2;
  FILE *outfp;
  int n, m, dtw;

  if (nargs != 3) {
    fprintf(stderr, "Usage: %s <file 1> <file 2>\n", args[0]);
    return -1;
  }
  
  if (!(infp1 = fopen(args[1], "r")))
  {
    fprintf(stderr, "No such file: %s\n", args[1]);
    return EXIT_FAILURE;
  }
  
  if (!(infp2 = fopen(args[2], "r")))
  {
    fprintf(stderr, "No such file: %s\n", args[2]);
    return EXIT_FAILURE;
  }
  
  printf("%s %s ", args[1], args[2]);
  fflush(stdout);
  
  outfp = fopen("hw2.q1.output.txt", "a");

  n = atoi(fgets(buf, BUF_SIZE, infp1));
  m = atoi(fgets(buf, BUF_SIZE, infp2));
  
  dtw = run_dtw(n, m, infp1, infp2);
  
  printf("%d\n", dtw);
  fprintf(outfp, "%s %s %d\n", args[1], args[2], dtw);
  
  fclose(infp1);
  fclose(infp2);
  fclose(outfp);
  return 0;
}
