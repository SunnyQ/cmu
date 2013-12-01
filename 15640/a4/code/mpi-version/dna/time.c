#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "time.h"

double get_curr_time() {
    double currTime;
    struct timeval timeVal;

    assert(gettimeofday(&timeVal, NULL) != -1);
    currTime = ((double)timeVal.tv_sec) + ((double)timeVal.tv_usec) / 1000000.0;
    return currTime;
}


