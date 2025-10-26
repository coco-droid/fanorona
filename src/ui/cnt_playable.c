#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"
#include "../utils/asset_manager.h"
#include "../plateau/plateau.h"
#include "../types.h"           // 🔧 FIX: Import types
#include "../config.h"          // 🔧 FIX: Import config
#include "../pions/pions.h"     // 🔧 FIX: Import pions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === CNT_PLAYABLE COMPONENT ===

UINode* ui_cnt_playable(UITree* tree, const char* id) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    UINode* playable_container = ui_div(tree, id);
    if (!playable_container) {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create playable container");
        return NULL;
    }
    
    // 🔧 FIX: board.png comme background principal (remplace le bleu)
    SDL_Texture* board_texture = NULL;
    GameWindow* window = use_main_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            board_texture = asset_load_texture(renderer, "board.png");
        }
    }
    
    if (board_texture) {
        atomic_set_background_image(playable_container->element, board_texture);
        atomic_set_background_size_str(playable_container->element, "cover");
    } else {
        // Fallback bleu si board.png manquant
        atomic_set_background_color(playable_container->element, 70, 130, 180, 255);
    }
    
    atomic_set_padding(playable_container->element, 20, 20, 20, 20);
    
    ui_set_display_flex(playable_container);
    ui_set_justify_content(playable_container, "center");
    ui_set_align_items(playable_container, "center");
    
    ui_cnt_playable_add_game_area(playable_container);
    
    ui_log_event("UIComponent", "Create", id, "Playable container with board.png background");
    return playable_container;
}

void ui_cnt_playable_add_game_area(UINode* playable_container) {
    if (!playable_container) return;
    
    UINode* game_area = ui_div(playable_container->tree, "game-area");
    if (!game_area) return;
    
    SET_SIZE(game_area, 500, 350);
    
    // 🔧 FIX: Plateau sans background (transparent)
    atomic_set_background_color(game_area->element, 0, 0, 0, 0); // Transparent
    atomic_set_padding(game_area->element, 10, 10, 10, 10);
    
    ui_set_display_flex(game_area);
    ui_set_justify_content(game_area, "center");
    ui_set_align_items(game_area, "center");
    
    // Plateau Fanorona
    UINode* plateau = ui_plateau_container_with_players(playable_container->tree, "fanorona-plateau", NULL, NULL);
    if (plateau) {
        SET_SIZE(plateau, 480, 320);
        // 🔧 FIX: Plateau transparent pour voir board.png à travers
        atomic_set_background_color(plateau->element, 0, 0, 0, 0);
        APPEND(game_area, plateau);
    }
    
    APPEND(playable_container, game_area);
    
    ui_log_event("UIComponent", "GameArea", playable_container->id, "Transparent game area over board.png");
}

UINode* ui_cnt_playable_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* playable_container = ui_cnt_playable(tree, id);
    if (playable_container) {
        SET_SIZE(playable_container, width, height);
        ui_log_event("UIComponent", "Style", id, "Playable container size customized");
    }
    return playable_container;
}
