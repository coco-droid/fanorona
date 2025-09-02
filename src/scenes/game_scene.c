#include "scene.h"
#include "../layer/layer.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

static void game_init(Scene *s) {
    s->lm = lm_create();
    s->board_layer = layer_create();
    layer_add_child(s->lm->root, s->board_layer);
}

static void game_layout(Scene *s, int w, int h) {
    SDL_Rect board = { .x = w/2-256, .y = h/2-128, .w = 512, .h = 256 };
    layer_set_rect(s->board_layer, &board);
}

static void game_cleanup(Scene *s) {
    if (s->lm) {
        lm_destroy(s->lm);
    }
    free(s);
}

Scene *game_scene_create(void) {
    Scene *s = malloc(sizeof(Scene));
    s->lm = NULL;
    s->board_layer = NULL;
    s->init = game_init;
    s->layout = game_layout;
    s->cleanup = game_cleanup;
    return s;
}