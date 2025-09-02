#pragma once
#include <SDL2/SDL.h>

// Render target functionality for off-screen rendering
typedef struct {
    SDL_Texture *texture;
    int width, height;
} RenderTarget;

RenderTarget *render_target_create(SDL_Renderer *ren, int w, int h);
void render_target_destroy(RenderTarget *rt);
