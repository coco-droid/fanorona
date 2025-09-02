#include "layer.h"
#include <stdlib.h>
#include <string.h>

Layer *layer_create(void) {
    Layer *l = malloc(sizeof(Layer));
    memset(l, 0, sizeof(Layer));
    l->rect = (SDL_Rect){0, 0, 0, 0};
    l->z_index = 0;
    l->dirty = true;
    return l;
}

void layer_destroy(Layer *l) {
    if (!l) return;
    
    // Destroy children recursively
    Layer *child = l->children;
    while (child) {
        Layer *next = child->next;
        layer_destroy(child);
        child = next;
    }
    
    free(l);
}

void layer_set_rect(Layer *l, const SDL_Rect *r) {
    if (!l || !r) return;
    l->rect = *r;
    l->dirty = true;
}

void layer_add_child(Layer *parent, Layer *child) {
    if (!parent || !child) return;
    
    child->parent = parent;
    child->next = parent->children;
    parent->children = child;
}
