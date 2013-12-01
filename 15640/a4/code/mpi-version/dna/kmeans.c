#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
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

int get_portion_size(int numStrands, int numProcs, int rank) {
  int divide = numStrands / numProcs;
  int remain = numStrands % numProcs;
  return rank < remain ? divide + 1 : divide;
}

int distribute_strands(Strand* strands, int numStrands, int lenStrand,
    int numProcs, MPI_Comm comm) {

  int numStrandsRemained = get_portion_size(numStrands, numProcs, 0);
  int ptr = numStrandsRemained;
  int numStrandsToDistribute, i;

  for ( i = 1; i < numProcs; i++) {
    numStrandsToDistribute = get_portion_size(numStrands, numProcs, i);
    MPI_Send(strands[ptr], numStrandsToDistribute * lenStrand, MPI_CHAR, i, i, comm);
    ptr += numStrandsToDistribute;
  }

  return numStrandsRemained;
}

int receive_strands(Strand* strands, int numStrands, int lenStrand,
    int rank, MPI_Comm comm) {

  int numBytesReceived;
  MPI_Status status;
  MPI_Recv(strands[0], numStrands * lenStrand, MPI_CHAR, 0, rank, comm, &status);
  MPI_Get_count(&status, MPI_CHAR, &numBytesReceived);
  return numBytesReceived / lenStrand;
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

void get_new_centroids_info(Strand *strands, Strand *centroids, 
    int numStrands, int numCentroids, int lenStrand,
    int *ACount, int *CCount, int *GCount, int *TCount) {

  int i, j, bestCentroidId;
  int minDist, dist = 0, index;

  for (i = 0; i < numCentroids; i++) {
    for (j = 0; j < lenStrand; j++) {
      index = i * lenStrand + j;
      ACount[index] = 0;
      CCount[index] = 0;
      GCount[index] = 0;
      TCount[index] = 0;
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
      index = bestCentroidId * lenStrand + j;
      if (strands[i][j] == 'A')
        ACount[index]++;
      else if (strands[i][j] == 'C')
        CCount[index]++;
      else if (strands[i][j] == 'G')
        GCount[index]++;
      else
        TCount[index]++;
    }
  }
}

int do_kmeans(Strand *strands, Strand *centroids,
    int numStrands, int numCentroids, int lenStrand, 
    int rank, MPI_Comm comm) {

  int itrNums, numCentroidsChanged, i, j;

  int ACount[numCentroids *lenStrand];
  int CCount[numCentroids * lenStrand];
  int GCount[numCentroids * lenStrand];
  int TCount[numCentroids * lenStrand];
  int ACountRecv[numCentroids * lenStrand];
  int CCountRecv[numCentroids * lenStrand];
  int GCountRecv[numCentroids * lenStrand];
  int TCountRecv[numCentroids * lenStrand];

  itrNums = 0;
  do {
    get_new_centroids_info(strands, centroids, 
        numStrands, numCentroids, lenStrand,
        ACount, CCount, GCount, TCount);

    // AllReduce to get summed info of new centroids. Unfortunately, 
    // AllReduce could only be applied to built-in types, so we have to 
    // convert the Strand type to float coordinates, do the AllReduce 
    // job, and then convert back.
    MPI_Allreduce(ACount, ACountRecv, numCentroids * lenStrand,
        MPI_INT, MPI_SUM, comm);
    MPI_Allreduce(CCount, CCountRecv, numCentroids * lenStrand,
        MPI_INT, MPI_SUM, comm);
    MPI_Allreduce(GCount, GCountRecv, numCentroids * lenStrand,
        MPI_INT, MPI_SUM, comm);
    MPI_Allreduce(TCount, TCountRecv, numCentroids * lenStrand,
        MPI_INT, MPI_SUM, comm);

    numCentroidsChanged = 0;
    char tmpStrand[lenStrand];
    for (i = 0; i < numCentroids; i++) {
      // calc the majority value as the new strands
      for (j = 0; j < lenStrand; j++) {
        int index = i * lenStrand + j;
        if (ACountRecv[index] > max(CCountRecv[index], max(GCountRecv[index], TCountRecv[index]))) 
          tmpStrand[j] = 'A';
        else if (CCountRecv[index] > max(ACountRecv[index], max(GCountRecv[index], TCountRecv[index]))) 
          tmpStrand[j] = 'C';
        else if (GCountRecv[index] > max(ACountRecv[index], max(CCountRecv[index], TCountRecv[index]))) 
          tmpStrand[j] = 'G';
        else
          tmpStrand[j] = 'T';
      }

      if (memcmp(tmpStrand, centroids[i], lenStrand) != 0)
        numCentroidsChanged++;
       memcpy(centroids[i], tmpStrand, lenStrand);
    }

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
