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

// return number of changed centroids
int calc_new_centroids(Point *points, Point *centroids,
    int numPoints, int numCentroids, float threshold) {
  Point *newCentroids = malloc(numCentroids * sizeof(Point));
  int numPointsAssigned[numCentroids];
  int i, j, bestCentroidId, numCentroidsChanged;
  float minDist, dist = 0;

  for (i = 0; i < numCentroids; i++) {
    numPointsAssigned[i] = 0;
    newCentroids[i].x = 0;
    newCentroids[i].y = 0;
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
    newCentroids[bestCentroidId].x += points[i].x;
    newCentroids[bestCentroidId].y += points[i].y;
  }
  
  numCentroidsChanged = 0;
  for (i = 0; i < numCentroids; i++) {
    // calc the avg value as the new coordinates
    if (numPointsAssigned[i] > 0) {
      newCentroids[i].x /= numPointsAssigned[i];
      newCentroids[i].y /= numPointsAssigned[i];

      if (euc_dist(newCentroids[i], centroids[i]) > threshold * threshold) {
        numCentroidsChanged++;
      }

      centroids[i].x = newCentroids[i].x;
      centroids[i].y = newCentroids[i].y;
    }
  }

  free(newCentroids);

  return numCentroidsChanged;
}

int do_kmeans(Point *points, Point *centroids,
    int numPoints, int numCentroids, float threshold) {
  int itrNums, numCentroidsChanged;

  itrNums = 0;
  do {
    numCentroidsChanged = calc_new_centroids(points, centroids,
        numPoints, numCentroids, threshold);
    itrNums++;
  } while (numCentroidsChanged > 0);

  return itrNums;
}


