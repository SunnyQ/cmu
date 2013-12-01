#ifndef _KMEANS_H_
#define _KMEANS_H_

#include "point.h" 

int load_points(Point* points, char *filePath, int numPoints);
void random_choose_centroids(Point *points, Point *centroids, 
    int numPoints, int numCentroids);
void get_new_centroids_info(Point *points, Point *centroids,
    float *newCentroids_x, float *newCentroids_y, int *numPointsAssigned, 
    int numPoints, int numCentroids);
int do_kmeans(Point *points, Point *centroids,
    int numPoints, int numCentroids, float threshold, 
    int rank, MPI_Datatype pointDataType, MPI_Comm comm);
int get_portion_size(int numPoints, int numProcs, int rank);
int distribute_points(Point* points, int numPoints, int numProcs,
    MPI_Datatype pointDataType, MPI_Comm comm);
int receive_points(Point* points, int numPoints, int rank,
    MPI_Datatype pointDataType, MPI_Comm comm);

#endif
