#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <time.h>

#include "kmeans.h"
#include "point.h"

#define BUFFER_SIZE 512

int load_points(Point* points, char *filePath, int numPoints) {
  FILE *file;
  char buf[BUFFER_SIZE];
  int numPointsLoaded = 0;
  
  file = fopen(filePath, "r");
  if (file == NULL) {
    return -1;
  }

  while (fgets(buf, BUFFER_SIZE, file) != NULL && numPointsLoaded < numPoints) {
    sscanf(buf, "%f,%f\n", &points[numPointsLoaded].x, &points[numPointsLoaded].y);
    numPointsLoaded++;
  }

  fclose(file);
  return numPointsLoaded;
}

int get_portion_size(int numPoints, int numProcs, int rank) {
  int divide = numPoints / numProcs;
  int remain = numPoints % numProcs;

  return rank < remain ? divide + 1 : divide;
}

int distribute_points(Point* points, int numPoints, int numProcs,
    MPI_Datatype pointDataType, MPI_Comm comm) {

  int numPointsRemained = get_portion_size(numPoints, numProcs, 0);
  int ptr = numPointsRemained;
  int numPointsToDistribute, i;

  for ( i = 1; i < numProcs; i++) {
    numPointsToDistribute = get_portion_size(numPoints, numProcs, i);
    MPI_Send(&points[ptr], numPointsToDistribute, pointDataType, i, i, comm);
    ptr += numPointsToDistribute;
  }

  return numPointsRemained;
}

int receive_points(Point* points, int numPoints, int rank,
    MPI_Datatype pointDataType, MPI_Comm comm) {

  int numPointsReceived;
  MPI_Status status;
  MPI_Recv(points, numPoints, pointDataType, 0, rank, comm, &status);
  MPI_Get_count(&status, pointDataType, &numPointsReceived);
  return numPointsReceived;
}


void random_choose_centroids(Point *points, Point *centroids, 
    int numPoints, int numCentroids) {
  int indices[numCentroids];
  int randomNum = 0, i, j;
  srand(time(NULL));
  for (i = 0; i < numCentroids; i++) {
    while (1) {
      randomNum = rand() % numPoints;
      for (j = 0; j < i; j++) {
        if (indices[j] == randomNum)
          break;
      }
      if (j >= i)
        break;
    }
    indices[i] = randomNum;
  }

  for (i = 0; i < numCentroids; i++) {
    centroids[i].x = points[indices[i]].x;
    centroids[i].y = points[indices[i]].y;
  }
}

void get_new_centroids_info(Point *points, Point *centroids, 
    float *newCentroids_x, float *newCentroids_y, int *numPointsAssigned, 
    int numPoints, int numCentroids) {

  int i, j, bestCentroidId;
  float minDist, dist = 0;

  for (i = 0; i < numCentroids; i++) {
    numPointsAssigned[i] = 0;
    newCentroids_x[i] = 0;
    newCentroids_y[i] = 0;
  }

  for (i = 0; i < numPoints; i++) {
    bestCentroidId = -1;
    minDist = 0;
    dist = 0;
    for (j = 0; j < numCentroids; j++) {
      dist = euc_dist(points[i], centroids[j]);
      if (bestCentroidId == -1 || dist < minDist) {
        bestCentroidId = j;
        minDist = dist;
      }
    }

    numPointsAssigned[bestCentroidId]++;
    newCentroids_x[bestCentroidId] += points[i].x;
    newCentroids_y[bestCentroidId] += points[i].y;
  }
}

int do_kmeans(Point *points, Point *centroids,
    int numPoints, int numCentroids, float threshold, 
    int rank, MPI_Datatype pointDataType, MPI_Comm comm) {

  int itrNums, numCentroidsChanged, i;

  float newCentroids_x[numCentroids];
  float newCentroids_y[numCentroids];
  float newCentroidsRecv_x[numCentroids];
  float newCentroidsRecv_y[numCentroids];
  int numPointsAssigned[numCentroids];
  int numPointsAssignedRecv[numCentroids];

  itrNums = 0;
  do {
    get_new_centroids_info(points, centroids, 
        newCentroids_x, newCentroids_y, numPointsAssigned, 
        numPoints, numCentroids);

    // AllReduce to get summed info of new centroids. Unfortunately, 
    // AllReduce could only be applied to built-in types, so we have to 
    // convert the Point type to float coordinates, do the AllReduce 
    // job, and then convert back.
    MPI_Allreduce(newCentroids_x, newCentroidsRecv_x, numCentroids,
        MPI_FLOAT, MPI_SUM, comm);
    MPI_Allreduce(newCentroids_y, newCentroidsRecv_y, numCentroids,
        MPI_FLOAT, MPI_SUM, comm);
    MPI_Allreduce(&numPointsAssigned, &numPointsAssignedRecv, numCentroids,
        MPI_INT, MPI_SUM, comm);

    numCentroidsChanged = 0;
    Point tmp;
    for (i = 0; i < numCentroids; i++) {
      // calc the avg value as the new coordinates
      if (numPointsAssignedRecv[i] > 0) {
        newCentroids_x[i] = newCentroidsRecv_x[i] / numPointsAssignedRecv[i];
        newCentroids_y[i] = newCentroidsRecv_y[i] / numPointsAssignedRecv[i];
        tmp.x = newCentroids_x[i];
        tmp.y = newCentroids_y[i];

        if (euc_dist(tmp, centroids[i]) > threshold * threshold) {
          numCentroidsChanged++;
        }

        centroids[i].x = tmp.x;
        centroids[i].y = tmp.y;
      }
    }

    itrNums++;
  } while (numCentroidsChanged > 0);

  return itrNums;
}
