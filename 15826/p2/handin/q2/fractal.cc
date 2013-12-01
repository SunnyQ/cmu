#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_N_SURFACE 100000 
#define UP 'U'
#define DOWN 'D'
#define WEST 'W'
#define EAST 'E'
#define NORTH 'N'
#define SOUTH 'S'

typedef struct
{
    double p1[3];
    double p2[3];
    double p3[3];
    double p4[3];
    double unit;
    char direction;
} surface;

surface *pool[MAX_N_SURFACE];
int pos = 0;

surface *getNewSurface(double unit, char direction) 
{
    surface *s = (surface *) malloc(sizeof(surface));
    s->unit = unit;
    s->direction = direction;
    s->p1[0] = 0.0;
    s->p1[1] = 0.0;
    s->p1[2] = 0.0;
    return s;
}

void setupRestPoints(surface *s)
{
    memcpy(s->p2, s->p1, 3 * sizeof(double));
    memcpy(s->p3, s->p1, 3 * sizeof(double));
    memcpy(s->p4, s->p1, 3 * sizeof(double));
	
    if (s->direction == NORTH || s->direction == SOUTH) {
        s->p2[0] += s->unit;
        s->p3[0] += s->unit;
    }
    else {
        s->p2[1] += s->unit;
        s->p3[1] += s->unit;
    }
    
    if (s->direction == UP || s->direction == DOWN) {
        s->p3[0] += s->unit;
        s->p4[0] += s->unit;
    }
    else {
        s->p3[2] += s->unit;
        s->p4[2] += s->unit;
    }
}

surface *getSpecialBoxSurface(surface *s, char direction)
{
    surface *box_s = getNewSurface(s->unit, direction);
    memcpy(box_s->p1, s->p1, 3 * sizeof(double));
	
    if (s->direction == WEST)
        box_s->p1[0] -= box_s->unit;
    else if (s->direction == NORTH)
        box_s->p1[1] -= box_s->unit;
    else if (s->direction == DOWN)
        box_s->p1[2] -= box_s->unit;
		
    if (direction == EAST)
        box_s->p1[0] += box_s->unit;
    else if (direction == SOUTH)
        box_s->p1[1] += box_s->unit;
    else if (direction == UP)
        box_s->p1[2] += box_s->unit;

    return box_s;
}

void fractal(surface *s, int n)
{
    int i, j;

    if (n == 0)
    {
        pool[pos++] = s;
        return;
    }

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            surface *sub_s = getNewSurface(s->unit / 3.0, s->direction);
            memcpy(sub_s->p1, s->p1, 3 * sizeof(double));
            if (sub_s->direction == UP || sub_s->direction == DOWN)
            {
                sub_s->p1[0] += sub_s->unit * i;
                sub_s->p1[1] += sub_s->unit * j;
            }
            else if (sub_s->direction == WEST || sub_s->direction == EAST)
            {
                sub_s->p1[1] += sub_s->unit * i;
                sub_s->p1[2] += sub_s->unit * j;
            }
            else if (sub_s->direction == NORTH || sub_s->direction == SOUTH)
            {
                sub_s->p1[0] += sub_s->unit * i;
                sub_s->p1[2] += sub_s->unit * j;
            }

            if (i == 1 && j == 1) {
                if (sub_s->direction != UP)
                    fractal(getSpecialBoxSurface(sub_s, DOWN), n - 1);
	        if (sub_s->direction != DOWN)
                    fractal(getSpecialBoxSurface(sub_s, UP), n - 1);
                if (sub_s->direction != SOUTH)
                    fractal(getSpecialBoxSurface(sub_s, NORTH), n - 1);
                if (sub_s->direction != NORTH)
                    fractal(getSpecialBoxSurface(sub_s, SOUTH), n - 1);
                if (sub_s->direction != EAST)
                    fractal(getSpecialBoxSurface(sub_s, WEST), n - 1);
                if (sub_s->direction != WEST)
                    fractal(getSpecialBoxSurface(sub_s, EAST), n - 1);
            }
            else
                fractal(sub_s, n - 1);
        }
    }
}

int main(int nargs, char* args[])
{
    int i, n;
    FILE *f;
	char outputFile[64];

    if (nargs != 2)
    {
        fprintf(stderr, "Usage: %s <n>\n", args[0]);
        return -1;
    }
	
    n = atoi(args[1]);
    fractal(getNewSurface(1.0, UP), n);

    sprintf(outputFile, "output_%d.txt", n);
    f = fopen(outputFile, "w");
    fprintf(f, "%d\n", pos);
    for (i = 0; i < pos; i++)
    {
        setupRestPoints(pool[i]);
        fprintf(f, "%lf %lf %lf %lf %lf %lf %lf "
                "%lf %lf %lf %lf %lf\n", 
                pool[i]->p1[0], pool[i]->p1[1], pool[i]->p1[2], 
                pool[i]->p2[0], pool[i]->p2[1], pool[i]->p2[2],
                pool[i]->p3[0], pool[i]->p3[1], pool[i]->p3[2], 
                pool[i]->p4[0], pool[i]->p4[1], pool[i]->p4[2]);
        //fprintf(f, "%lf %lf %lf\n", pool[i]->p1[0], pool[i]->p1[1], pool[i]->p1[2]);
        //fprintf(f, "%lf %lf %lf\n", pool[i]->p2[0], pool[i]->p2[1], pool[i]->p2[2]);
        //fprintf(f, "%lf %lf %lf\n", pool[i]->p3[0], pool[i]->p3[1], pool[i]->p3[2]);
        //fprintf(f, "%lf %lf %lf\n", pool[i]->p4[0], pool[i]->p4[1], pool[i]->p4[2]);
    }
    fclose(f);
    return 0;
}
