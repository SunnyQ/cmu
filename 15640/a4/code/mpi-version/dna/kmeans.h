#ifndef _KMEANS_H_
#define _KMEANS_H_

#include <mpi.h>
#include "strand.h" 

int load_strands(Strand* strands, char *filePath, 
    int numStrands, int lenStrand);

void random_choose_centroids(Strand *strands, Strand *centroids, 
    int numStrands, int numCentroids, int lenStrand);

void get_new_centroids_info(Strand *strands, Strand *centroids, 
    int numStrands, int numCentroids, int lenStrand,
    int *ACount, int *CCount, int *GCount, int *TCount);

int do_kmeans(Strand *strands, Strand *centroids,
    int numStrands, int numCentroids, int lenStrand, 
    int rank, MPI_Comm comm);

int get_portion_size(int numStrands, int numProcs, int rank);

int distribute_strands(Strand* strands, int numStrand, int lenStrand,
    int numProcs, MPI_Comm comm);
int receive_strands(Strand* strands, int numStrands, int lenStrand, 
    int rank, MPI_Comm comm);

void init_memory_space(Strand **strands, char **strandsMem, 
    int numStrands, int lenStrand);

#endif
