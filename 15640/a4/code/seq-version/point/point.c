#include <stdio.h>

#include "point.h"

float euc_dist(Point p1, Point p2) {
  float x_dist = (p1.x - p2.x);
  float y_dist = (p1.y - p2.y);
  return x_dist * x_dist + y_dist * y_dist;
}

void print_point(Point p) {
  printf("%f,%f\n", p.x, p.y);
}
