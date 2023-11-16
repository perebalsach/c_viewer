#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"
#include "stdint.h"

typedef struct {
    int a;
    int b;
    int c;
} Face_t;

typedef struct {
    vec2_t points[3];
} Triangle_t;

void int_swap(int* a, int* b);
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

#endif // TRIANGLE_H