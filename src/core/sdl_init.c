#include "sdl_init.h"
bool core_init(const char *title, int w, int h, CoreState *out) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    out->win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    if (!out->win) { SDL_Quit(); return false; }
    out->ren = SDL_CreateRenderer(out->win, -1, SDL_RENDERER_ACCELERATED);
    if (!out->ren) { SDL_DestroyWindow(out->win); SDL_Quit(); return false; }
    out->w = w; out->h = h;
    return true;
}
void core_quit(CoreState *st) { SDL_DestroyRenderer(st->ren); SDL_DestroyWindow(st->win); SDL_Quit(); }