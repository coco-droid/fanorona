#include "hitbox.h"

bool hitbox_contains(const SDL_Rect *r, int x, int y) {
    if (!r) return false;
    return (x >= r->x && x < r->x + r->w && 
            y >= r->y && y < r->y + r->h);
}
