#ifndef _KMEANS_H_
#define _KMEANS_H_

#include "point.h" 

int load_points(Point* points, char *filePath, int numPoints);
void random_choose_centroids(Point *points, Point *centroids, 
    int numPoints, int numCentroids);
int generate_new_centroids(Point *points, Point *centroids,
    int numPoints, int numCentroids, float threshold);
int do_kmeans(Point *points, Point *centroids,
    int numPoints, int numCentroids, float threshold);

#endif
