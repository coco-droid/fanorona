#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>
typedef struct Layer Layer;
struct Layer {
    SDL_Rect   rect;      // absolu
    int        z_index;
    bool       dirty;
    Layer     *parent, *next, *children;
    void     (*on_render)(Layer *self, SDL_Renderer *ren);
    void     (*on_event) (Layer *self, SDL_Event *e);
};

Layer *layer_create(void);
void   layer_destroy(Layer *l);
void   layer_set_rect(Layer *l, const SDL_Rect *r);   // marque dirty
void   layer_add_child(Layer *parent, Layer *child);