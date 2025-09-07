#pragma once
#include "../layer/layer.h"
#include <SDL2/SDL_ttf.h>

typedef struct {
    Layer base;
    SDL_Texture *tex_normal, *tex_hover, *tex_pressed;
    SDL_Texture *image;  // New: Background image texture
    void (*on_click)(void *ud);
    void *ud;
} Button;

Button *button_create(const char *text, TTF_Font *f, SDL_Texture *image, void (*cb)(void *), void *ud);