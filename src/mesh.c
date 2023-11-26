#include "array.h"
#include "mesh.h"
#include <stdio.h>
#include <string.h>
#include <string.h>

mesh_t mesh = {
		.vertices = NULL,
		.faces = NULL,
		.rotation = {0, 0, 0},
		.scale = {1.0, 1.0, 1.0},
		.rotation = {0.0, 0.0, 0.0},
		.translation = {1.0, 1.0, 1.0}
};

void load_obj_file_data(char *filename) {
	FILE *file;

	file = fopen(filename, "r");
	if (file == NULL) {
		printf("\n ---------->>Could not open the file %s", filename);
		return;
	}

	char buffer[1024];
	while (fgets(buffer, 1024, file) != NULL) {
		if (strncmp(buffer, "v ", 2) == 0) {
			vec3_t vertex;
			sscanf(buffer, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			array_push(mesh.vertices, vertex);
		}
		if (strncmp(buffer, "f ", 2) == 0) {
			int vertex_indices[3];
			int texture_indices[3];
			int normal_indices[3];
			sscanf(
					buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d ",
					&vertex_indices[0], &texture_indices[0], &normal_indices[0],
					&vertex_indices[1], &texture_indices[1], &normal_indices[1],
					&vertex_indices[2], &texture_indices[2], &normal_indices[2]
			);
			Face_t face = {
					.a = vertex_indices[0],
					.b = vertex_indices[1],
					.c = vertex_indices[2]
			};

			array_push(mesh.faces, face);
		}
	}

	fclose(file);
}
