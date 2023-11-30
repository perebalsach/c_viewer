#include "display.h"

int window_width = 800;
int window_height = 600;

SDL_Texture* color_buffer_texture = NULL;
uint32_t* color_buffer = NULL;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void render_color_buffer(void) {
    SDL_UpdateTexture(
        color_buffer_texture, 
        NULL, 
        color_buffer, 
        (int)(window_width * sizeof(uint32_t))
    );

    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(color_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}


bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL. \n");
        return false;
    }
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w / 2;
    window_height = display_mode.h / 2;

    SDL_Window* window = SDL_CreateWindow(
            NULL,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width,
            window_height,
            SDL_WINDOW_BORDERLESS
            );

    if (!window) {
        fprintf(stderr, "Error creating SDL Window Window\n");
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating the SDL Renderer\n");
    }
    
    SDL_SetWindowFullscreen(window, SDL_WINDOW_BORDERLESS);

    return true;
}

void draw_pixel(int x, int y, color_t color) {
    if (x > 0 && x < window_width && y >= 0 && y < window_height) {
        color_buffer[(window_width * y ) + x ] = color;
    }
}

void draw_rect(int x, int y, int width, int height, color_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x , current_y , color);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, color_t color) {
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float)longest_side_length;
    float y_inc = delta_y / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;
    for (int i = 0; i <= longest_side_length; i++) {
        draw_pixel(round(current_x), round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}
