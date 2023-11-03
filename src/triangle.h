#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"

typedef struct {
    int a;
    int b;
    int c;
} Face_t ;

typedef struct {
    vec2_t points[3];
} Triangle_t;

#endif // TRIANGLE_H