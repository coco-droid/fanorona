#pragma once
#include "layer.h"

typedef struct {
    Layer *root;
    SDL_Rect *dirty_list;
    int dirty_count, dirty_cap;
} LayerManager;

LayerManager *lm_create(void);
void          lm_destroy(LayerManager *lm);
void          lm_add_dirty(LayerManager *lm, const SDL_Rect *r);
void          lm_render(LayerManager *lm, SDL_Renderer *ren);
void          lm_dispatch(LayerManager *lm, SDL_Event *e);