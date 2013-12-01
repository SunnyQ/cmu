#ifndef _POINT_H_
#define _POINT_H_
#include <mpi.h>

typedef struct {
  float x, y;
} Point;

float euc_dist(Point p1, Point p2);
void print_point(Point p);
void initMPIType(MPI_Datatype *mpiDataType);

#endif
