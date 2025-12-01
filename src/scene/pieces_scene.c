#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../config.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PiecesSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* black_button;
    UINode* brown_button;
    UINode* back_link;
    UINode* next_link;
    SDL_Renderer* last_renderer;
} PiecesSceneData;

// 🆕 Callback pour le bouton RETOUR avec réinitialisation
static void on_back_action(void* element, SDL_Event* event) {
    (void)element; (void)event;
    printf("🔙 Retour demandé - Réinitialisation de la configuration\n");
    config_reset_to_default();
    
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* link = (UINode*)atomic->user_data;
    
    if (link) {
        extern void ui_link_activate(UINode* link);
        ui_link_activate(link);
    }
}

static void black_pieces_clicked(void* element, SDL_Event* event) {
    (void)element; (void)event;
    config_set_player1_piece_color(PIECE_COLOR_BLACK);
    config_set_player2_piece_color(PIECE_COLOR_BROWN);
    printf("⚫ Joueur 1 a choisi les pièces NOIRES\n");
}

static void brown_pieces_clicked(void* element, SDL_Event* event) {
    (void)element; (void)event;
    config_set_player1_piece_color(PIECE_COLOR_BROWN);
    config_set_player2_piece_color(PIECE_COLOR_BLACK);
    printf("🟤 Joueur 1 a choisi les pièces BRUNES\n");
}

static void pieces_scene_build_ui(Scene* scene, SDL_Renderer* renderer) {
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    if (!data || !renderer) return;

    // 🆕 FIX: Clear event manager
    if (scene->event_manager) {
        event_manager_clear_all(scene->event_manager);
    }

    if (data->ui_tree) {
        ui_tree_stop_all_animations(data->ui_tree);
        data->ui_tree->event_manager = NULL;
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
        data->black_button = NULL;
        data->brown_button = NULL;
        data->back_link = NULL;
        data->next_link = NULL;
    }

    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    if (scene->event_manager) {
        data->ui_tree->event_manager = scene->event_manager;
    }
    
    SDL_Texture* background_texture = asset_load_texture(renderer, "fix_bg.png");
    
    UINode* app = UI_DIV(data->ui_tree, "pieces-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "pieces-modal", 500, 450);
    
    UINode* content_parent = UI_DIV(data->ui_tree, "pieces-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 10); // 🆕 FIX: Réduit de 20 à 10 pour rapprocher les boutons
    
    UINode* pieces_header = UI_TEXT(data->ui_tree, "pieces-header", "CHOISIR LES PIÈCES");
    ui_set_text_color(pieces_header, "rgb(255, 165, 0)");
    ui_set_text_size(pieces_header, 20);
    ui_set_text_align(pieces_header, "center");
    ui_set_text_style(pieces_header, true, false);
    atomic_set_margin(pieces_header->element, 24, 0, 0, 0);
    
    UINode* pieces_container = UI_DIV(data->ui_tree, "pieces-buttons-container");
    SET_SIZE(pieces_container, 350, 180);
    ui_set_display_flex(pieces_container);
    FLEX_COLUMN(pieces_container);
    ui_set_justify_content(pieces_container, "center");
    ui_set_align_items(pieces_container, "center");
    ui_set_flex_gap(pieces_container, 20);
    
    data->black_button = ui_create_link(data->ui_tree, "black-pieces-link", "PIÈCES NOIRES", NULL, SCENE_TRANSITION_REPLACE);
    if (data->black_button) {
        SET_SIZE(data->black_button, 300, 50);
        ui_set_text_align(data->black_button, "center");
        atomic_set_background_color(data->black_button->element, 30, 30, 30, 200);
        atomic_set_border(data->black_button->element, 2, 100, 100, 100, 255);
        atomic_set_text_color_rgba(data->black_button->element, 255, 255, 255, 255);
        atomic_set_padding(data->black_button->element, 10, 15, 10, 15);
        atomic_set_click_handler(data->black_button->element, black_pieces_clicked);
        ui_animate_slide_in_left(data->black_button, 0.8f, 200.0f);
        APPEND(pieces_container, data->black_button);
    }
    
    data->brown_button = ui_create_link(data->ui_tree, "brown-pieces-link", "PIÈCES BRUNES", NULL, SCENE_TRANSITION_REPLACE);
    if (data->brown_button) {
        SET_SIZE(data->brown_button, 300, 50);
        ui_set_text_align(data->brown_button, "center");
        atomic_set_background_color(data->brown_button->element, 139, 69, 19, 200);
        atomic_set_border(data->brown_button->element, 2, 160, 82, 45, 255);
        atomic_set_text_color_rgba(data->brown_button->element, 255, 255, 255, 255);
        atomic_set_padding(data->brown_button->element, 10, 15, 10, 15);
        atomic_set_click_handler(data->brown_button->element, brown_pieces_clicked);
        ui_animate_slide_in_right(data->brown_button, 1.0f, 200.0f);
        APPEND(pieces_container, data->brown_button);
    }
    
    data->next_link = ui_create_link(data->ui_tree, "next-game-link", "SUIVANT", "game", SCENE_TRANSITION_CLOSE_AND_OPEN);
    if (data->next_link) {
        SET_SIZE(data->next_link, 200, 45);
        ui_set_text_align(data->next_link, "center");
        // 🆕 FIX: Style doré pour bouton Suivant
        atomic_set_background_color(data->next_link->element, 20, 20, 20, 220);
        atomic_set_border(data->next_link->element, 2, 255, 215, 0, 255);
        atomic_set_text_color_rgba(data->next_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->next_link->element, 10, 15, 10, 15);
        ui_link_set_target_window(data->next_link, WINDOW_TYPE_MAIN);
        ui_animate_pulse(data->next_link, 2.0f);
    }
    
    data->back_link = ui_create_link(data->ui_tree, "back-profile-link", "RETOUR", "profile", SCENE_TRANSITION_REPLACE);
    if (data->back_link) {
        SET_SIZE(data->back_link, 150, 35);
        ui_set_text_align(data->back_link, "center");
        // 🆕 FIX: Style doré pour bouton Retour
        atomic_set_background_color(data->back_link->element, 20, 20, 20, 220);
        atomic_set_border(data->back_link->element, 2, 255, 215, 0, 255);
        atomic_set_text_color_rgba(data->back_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->back_link->element, 6, 10, 6, 10);
        ui_link_set_target_window(data->back_link, WINDOW_TYPE_MINI);
        // 🆕 FIX: Callback de reset
        atomic_set_click_handler(data->back_link->element, on_back_action);
    }
    
    UINode* nav_container = UI_DIV(data->ui_tree, "nav-buttons-container");
    SET_SIZE(nav_container, 360, 50); // 🔧 FIX: Élargi pour accommoder le gap (150+200+8 = 358 -> 360)
    ui_set_display_flex(nav_container);
    FLEX_ROW(nav_container);
    ui_set_justify_content(nav_container, "center"); // 🔧 FIX: Centré avec gap explicite
    ui_set_align_items(nav_container, "center");
    ui_set_flex_gap(nav_container, 8); // 🔧 FIX: 8px d'espace horizontal
    
    APPEND(nav_container, data->back_link);
    APPEND(nav_container, data->next_link);
    
    APPEND(content_parent, pieces_header);
    APPEND(content_parent, pieces_container);
    APPEND(content_parent, nav_container);
    
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_animate_fade_in(modal_container, 0.8f);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    if (scene->event_manager) {
        ui_tree_register_all_events(data->ui_tree);
    }
    
    if (data->core) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(data->core);
        if (scene_manager) {
            if (data->black_button) ui_link_connect_to_manager(data->black_button, scene_manager);
            if (data->brown_button) ui_link_connect_to_manager(data->brown_button, scene_manager);
            if (data->back_link) ui_link_connect_to_manager(data->back_link, scene_manager);
            if (data->next_link) {
                GameMode current_mode = config_get_mode();
                if (current_mode == GAME_MODE_VS_AI) {
                    ui_link_set_target(data->next_link, "ai");
                    ui_link_set_transition(data->next_link, SCENE_TRANSITION_REPLACE);
                    ui_link_set_target_window(data->next_link, WINDOW_TYPE_MINI);
                } else {
                    ui_link_set_target(data->next_link, "game");
                    ui_link_set_transition(data->next_link, SCENE_TRANSITION_CLOSE_AND_OPEN);
                    ui_link_set_target_window(data->next_link, WINDOW_TYPE_MAIN);
                }
                ui_link_connect_to_manager(data->next_link, scene_manager);
            }
        }
    }
    
    scene->ui_tree = data->ui_tree;
}

static void pieces_scene_init(Scene* scene) {
    printf("🎨 Initialisation de la scène Choix de Pièces\n");
    
    ui_set_hitbox_visualization(false);
    
    PiecesSceneData* data = (PiecesSceneData*)malloc(sizeof(PiecesSceneData));
    if (!data) return;
    
    data->initialized = true;
    data->core = NULL;
    data->black_button = NULL;
    data->brown_button = NULL;
    data->back_link = NULL;
    data->next_link = NULL;
    data->last_renderer = NULL;
    data->ui_tree = NULL;
    
    scene->data = data;
    
    GameWindow* window = use_mini_window();
    if (window && window->renderer) {
        pieces_scene_build_ui(scene, window->renderer);
        data->last_renderer = window->renderer;
    }
}

static void pieces_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        if (data->black_button) ui_link_update(data->black_button, delta_time);
        if (data->brown_button) ui_link_update(data->brown_button, delta_time);
        if (data->back_link) ui_link_update(data->back_link, delta_time);
        
        // 🔧 FIX: Dynamically update next_link target based on current mode
        // This fixes the bug where it skips AI scene because UI was built before mode selection
        if (data->next_link) {
            GameMode current_mode = config_get_mode();
            
            // 🔧 FIX: Use accessor to get target_scene_id from component data
            UILinkData* link_data = ui_link_get_data(data->next_link);
            const char* current_target = link_data ? link_data->target_scene_id : NULL;
            
            const char* expected_target = (current_mode == GAME_MODE_VS_AI) ? "ai" : "game";
            
            // Only update if target is different or NULL
            if (!current_target || strcmp(current_target, expected_target) != 0) {
                ui_link_set_target(data->next_link, expected_target);
                
                if (current_mode == GAME_MODE_VS_AI) {
                    ui_link_set_transition(data->next_link, SCENE_TRANSITION_REPLACE);
                    ui_link_set_target_window(data->next_link, WINDOW_TYPE_MINI);
                } else {
                    ui_link_set_transition(data->next_link, SCENE_TRANSITION_CLOSE_AND_OPEN);
                    ui_link_set_target_window(data->next_link, WINDOW_TYPE_MAIN);
                }
                // printf("🔄 [PIECES] Link target corrected to '%s' (Mode: %d)\n", expected_target, current_mode);
            }
            
            ui_link_update(data->next_link, delta_time);
        }
    }
}

static void pieces_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    
    if (renderer != data->last_renderer) {
        pieces_scene_build_ui(scene, renderer);
        data->last_renderer = renderer;
    }
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

static void pieces_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Pieces\n");
    if (!scene || !scene->data) return;
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    
    // 🆕 FIX: Clear event manager
    if (scene->event_manager) {
        event_manager_clear_all(scene->event_manager);
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = NULL;
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    scene->initialized = false;
    
    printf("✅ Nettoyage de la scène Pieces terminé\n");
}

Scene* create_pieces_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->id = strdup("pieces");
    scene->name = strdup("Choix des Pièces");
    
    if (!scene->id || !scene->name) {
        if (scene->id) free(scene->id);
        if (scene->name) free(scene->name);
        free(scene);
        return NULL;
    }
    
    scene->target_window = WINDOW_TYPE_MINI;
    scene->event_manager = NULL;
    scene->ui_tree = NULL;
    scene->initialized = false;
    scene->active = false;
    
    scene->init = pieces_scene_init;
    scene->update = pieces_scene_update;
    scene->render = pieces_scene_render;
    scene->cleanup = pieces_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

void pieces_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    if (!data) return;
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    extern SceneManager* game_core_get_scene_manager(GameCore* core);
    SceneManager* scene_manager = game_core_get_scene_manager(core);
    
    if (scene_manager) {
        if (data->black_button) ui_link_connect_to_manager(data->black_button, scene_manager);
        if (data->brown_button) ui_link_connect_to_manager(data->brown_button, scene_manager);
        if (data->back_link) ui_link_connect_to_manager(data->back_link, scene_manager);
        if (data->next_link) ui_link_connect_to_manager(data->next_link, scene_manager);
    }
}
