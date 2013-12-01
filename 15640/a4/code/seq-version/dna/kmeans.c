#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <time.h>

#include "kmeans.h"
#include "strand.h"

#define BUFFER_SIZE 512

static int max(int a, int b ){
  return a > b ? a : b;
}

int load_strands(Strand* strands, char *filePath, 
    int numStrands, int lenStrand) {
  FILE *file;
  char buf[BUFFER_SIZE];
  int numStrandsLoaded = 0;
  
  file = fopen(filePath, "r");
  if (file == NULL) {
    return -1;
  }

  while (fgets(buf, BUFFER_SIZE, file) != NULL && numStrandsLoaded < numStrands) {
    memcpy(strands[numStrandsLoaded], buf, lenStrand);
    numStrandsLoaded++;
  }

  fclose(file);
  return numStrandsLoaded;
}

void random_choose_centroids(Strand *strands, Strand *centroids, 
    int numStrands, int numCentroids, int lenStrand) {
  int indices[numCentroids];
  int randomNum = 0, i, j;
  srand(time(NULL));
  for (i = 0; i < numCentroids; i++) {
    while (1) {
      randomNum = rand() % numStrands;
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
    memcpy(centroids[i], strands[indices[i]], lenStrand);
  }
}

// return number of changed centroids
static int calc_new_centroids(Strand *strands, Strand *centroids,
    int numStrands, int numCentroids, int lenStrand) {

  // record number of different bases for each centroid
  int ACount[numCentroids][lenStrand];
  int CCount[numCentroids][lenStrand];
  int GCount[numCentroids][lenStrand];
  int TCount[numCentroids][lenStrand];
  
  int i, j, bestCentroidId, numCentroidsChanged;
  int minDist, dist = 0;

  for (i = 0; i < numCentroids; i++) {
    for (j = 0; j < lenStrand; j++) {
      ACount[i][j] = 0;
      CCount[i][j] = 0;
      GCount[i][j] = 0;
      TCount[i][j] = 0;
    }
  }

  for (i = 0; i < numStrands; i++) {
    bestCentroidId = -1;
    minDist = 0;
    dist = 0;
    for (j = 0; j < numCentroids; j++) {
      dist = edit_dist(strands[i], centroids[j], lenStrand);
      if (bestCentroidId == -1 || dist < minDist) {
        bestCentroidId = j;
        minDist = dist;
      }
    }

    for (j = 0; j < lenStrand; j++) {
      if (strands[i][j] == 'A')
        ACount[bestCentroidId][j]++;
      else if (strands[i][j] == 'C')
        CCount[bestCentroidId][j]++;
      else if (strands[i][j] == 'G')
        GCount[bestCentroidId][j]++;
      else
        TCount[bestCentroidId][j]++;
    }
  }
  
  numCentroidsChanged = 0;
  char tmpStrand[lenStrand];

  for (i = 0; i < numCentroids; i++) {
    // calc the majority value as the new strands
    for (j = 0; j < lenStrand; j++) {
      if (ACount[i][j] > max(CCount[i][j], max(GCount[i][j], TCount[i][j]))) 
        tmpStrand[j] = 'A';
      else if (CCount[i][j] > max(ACount[i][j], max(GCount[i][j], TCount[i][j]))) 
        tmpStrand[j] = 'C';
      else if (GCount[i][j] > max(ACount[i][j], max(CCount[i][j], TCount[i][j]))) 
        tmpStrand[j] = 'G';
      else
        tmpStrand[j] = 'T';
    }
    if (memcmp(tmpStrand, centroids[i], lenStrand) != 0)
      numCentroidsChanged++;

    memcpy(centroids[i], tmpStrand, lenStrand);
  }

  return numCentroidsChanged;
}

int do_kmeans(Strand *strands, Strand *centroids,
    int numStrands, int numCentroids, int lenStrand) {
  int itrNums, numCentroidsChanged;

  itrNums = 0;
  do {
    numCentroidsChanged = calc_new_centroids(strands, centroids,
        numStrands, numCentroids, lenStrand);
    itrNums++;
  } while (numCentroidsChanged > 0);

  return itrNums;
}

void init_memory_space(Strand **strands, char **strandsMem,
    int numStrands, int lenStrand) {
  int i;
  *strands = malloc(numStrands * sizeof(Strand));
  assert(*strands != NULL);
  *strandsMem = malloc(numStrands * lenStrand * sizeof(char));
  assert(*strandsMem != NULL);
  for (i = 0; i < numStrands; i++) {
    (*strands)[i] = *strandsMem + i * lenStrand;
  }
}
