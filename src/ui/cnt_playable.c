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
    SET_SIZE(game_area, 500, 350);
    
    // Style de la zone de jeu : plateau avec bordure
    atomic_set_background_color(game_area->element, 139, 69, 19, 255); // Bois marron
    atomic_set_border(game_area->element, 3, 101, 67, 33, 255); // Bordure bois foncé
    atomic_set_border_radius(game_area->element, 8);
    atomic_set_padding(game_area->element, 10, 10, 10, 10);
    
    // Configuration pour le plateau de jeu
    ui_set_display_flex(game_area);
    ui_set_justify_content(game_area, "center");
    ui_set_align_items(game_area, "center");
    
    // === PLATEAU FANORONA AVEC JOUEURS ===
    UINode* plateau = ui_plateau_container_with_players(playable_container->tree, "fanorona-plateau", NULL, NULL);
    if (plateau) {
        SET_SIZE(plateau, 480, 320);
        
        // Le plateau sera automatiquement centré dans game_area grâce au flexbox
        APPEND(game_area, plateau);
        
        printf("   🎯 Plateau Fanorona intégré (480x320) avec :\n");
        printf("      • Damier 5x9 intersections\n");
        printf("      • Joueurs de test avec textures\n");
        printf("      • piece_black.png et piece_brun.png\n");
        printf("      • Disposition initiale des pions\n");
        printf("      • Rendu personnalisé avec fond mat\n");
    }
    
    APPEND(playable_container, game_area);
    
    ui_log_event("UIComponent", "GameArea", playable_container->id, "Game area with textured Fanorona plateau added");
    printf("   🏁 Zone de jeu 500x350 avec plateau Fanorona et textures\n");
}

UINode* ui_cnt_playable_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* playable_container = ui_cnt_playable(tree, id);
    if (playable_container) {
        SET_SIZE(playable_container, width, height);
        ui_log_event("UIComponent", "Style", id, "Playable container size set");
    }
    return playable_container;
}
