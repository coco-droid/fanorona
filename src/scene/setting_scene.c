#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../ui/native/atomic.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SettingSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* back_link;
} SettingSceneData;

static void setting_scene_init(Scene* scene) {
    printf("📋 Initialisation de la scène Settings\n");
    
    SettingSceneData* data = (SettingSceneData*)malloc(sizeof(SettingSceneData));
    if (!data) return;
    
    data->initialized = true;
    data->core = NULL;
    data->back_link = NULL;
    
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Background
    SDL_Texture* background_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
        }
    }
    
    UINode* app = UI_DIV(data->ui_tree, "setting-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container principal SANS la barre inférieure (cog/pause)
    UINode* modal_container = ui_container_extended(data->ui_tree, "setting-modal", false);
    if (modal_container) {
        SET_SIZE(modal_container, 500, 450);
        ALIGN_SELF_BOTH(modal_container); // Centrage
        
        // Contenu des paramètres
        UINode* content = UI_DIV(data->ui_tree, "setting-content");
        SET_SIZE(content, 400, 250);
        ui_set_display_flex(content);
        FLEX_COLUMN(content);
        ui_set_justify_content(content, "center");
        ui_set_align_items(content, "center");
        ui_set_flex_gap(content, 20);
        
        // Volume
        UINode* vol_row = UI_DIV(data->ui_tree, "vol-row");
        SET_SIZE(vol_row, 300, 40);
        ui_set_display_flex(vol_row);
        FLEX_ROW(vol_row);
        ui_set_justify_content(vol_row, "space-between");
        ui_set_align_items(vol_row, "center");
        
        UINode* vol_label = UI_TEXT(data->ui_tree, "vol-label", "VOLUME");
        ui_set_text_color(vol_label, "white");
        APPEND(vol_row, vol_label);
        
        UINode* vol_val = UI_TEXT(data->ui_tree, "vol-val", "100%");
        ui_set_text_color(vol_val, "orange");
        APPEND(vol_row, vol_val);
        APPEND(content, vol_row);
        
        // SFX
        UINode* sfx_row = UI_DIV(data->ui_tree, "sfx-row");
        SET_SIZE(sfx_row, 300, 40);
        ui_set_display_flex(sfx_row);
        FLEX_ROW(sfx_row);
        ui_set_justify_content(sfx_row, "space-between");
        ui_set_align_items(sfx_row, "center");
        
        UINode* sfx_label = UI_TEXT(data->ui_tree, "sfx-label", "EFFETS SONORES");
        ui_set_text_color(sfx_label, "white");
        APPEND(sfx_row, sfx_label);
        
        UINode* sfx_val = UI_TEXT(data->ui_tree, "sfx-val", "ON");
        ui_set_text_color(sfx_val, "green");
        APPEND(sfx_row, sfx_val);
        APPEND(content, sfx_row);
        
        // FPS
        UINode* fps_row = UI_DIV(data->ui_tree, "fps-row");
        SET_SIZE(fps_row, 300, 40);
        ui_set_display_flex(fps_row);
        FLEX_ROW(fps_row);
        ui_set_justify_content(fps_row, "space-between");
        ui_set_align_items(fps_row, "center");
        
        UINode* fps_label = UI_TEXT(data->ui_tree, "fps-label", "FPS");
        ui_set_text_color(fps_label, "white");
        APPEND(fps_row, fps_label);
        
        UINode* fps_val = UI_TEXT(data->ui_tree, "fps-val", "60");
        ui_set_text_color(fps_val, "cyan");
        APPEND(fps_row, fps_val);
        APPEND(content, fps_row);
        
        // Bouton Retour
        data->back_link = ui_create_link(data->ui_tree, "back-link", "RETOUR", "menu", SCENE_TRANSITION_REPLACE);
        if (data->back_link) {
            SET_SIZE(data->back_link, 200, 40);
            ui_set_text_align(data->back_link, "center");
            atomic_set_background_color(data->back_link->element, 100, 100, 100, 200);
            atomic_set_border(data->back_link->element, 1, 200, 200, 200, 255);
            atomic_set_text_color_rgba(data->back_link->element, 255, 255, 255, 255);
            ui_link_set_target_window(data->back_link, WINDOW_TYPE_MINI);
            APPEND(content, data->back_link);
        }
        
        ui_container_add_content(modal_container, content);
        APPEND(app, modal_container);
    }
    
    APPEND(data->ui_tree->root, app);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

static void setting_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    SettingSceneData* data = (SettingSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        if (data->back_link) ui_link_update(data->back_link, delta_time);
    }
}

static void setting_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    SettingSceneData* data = (SettingSceneData*)scene->data;
    if (data->ui_tree) ui_tree_render(data->ui_tree, window_get_renderer(window));
}

static void setting_scene_cleanup(Scene* scene) {
    if (!scene || !scene->data) return;
    SettingSceneData* data = (SettingSceneData*)scene->data;
    if (data->ui_tree) ui_tree_destroy(data->ui_tree);
    free(data);
    scene->data = NULL;
}

Scene* create_setting_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->id = strdup("setting");
    scene->name = strdup("Paramètres");
    scene->target_window = WINDOW_TYPE_MINI;
    scene->event_manager = NULL;
    scene->ui_tree = NULL;
    scene->initialized = false;
    scene->active = false;
    
    scene->init = setting_scene_init;
    scene->update = setting_scene_update;
    scene->render = setting_scene_render;
    scene->cleanup = setting_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

void setting_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    SettingSceneData* data = (SettingSceneData*)scene->data;
    if (!data) return;
    
    if (!scene->event_manager) scene->event_manager = event_manager_create();
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    if (data->back_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        if (scene_manager) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
        }
    }
}
