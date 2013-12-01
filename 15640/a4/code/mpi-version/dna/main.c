#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "strand.h"
#include "time.h"
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
  int numStrands = -1;
  int numCentroids = -1;
  int lenStrand = -1;
  int opt, i, numItrs;
  double currTime, ioWallTime, kmeansWallTime;

  int numProcs, rank, namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Get_processor_name(processor_name, &namelen);

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
    MPI_Finalize();
    exit(-1);
  }

  ioWallTime = get_curr_time();

  MPI_Barrier(MPI_COMM_WORLD);

  Strand *strands = NULL;
  Strand *centroids = NULL;
  char *strandsMem = NULL;
  char *centroidsMem = NULL;

  if (rank == 0) {

    // load strands 
    init_memory_space(&strands, &strandsMem, numStrands, lenStrand);

    int numStrandsLoaded = load_strands(strands, strandsFilePath, 
        numStrands, lenStrand);
    if (numStrandsLoaded != numStrands) {
      fprintf(stderr, "Error: could only load %d strands from sources\n", 
          numStrandsLoaded);
      MPI_Finalize();
      exit(-1);
    }

    // load or generate initial centroids
    init_memory_space(&centroids, &centroidsMem, numCentroids, lenStrand);
    
    if (centroidsFilePath != NULL) {  // load centroids from given file
      int numCentroidsLoaded = load_strands(centroids, centroidsFilePath, 
          numCentroids, lenStrand);
      if (numCentroidsLoaded != numCentroids) {
        fprintf(stderr, "Error: could only load %d centroids from sources\n",
            numCentroidsLoaded);
        MPI_Finalize();
        exit(-1);
      }
    } else {  // randomly choose centroids
      random_choose_centroids(strands, centroids, 
        numStrands, numCentroids, lenStrand);
    }

    // distributed strands 
    numStrands = distribute_strands(strands, numStrands, lenStrand,
        numProcs, MPI_COMM_WORLD);

    // destroy distributed strands
    strandsMem = realloc(strandsMem, numStrands * lenStrand * sizeof(char));
    assert(strandsMem != NULL);
    strands = realloc(strands, numStrands * sizeof(Strand));
    assert(strands != NULL);

  } else {

    numStrands = get_portion_size(numStrands, numProcs, rank);

    // receive strands 
    init_memory_space(&strands, &strandsMem, numStrands, lenStrand);

    int numStrandsReceived = receive_strands(strands, numStrands, lenStrand,
        rank, MPI_COMM_WORLD);

    if (numStrandsReceived != numStrands) {
      fprintf(stderr, "Error: could only receive %d strands from root\n", 
          numStrandsReceived);
      MPI_Finalize();
      exit(-1);
    }

    // load or generate initial centroids
    init_memory_space(&centroids, &centroidsMem, numCentroids, lenStrand);
  }

  currTime = get_curr_time();
  ioWallTime = currTime - ioWallTime;

  kmeansWallTime = get_curr_time();

  // broadcast centroids
  MPI_Bcast(centroidsMem, numCentroids * lenStrand, MPI_CHAR, 0, MPI_COMM_WORLD);

  numItrs = do_kmeans(strands, centroids, 
      numStrands, numCentroids, lenStrand,
      rank, MPI_COMM_WORLD);

  currTime = get_curr_time();
  kmeansWallTime = currTime - kmeansWallTime;

  if (rank == 0) {
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
  }

  free(centroidsMem);
  free(centroids);
  free(strandsMem);
  free(strands);
  MPI_Finalize();
  return 0;
}
