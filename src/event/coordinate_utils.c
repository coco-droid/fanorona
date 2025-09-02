#include "coordinate_utils.h"

SDL_Point layer_global_to_local(const Layer *l, int gx, int gy) {
    SDL_Point local = {0, 0};
    if (!l) return local;
    
    local.x = gx - l->rect.x;
    local.y = gy - l->rect.y;
    return local;
}
