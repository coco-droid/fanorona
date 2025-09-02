#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>
void dirty_union(SDL_Rect *a, const SDL_Rect *b);
bool dirty_intersect(const SDL_Rect *a, const SDL_Rect *b);