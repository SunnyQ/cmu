#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>

#include "time.h"
#include "point.h"
#include "kmeans.h"

static void usage(char *argv0) {
  char *usage=
    "Usage: %s [args]\n"
    "\t-f file_path        : file of input points\n"
    "\t-p num_points       : number of points\n"
    "\t-c num_centroids    : number of centroids\n"
    "\t-t threshold        : threshold value (default 0.001)\n"
    "\t-i initial_centroids: initial centroids (default randomly choosen from input points)\n";
  fprintf(stderr, usage, argv0);
  exit(-1);
}

int main(int argc, char **argv) {
  char *pointsFilePath = NULL;
  char *centroidsFilePath = NULL;
  int numPoints = -1;
  int numCentroids = -1;
  float threshold = 0.001;
  int opt, i, numItrs;
  double currTime, ioWallTime, kmeansWallTime;

  int numProcs, rank, namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Get_processor_name(processor_name, &namelen);

  while ((opt = getopt(argc, argv, "f:p:c:t:i:")) != -1) {
    switch (opt) {
      case 'f': 
        pointsFilePath = optarg;
        break;
      case 'i': 
        centroidsFilePath = optarg;
        break;
      case 'c': 
        numCentroids = atoi(optarg);
        break;
      case 'p': 
        numPoints = atoi(optarg);
        break;
      case 't': 
        threshold = atof(optarg);
        break;
      case '?': 
        usage(argv[0]);
        break;
      default: 
        usage(argv[0]);
        break;
    }
  }

  if (pointsFilePath == NULL || 
      numPoints <= 1 ||
      numCentroids <= 1 || 
      threshold < 0) {
    usage(argv[0]);
    MPI_Finalize();
    exit(-1);
  }

  ioWallTime = get_curr_time();

  // init mpi struct type for points
  MPI_Datatype pointDataType;
  initMPIType(&pointDataType);

  MPI_Barrier(MPI_COMM_WORLD);

  Point *points = NULL;
  Point *centroids = NULL;


  if (rank == 0) {

    // load points
    points = malloc(numPoints * sizeof(Point));
    assert(points != NULL);

    int numPointsLoaded = load_points(points, pointsFilePath, numPoints);
    if (numPointsLoaded != numPoints) {
      fprintf(stderr, "Error: could only load %d points from sources\n", 
          numPointsLoaded);
      MPI_Finalize();
      exit(-1);
    }

    // load or generate initial centroids
    centroids = malloc(numCentroids * sizeof(Point));
    assert(centroids != NULL);

    if (centroidsFilePath != NULL) {  // load centroids from given file
      int numCentroidsLoaded = load_points(centroids, centroidsFilePath, numCentroids);
      if (numCentroidsLoaded != numCentroids) {
        fprintf(stderr, "Error: could only load %d centroids from sources\n", numCentroidsLoaded);
        MPI_Finalize();
        exit(-1);
      }
    } else {  // randomly choose centroids
      random_choose_centroids(points, centroids, numPoints, numCentroids);
    }

    // distributed points
    numPoints = distribute_points(points, numPoints, numProcs, 
        pointDataType, MPI_COMM_WORLD);

    points = realloc(points, numPoints * sizeof(Point));
    assert(points != NULL);


  } else {

    numPoints = get_portion_size(numPoints, numProcs, rank);
    points = malloc(numPoints * sizeof(Point));

    int numPointsReceived = receive_points(points, numPoints, rank, 
        pointDataType, MPI_COMM_WORLD);

    if (numPointsReceived != numPoints) {
      fprintf(stderr, "Error: could only receive %d points from root\n", 
          numPointsReceived);
      MPI_Finalize();
      exit(-1);
    }

    // load or generate initial centroids
    centroids = malloc(numCentroids * sizeof(Point));
    assert(centroids != NULL);
  }


  currTime = get_curr_time();
  ioWallTime = currTime - ioWallTime;

  kmeansWallTime = get_curr_time();

  // broadcast centroids
  MPI_Bcast(centroids, numCentroids, pointDataType, 0, MPI_COMM_WORLD);

  numItrs = do_kmeans(points, centroids, numPoints, numCentroids, threshold,
      rank, pointDataType, MPI_COMM_WORLD);

  currTime = get_curr_time();
  kmeansWallTime = currTime - kmeansWallTime;

  if (rank == 0) {
    printf("========================================\n");
    printf("Number of Iterations: %d\n", numItrs);
    printf("========================================\n");
    printf("K-Means Centroids: \n");
    for (i = 0; i < numCentroids; i++) {
      print_point(centroids[i]);
    }
    printf("========================================\n");
    printf("IO wall time: \t\t%10.4f sec\n", ioWallTime);
    printf("KMeans wall time: \t%10.4f sec\n", kmeansWallTime);
    printf("Total wall time: \t%10.4f sec\n", (kmeansWallTime + ioWallTime));
    printf("========================================\n\n");
  }

  free(centroids);
  free(points);
  MPI_Finalize();
  return 0;
}
