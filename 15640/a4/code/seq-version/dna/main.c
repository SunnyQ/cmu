#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>

#include "time.h"
#include "strand.h"
#include "kmeans.h"

static void usage(char *argv0) {
  char *usage=
    "Usage: %s [args]\n"
    "\t-f file_path        : file of input strands\n"
    "\t-l len_strands      : length of each strand\n"
    "\t-p num_strands      : number of strands\n"
    "\t-c num_centroids    : number of centroids\n"
    "\t-i initial_centroids: initial centroids (default randomly choosen from input strands)\n";
  fprintf(stderr, usage, argv0);
  exit(-1);
}

int main(int argc, char **argv) {
  char *strandsFilePath = NULL;
  char *centroidsFilePath = NULL;
  Strand *strands = NULL;
  Strand *centroids = NULL;
  char *strandsMem = NULL;
  char *centroidsMem = NULL;
  int numStrands = -1;
  int numCentroids = -1;
  int lenStrand = -1;
  int opt, i, numItrs;
  double currTime, ioWallTime, kmeansWallTime;

  while ((opt = getopt(argc, argv, "f:p:c:l:i:")) != -1) {
    switch (opt) {
      case 'f': 
        strandsFilePath = optarg;
        break;
      case 'i': 
        centroidsFilePath = optarg;
        break;
      case 'c': 
        numCentroids = atoi(optarg);
        break;
      case 'p': 
        numStrands = atoi(optarg);
        break;
      case 'l': 
        lenStrand = atof(optarg);
        break;
      case '?': 
        usage(argv[0]);
        break;
      default: 
        usage(argv[0]);
        break;
    }
  }

  if (strandsFilePath == NULL || 
      numStrands <= 0 ||
      numCentroids <= 0 || 
      lenStrand < 0) {
    usage(argv[0]);
  }

  ioWallTime = get_curr_time();

  init_memory_space(&strands, &strandsMem, numStrands, lenStrand);

  int numStrandsLoaded = load_strands(strands, strandsFilePath, 
      numStrands, lenStrand);
  if (numStrandsLoaded != numStrands) {
    fprintf(stderr, "Error: could only load %d strands from sources\n",
        numStrandsLoaded);
    exit(-1);
  }

  init_memory_space(&centroids, &centroidsMem, numCentroids, lenStrand);

  if (centroidsFilePath != NULL) {  // load centroids from given file
    int numCentroidsLoaded = load_strands(centroids, centroidsFilePath, 
        numCentroids, lenStrand);
    if (numCentroidsLoaded != numCentroids) {
      fprintf(stderr, "Error: could only load %d centroids from sources\n", 
          numCentroidsLoaded);
      exit(-1);
    }
  } else {  // randomly choose centroids
    random_choose_centroids(strands, centroids, 
        numStrands, numCentroids, lenStrand);
  }

  currTime = get_curr_time();
  ioWallTime = currTime - ioWallTime;
  
  kmeansWallTime = get_curr_time();

  numItrs = do_kmeans(strands, centroids, numStrands, numCentroids, lenStrand);

  currTime = get_curr_time();
  kmeansWallTime = currTime - kmeansWallTime;

  printf("========================================\n");
  printf("Number of Iterations: %d\n", numItrs);
  printf("========================================\n");
  printf("K-Means Centroids: \n");
  for (i = 0; i < numCentroids; i++) {
    print_strand(centroids[i], lenStrand);
  }
  printf("========================================\n");
  printf("IO wall time: \t\t%10.4f sec\n", ioWallTime);
  printf("KMeans wall time: \t%10.4f sec\n", kmeansWallTime);
  printf("Total wall time: \t%10.4f sec\n", (kmeansWallTime + ioWallTime));
  printf("========================================\n");

  free(centroidsMem);
  free(centroids);
  free(strandsMem);
  free(strands);
  return 0;
}
