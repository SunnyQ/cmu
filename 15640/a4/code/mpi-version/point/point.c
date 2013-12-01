#include <mpi.h>
#include <stdio.h>

#include "point.h"

float euc_dist(Point p1, Point p2) {
  float x_dist = (p1.x - p2.x);
  float y_dist = (p1.y - p2.y);
  return x_dist * x_dist + y_dist * y_dist;
}

void initMPIType(MPI_Datatype *mpiDataType) {
  const int nitems = 2;
  int blocklengths[2] = {1,1};
  MPI_Datatype types[2] = {MPI_FLOAT, MPI_FLOAT};
  MPI_Aint offsets[2];

  offsets[0] = offsetof(Point, x);
  offsets[1] = offsetof(Point, y);

  MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpiDataType);
  MPI_Type_commit(mpiDataType);
}

void print_point(Point p) {
    printf("%f,%f\n", p.x, p.y);
}
