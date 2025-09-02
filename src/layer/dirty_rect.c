#include "dirty_rect.h"
#include <stdbool.h>
void dirty_union(SDL_Rect *a, const SDL_Rect *b) {
    int x1 = (a->x < b->x) ? a->x : b->x;
    int y1 = (a->y < b->y) ? a->y : b->y;
    int x2 = (a->x + a->w > b->x + b->w) ? a->x + a->w : b->x + b->w;
    int y2 = (a->y + a->h > b->y + b->h) ? a->y + a->h : b->y + b->h;
    
    a->x = x1;
    a->y = y1;
    a->w = x2 - x1;
    a->h = y2 - y1;
}

bool dirty_intersect(const SDL_Rect *a, const SDL_Rect *b) {
    return !(a->x >= b->x + b->w || b->x >= a->x + a->w ||
             a->y >= b->y + b->h || b->y >= a->y + a->h);
}
