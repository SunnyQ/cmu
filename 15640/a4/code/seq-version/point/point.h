#ifndef _POINT_H_
#define _POINT_H_

typedef struct {
  float x, y;
} Point;

float euc_dist(Point p1, Point p2);
void print_point(Point p);

#endif
