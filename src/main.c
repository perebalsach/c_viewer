#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "sorting.h"
#include "matrix.h"

Triangle_t *triangles_to_render = NULL;

bool is_running = false;
int previous_frame_time = 0;

vec3_t camera_position = {.x = 0, .y = 0, .z = 0};
mat4_t proj_matrix;

void setup(void) {
	render_method = WIREFRAME;
	cull_method = CULL_BACKFACE;

	color_buffer = (uint32_t *) malloc(sizeof(uint32_t) * window_width * window_height);
	color_buffer_texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			window_width,
			window_height
	);

	// Create perspective projection matrix
	float fov = M_PI / 3.0; // 60deg same as 180/3
	float aspect = (float)window_height / (float)window_width;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

	load_obj_file_data("../../assets/monkey.obj");
	load_cube_mesh_data();
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				is_running = false;
			} else if (event.key.keysym.sym == SDLK_1) {
				render_method = WIREFRAME_DOTS;
			} else if (event.key.keysym.sym == SDLK_2) {
				render_method = WIREFRAME;
			} else if (event.key.keysym.sym == SDLK_3) {
				render_method = SHADED;
			} else if (event.key.keysym.sym == SDLK_4) {
				render_method = WIREFRAME_SHADED;
			} else if (event.key.keysym.sym == SDLK_c) {
				cull_method = CULL_BACKFACE;
			} else if (event.key.keysym.sym == SDLK_d) {
				cull_method = CULL_NONE;
			}
	}
}

void update(void) {

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = SDL_GetTicks();

	triangles_to_render = NULL;

	mesh.rotation.x += 0.01;
	mesh.translation.z = 5.0;

	// Scale translation and rotation matrices
	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		Face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec4_t transformed_vertices[3];

		mat4_t world_matrix = mat4_identity();
		// Loop all the vertices of this current face and apply transformations
		for (int j = 0; j < 3; j++) {
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

			// Use a matrix to scale vertex
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			// Save transformed vertex in the array of transformed vertices
			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

			transformed_vertices[j] = transformed_vertex;
		}

		// Check backface Culling
		if (cull_method == CULL_BACKFACE) {
			vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
			vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
			vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

			// Get the vector subtraction of B-A and C-A
			vec3_t vector_ab = vec3_sub(vector_b, vector_a);
			vec3_t vector_ac = vec3_sub(vector_c, vector_a);

			// Compute the face normal (using cross product to find perpendicular)
			vec3_t normal = vec3_cross(vector_ab, vector_ac);
			vec3_normalize(&normal);

			// Find the vector between a point in the triangle and the camera origin
			vec3_t camera_ray = vec3_sub(camera_position, vector_a);

			// Calculate how aligned the camera ray is with the face normal (using dot product)
			float dot_normal_camera = vec3_dot(normal, camera_ray);

			if (dot_normal_camera < 0) {
				continue;
			}
		}

		vec4_t projected_points[3];
		// Loop all the 3 vertices to perform projection
		for (int j = 0; j < 3; j++) {
			projected_points[j] = mat4_mult_vec4_project(proj_matrix, transformed_vertices[j]);

			// Scale the projected points into the view
			projected_points[j].x *= (float)(window_width/2.0);
			projected_points[j].y *= (float)(window_height/2.0);

			// Translate the projected points to the middle of the screen
			projected_points[j].x += (float)(window_width / 2.0);
			projected_points[j].y += (float)(window_height / 2.0);

		}

		// Calculate the average depth
		float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;
		Triangle_t projected_triangle = {
			.points = {
				{projected_points[0].x, projected_points[0].y},
				{projected_points[1].x, projected_points[1].y},
				{projected_points[2].x, projected_points[2].y}
			},
			.color = 0xFF00FF00,
			.avg_depth = avg_depth
		};

		array_push(triangles_to_render, projected_triangle);
	}

	// Sort triangles by the avg depth with the quick sort algorithm
	int n = sizeof(triangles_to_render[0]) / sizeof(triangles_to_render);
	quick_sort(triangles_to_render, 0, n - 1);
}


void render(void) {
	SDL_RenderClear(renderer);

	int num_triangles = array_length(triangles_to_render);
	// Loop all projected triangles and render them
	for (int i = 0; i < num_triangles; i++) {
		Triangle_t triangle = triangles_to_render[i];

		if (render_method == SHADED || render_method == WIREFRAME_SHADED) {
			draw_filled_triangle(
					triangle.points[0].x, triangle.points[0].y,
					triangle.points[1].x, triangle.points[1].y,
					triangle.points[2].x, triangle.points[2].y,
					0x6C73A6
			);
		}

		if (render_method == WIREFRAME || render_method == WIREFRAME_DOTS || render_method == WIREFRAME_SHADED) {
			draw_triangle(
					triangle.points[0].x, triangle.points[0].y,
					triangle.points[1].x, triangle.points[1].y,
					triangle.points[2].x, triangle.points[2].y,
					0xFFFFFF00
			);
		}

		if (render_method == WIREFRAME_DOTS) {
			draw_rect(triangle.points[0].x - 3, triangle.points[0].y -3, 6, 6, 0xFF1100);
			draw_rect(triangle.points[1].x - 3, triangle.points[1].y -3, 6, 6, 0xFF1100);
			draw_rect(triangle.points[2].x - 3, triangle.points[2].y -3, 6, 6, 0xFF1100);
		}
	}

	array_free(triangles_to_render);
	render_color_buffer();
	clear_color_buffer(0x4e5052);
	SDL_RenderPresent(renderer);
}


void free_resources(void) {
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(int argc, char *argv[]) {
	is_running = initialize_window();

	setup();

	while (is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();
	free_resources();

	return 0;
}

