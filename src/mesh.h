#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"

typedef struct {
    vec3_t* vertices;
    Face_t* faces;
    vec3_t rotation;
} mesh_t;

extern mesh_t mesh;

void load_obj_file_data(char* filename);

#endif // MESH_H