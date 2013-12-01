#ifndef _KMEANS_H_
#define _KMEANS_H_

#include "strand.h" 

int load_strands(Strand* strands, char *filePath, 
    int numStrands, int lenStrand);

void random_choose_centroids(Strand *strands, Strand *centroids, 
    int numStrands, int numCentroids, int lenStrand);

int do_kmeans(Strand *strands, Strand *centroids,
    int numStrands, int numCentroids, int lenStrand);

void init_memory_space(Strand **strands, char **strandsMem,
        int numStrands, int lenStrand);
#endif
