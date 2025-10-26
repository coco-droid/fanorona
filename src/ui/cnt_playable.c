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

// 🔧 FIX: Define expected playable area dimensions
#define CNT_PLAYABLE_WIDTH 550
#define CNT_PLAYABLE_HEIGHT 400

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
    
    // 🔧 FIX: Set explicit dimensions instead of relying on parent
    SET_SIZE(playable_container, CNT_PLAYABLE_WIDTH, CNT_PLAYABLE_HEIGHT);
    atomic_set_padding(playable_container->element, 0, 0, 0, 0);
    
    // 🔧 FIX: board.png en background qui s'adapte à la taille du container
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
        atomic_set_background_size_str(playable_container->element, "stretch");
        atomic_set_background_repeat_str(playable_container->element, "no-repeat");
    } else {
        atomic_set_background_color(playable_container->element, 70, 130, 180, 255);
    }
    
    ui_set_display_flex(playable_container);
    ui_set_justify_content(playable_container, "center");
    ui_set_align_items(playable_container, "center");
    
    ui_cnt_playable_add_game_area(playable_container);
    
    ui_log_event("UIComponent", "Create", id, "Playable container with board.png (no padding)");
    return playable_container;
}

void ui_cnt_playable_add_game_area(UINode* playable_container) {
    if (!playable_container) return;
    
    // 🔧 FIX: Use container's actual dimensions
    UINode* game_area = ui_div(playable_container->tree, "game-area");
    if (!game_area) return;
    
    // 🔧 FIX: Use 90% of the now-defined dimensions
    int game_w = (CNT_PLAYABLE_WIDTH * 90) / 100;
    int game_h = (CNT_PLAYABLE_HEIGHT * 90) / 100;
    SET_SIZE(game_area, game_w, game_h);
    
    atomic_set_background_color(game_area->element, 0, 0, 0, 0);
    atomic_set_padding(game_area->element, 5, 5, 5, 5);
    
    ui_set_display_flex(game_area);
    ui_set_justify_content(game_area, "center");
    ui_set_align_items(game_area, "center");
    
    // 🔧 FIX: Plateau uses its own constants (480x320)
    UINode* plateau = ui_plateau_container_with_players(playable_container->tree, "fanorona-plateau", NULL, NULL);
    if (plateau) {
        // Plateau size already set in ui_plateau_container_with_players
        APPEND(game_area, plateau);
    }
    
    APPEND(playable_container, game_area);
    
    ui_log_event("UIComponent", "GameArea", playable_container->id, 
                 "Game area with fixed dimensions");
}

// 🔧 FIX: Cette fonction force maintenant les bonnes dimensions
UINode* ui_cnt_playable_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* playable_container = ui_cnt_playable(tree, id);
    if (playable_container) {
        SET_SIZE(playable_container, width, height);
        
        // 🔧 FIX: Recalculer game_area avec nouvelles dimensions
        UINode* game_area = ui_tree_find_node(tree, "game-area");
        if (game_area) {
            int game_w = (width * 90) / 100;
            int game_h = (height * 90) / 100;
            SET_SIZE(game_area, game_w, game_h);
            
            // 🔧 FIX: NE PLUS FORCER les dimensions du plateau
            // Le plateau conserve ses dimensions définies dans plateau_cnt.c (480x150)
        }
        
        ui_log_event("UIComponent", "Style", id, 
                     "Playable container resized, plateau keeps its own size");
    }
    return playable_container;
}
