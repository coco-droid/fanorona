#include "scene.h"
#include <stdlib.h>

static void menu_init(Scene *s) {
    s->lm = lm_create();
}

static void menu_layout(Scene *s, int w, int h) {
    (void)s; (void)w; (void)h; // Suppress unused warnings
    // Menu layout implementation
}

static void menu_cleanup(Scene *s) {
    if (s->lm) {
        lm_destroy(s->lm);
    }
    free(s);
}

Scene *menu_scene_create(void) {
    Scene *s = malloc(sizeof(Scene));
    s->lm = NULL;
    s->board_layer = NULL;
    s->init = menu_init;
    s->layout = menu_layout;
    s->cleanup = menu_cleanup;
    return s;
}
