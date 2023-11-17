#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

Triangle_t* triangles_to_render = NULL;

vec3_t camera_position = {.x = 0, .y = 0, .z = 0};

const int fov = 640;

bool is_running = false;
int previous_frame_time = 0;

void setup(void) {
	render_method = WIREFRAME;
	cull_method = CULL_BACKFACE;

	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
	color_buffer_texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			window_width,
			window_height
			);

	load_obj_file_data("../../assets/cube.obj");
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch(event.type) {
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

vec2_t project(vec3_t point) {
	vec2_t projected_point = {
		.x = (fov * point.x) / point.z,
		.y = (fov * point.y) / point.z
	};
	return projected_point;
}

void swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void bubble_short(int arr[], int n) {
	int i, j;
	bool swapped;
	for (int i = 0; i < n - 1; ++i) {
		if (arr[i] > arr[i+1]) {
			swap(&arr[i], &arr[i+1]);
			swapped = true;
		}
	}
}

void update(void) {

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = SDL_GetTicks();

	triangles_to_render = NULL;

	mesh.rotation.x += 0.00;
	mesh.rotation.y += 0.01;
	mesh.rotation.z += 0.00;

	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		Face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec3_t transformed_vertices[3];

		for (int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];

			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			transformed_vertex.z += 5;
			transformed_vertices[j] = transformed_vertex;
		}

		// Check backface Culling
		if (cull_method == CULL_BACKFACE) {
			vec3_t vector_a = transformed_vertices[0];
			vec3_t vector_b = transformed_vertices[1];
			vec3_t vector_c = transformed_vertices[2];

			// Get the vector subtraction of B-A and C-A
			vec3_t vector_ab = vec3_sub(vector_b, vector_a);
			vec3_t vector_ac = vec3_sub(vector_c, vector_a);

			// Compute the face normal (using cross product to find perpendicular)
			vec3_t normal = vec3_cross(vector_ab, vector_ac);
			vec3_normalize(&normal);

			// Find the vector between a point in the triangle and the camera origin
			vec3_t camera_ray = vec3_sub(camera_position, vector_a );

			// Calculate how aligned the camera ray is with the face normal (using dot product)
			float dot_normal_camera = vec3_dot(normal, camera_ray);

			if (dot_normal_camera < 0) {
				continue;
			}
		}

		vec2_t projected_points[3];

		for (int j = 0; j < 3; j++) {
			projected_points[j] = project(transformed_vertices[j]);

			projected_points[j].x += (window_width / 2);
			projected_points[j].y += (window_height / 2);
		}

		// Calculate the average depth
		float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3;
		Triangle_t projected_triangle = {
				.points = {
						{projected_points[0].x, projected_points[0].y },
						{projected_points[1].x, projected_points[1].y },
						{projected_points[2].x, projected_points[2].y }
				},
				.color = 0xFF00FF00,
				.avg_depth = avg_depth
		};

		array_push(triangles_to_render, projected_triangle);
	}

	// Sort triangles by the avg depth
	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; ++i) {
		for (int j = i; j < num_triangles; ++j) {
			if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth) {
				Triangle_t temp = triangles_to_render[i];
				triangles_to_render[i] = triangles_to_render[j];
				triangles_to_render[j] = temp;
			}
		}
	}
}


void render(void) {
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
			draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFF1100);
			draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFF1100);
			draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFF1100);
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

int main(void) {
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

