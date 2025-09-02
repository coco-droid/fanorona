#pragma once
#include "../layer/layer_manager.h"

typedef struct Scene Scene;
struct Scene {
    LayerManager *lm;
    Layer *board_layer;
    void (*init)(Scene *s);
    void (*layout)(Scene *s, int w, int h);
    void (*cleanup)(Scene *s);
};

Scene *game_scene_create(void);
Scene *menu_scene_create(void);