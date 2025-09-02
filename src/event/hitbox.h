#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>

bool hitbox_contains(const SDL_Rect *r, int x, int y);