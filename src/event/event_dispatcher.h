#pragma once
#include "../../src/layer/layer.h"
#include "../../src/layer/layer_manager.h"
#include <SDL2/SDL.h>

typedef struct {
    LayerManager *lm;
} EventDispatcher;

EventDispatcher *ed_create(LayerManager *lm);
void             ed_destroy(EventDispatcher *ed);
void             ed_dispatch(EventDispatcher *ed, SDL_Event *e);