#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Window   *win;
    SDL_Renderer *ren;
    int w, h;
} CoreState;

bool core_init(const char *title, int w, int h, CoreState *out);
void core_quit(CoreState *st);