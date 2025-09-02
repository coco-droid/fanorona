#include "layer_manager.h"
#include "dirty_rect.h"
#include <stdlib.h>
#include <string.h>

LayerManager *lm_create(void) {
    LayerManager *lm = malloc(sizeof(LayerManager));
    lm->root = layer_create();
    lm->dirty_list = malloc(sizeof(SDL_Rect) * 32);
    lm->dirty_count = 0;
    lm->dirty_cap = 32;
    return lm;
}

void lm_destroy(LayerManager *lm) {
    layer_destroy(lm->root);
    free(lm->dirty_list);
    free(lm);
}

void lm_add_dirty(LayerManager *lm, const SDL_Rect *r) {
    if (lm->dirty_count >= lm->dirty_cap) {
        lm->dirty_cap *= 2;
        lm->dirty_list = realloc(lm->dirty_list, sizeof(SDL_Rect) * lm->dirty_cap);
    }
    lm->dirty_list[lm->dirty_count++] = *r;
}

static void render_layer_recursive(Layer *l, SDL_Renderer *ren) {
    if (l->on_render) {
        l->on_render(l, ren);
    }
    for (Layer *child = l->children; child; child = child->next) {
        render_layer_recursive(child, ren);
    }
}

void lm_render(LayerManager *lm, SDL_Renderer *ren) {
    render_layer_recursive(lm->root, ren);
    lm->dirty_count = 0; // Reset dirty regions after render
}

static void dispatch_to_layer(Layer *l, SDL_Event *e) {
    if (l->on_event) {
        l->on_event(l, e);
    }
    for (Layer *child = l->children; child; child = child->next) {
        dispatch_to_layer(child, e);
    }
}

void lm_dispatch(LayerManager *lm, SDL_Event *e) {
    dispatch_to_layer(lm->root, e);
}
