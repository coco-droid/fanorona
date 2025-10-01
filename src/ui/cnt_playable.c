#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"
#include "../utils/asset_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === CNT_PLAYABLE COMPONENT ===

UINode* ui_cnt_playable(UITree* tree, const char* id) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    // Créer le container principal de la zone de jeu
    UINode* playable_container = ui_div(tree, id);
    if (!playable_container) {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create playable container");
        return NULL;
    }
    
    // Style de la zone de jeu : fond bleu, occupe 2/3 de l'écran
    atomic_set_background_color(playable_container->element, 70, 130, 180, 255); // Bleu acier
    atomic_set_padding(playable_container->element, 20, 20, 20, 20);
    
    // Configuration flexbox pour centrer le contenu de jeu
    ui_set_display_flex(playable_container);
    ui_set_justify_content(playable_container, "center");
    ui_set_align_items(playable_container, "center");
    
    // === ZONE DE JEU CENTRALE ===
    ui_cnt_playable_add_game_area(playable_container);
    
    ui_log_event("UIComponent", "Create", id, "Playable container created");
    printf("✅ Zone de jeu '%s' créée avec :\n", id);
    printf("   🎮 Fond bleu acier (70, 130, 180)\n");
    printf("   📏 Occupe 2/3 de l'écran\n");
    printf("   🎯 Zone de jeu centrée\n");
    
    return playable_container;
}

void ui_cnt_playable_add_game_area(UINode* playable_container) {
    if (!playable_container) return;
    
    // Container pour la zone de jeu proprement dite
    UINode* game_area = ui_div(playable_container->tree, "game-area");
    if (!game_area) return;
    
    // Taille de la zone de jeu (plateau Fanorona typique)
    SET_SIZE(game_area, 400, 400);
    
    // Style de la zone de jeu : plateau avec bordure
    atomic_set_background_color(game_area->element, 139, 69, 19, 255); // Bois marron
    atomic_set_border(game_area->element, 3, 101, 67, 33, 255); // Bordure bois foncé
    atomic_set_border_radius(game_area->element, 8);
    
    // Configuration pour le plateau de jeu
    ui_set_display_flex(game_area);
    ui_set_justify_content(game_area, "center");
    ui_set_align_items(game_area, "center");
    
    // === GRILLE DE JEU (pour l'instant, juste un indicateur) ===
    UINode* game_grid = ui_div(playable_container->tree, "game-grid");
    if (game_grid) {
        SET_SIZE(game_grid, 360, 360);
        atomic_set_background_color(game_grid->element, 160, 82, 45, 255); // Bois plus clair
        atomic_set_border_radius(game_grid->element, 4);
        
        // Texte temporaire pour indiquer la zone de jeu
        UINode* placeholder_text = ui_text(playable_container->tree, "game-placeholder", "PLATEAU FANORONA\n(Zone de jeu)");
        if (placeholder_text) {
            ui_set_text_color(placeholder_text, "rgb(255, 255, 255)");
            ui_set_text_size(placeholder_text, 18);
            ui_set_text_align(placeholder_text, "center");
            ui_set_text_style(placeholder_text, true, false);
            CENTER(placeholder_text);
            APPEND(game_grid, placeholder_text);
        }
        
        APPEND(game_area, game_grid);
    }
    
    APPEND(playable_container, game_area);
    
    ui_log_event("UIComponent", "GameArea", playable_container->id, "Game area with grid added");
    printf("   🏁 Zone de jeu 400x400 avec plateau bois\n");
    printf("   📋 Grille de jeu 360x360 prête pour le gameplay\n");
}

UINode* ui_cnt_playable_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* playable_container = ui_cnt_playable(tree, id);
    if (playable_container) {
        SET_SIZE(playable_container, width, height);
        ui_log_event("UIComponent", "Style", id, "Playable container size set");
    }
    return playable_container;
}
