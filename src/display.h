#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)
typedef uint32_t color_t;

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE
} cull_method;

enum render_method {
	RENDER_WIRE,
	RENDER_WIRE_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIRE,
	RENDER_FILL_SMOOTH_SHADED,
	RENDER_FILL_FLAT_SHADED,
	DISPLAY_NORMALS
} render_method;

extern int window_width;
extern int window_height;

extern SDL_Texture* color_buffer_texture;
extern uint32_t* color_buffer;

extern SDL_Window* window;
extern SDL_Renderer* renderer;

bool initialize_window(void);
void clear_color_buffer(uint32_t color);
void render_color_buffer(void);
void destroy_window(void);
void draw_pixel(int x, int y, color_t color);
void draw_rect(int x, int y, int width, int height, color_t color);
void draw_line(int x0, int y0, int x1, int y1, color_t color);

#endif