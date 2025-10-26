#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../types.h"           // 🔧 FIX: Import types
#include "../config.h"          // 🔧 FIX: Import config
#include "../pions/pions.h"     // 🔧 FIX: Import pions
#include "../logic/logic.h"     // 🔧 FIX: Import logic
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène de jeu
typedef struct GameSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* sidebar;
    UINode* playable_area;
    GameLogic* game_logic;       // 🆕 Ajout de la logique de jeu
} GameSceneData;

// Initialisation de la scène de jeu
static void game_scene_init(Scene* scene) {
    printf("🎮 Initialisation de la scène de jeu avec layout sidebar + zone de jeu\n");
    
    ui_set_hitbox_visualization(false);
    printf("🚫 Visualisation des hitboxes DÉSACTIVÉE pour la scène de jeu\n");
    
    GameSceneData* data = (GameSceneData*)malloc(sizeof(GameSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour GameSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->sidebar = NULL;
    data->playable_area = NULL;
    data->game_logic = NULL;
    
    // 🆕 Créer la logique de jeu AVANT l'UI
    data->game_logic = game_logic_create();
    if (data->game_logic) {
        game_logic_start_new_game(data->game_logic);
        printf("✅ GameLogic initialisée en mode: %s\n", config_mode_to_string(config_get_mode()));
        game_logic_debug_print(data->game_logic);
    }
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // 🔧 FIX: Récupérer les dimensions depuis la main window
    int app_width = DEFAULT_MAIN_WINDOW_WIDTH;
    int app_height = DEFAULT_MAIN_WINDOW_HEIGHT;
    
    // Container principal (dimensions de main window)
    UINode* app = UI_DIV(data->ui_tree, "game-app");
    if (!app) {
        printf("❌ Erreur: Impossible de créer le container principal\n");
        free(data);
        return;
    }
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, app_width, app_height);
    
    // Fond neutre pour la scène de jeu
    atomic_set_background_color(app->element, 45, 45, 45, 255);
    
    // Configuration flexbox horizontale
    ui_set_display_flex(app);
    ui_set_flex_direction(app, "row");
    ui_set_justify_content(app, "flex-start");
    ui_set_align_items(app, "stretch");
    
    // === SIDEBAR (1/3 exact) ===
    int sidebar_width = app_width / 3;
    data->sidebar = UI_SIDEBAR(data->ui_tree, "game-sidebar");
    if (data->sidebar) {
        SET_SIZE(data->sidebar, sidebar_width, app_height);
        ui_set_z_index(data->sidebar, 10);
        ui_animate_slide_in_left(data->sidebar, 0.6f, 300.0f);
        
        // 🔧 FIX: Ajouter les joueurs DIRECTEMENT avec les vraies données
        if (data->game_logic && data->game_logic->player1 && data->game_logic->player2) {
            ui_sidebar_add_player_containers(
                data->sidebar, 
                data->game_logic->player1, 
                data->game_logic->player2
            );
            printf("✅ Joueurs ajoutés à la sidebar avec données RÉELLES de GameLogic\n");
        } else {
            printf("❌ GameLogic ou joueurs non initialisés\n");
        }
        
        APPEND(app, data->sidebar);
        printf("📋 Sidebar créée (%dx%d) avec joueurs réels\n", sidebar_width, app_height);
    }
    
    // === ZONE DE JEU (2/3 exact) ===
    int playable_width = (app_width * 2) / 3;
    data->playable_area = ui_cnt_playable_with_size(data->ui_tree, "game-playable", playable_width, app_height);
    if (data->playable_area) {
        ui_set_z_index(data->playable_area, 5);  // 🔧 FIX: Lower z-index than sidebar
        ui_animate_fade_in(data->playable_area, 1.2f);
        
        UINode* plateau = ui_tree_find_node(data->ui_tree, "fanorona-plateau");
        if (plateau && data->game_logic) {
            ui_plateau_set_game_logic(plateau, data->game_logic);
            ui_plateau_set_players(plateau, data->game_logic->player1, data->game_logic->player2);
        }
        
        APPEND(app, data->playable_area);
        printf("🎮 Zone de jeu créée (%dx%d) sans overlap sidebar\n", playable_width, app_height);
    }
    
    APPEND(data->ui_tree->root, app);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface de jeu créée avec dimensions depuis config.h:\n");
    printf("   📏 Layout horizontal: sidebar (%d/%d) + zone de jeu (%d/%d)\n", 
           230, app_width, playable_width, app_width);
    printf("   🖼️ Optimisé pour main window (%dx%d)\n", app_width, app_height);
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène de jeu
static void game_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    GameSceneData* data = (GameSceneData*)scene->data;
    
    // 🆕 Mettre à jour la logique de jeu
    if (data->game_logic) {
        game_logic_update(data->game_logic, delta_time);
    }
    
    // 🆕 AJOUT: Mettre à jour les animations (inclut les animations de pièces)
    ui_update_animations(delta_time);
    
    // Mettre à jour l'arbre UI avec effets de scale
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        // 🆕 Les animations de pièces sont automatiquement mises à jour via ui_update_animations()
        // Les effets incluent :
        // - Mouvements de pièces avec transitions fluides
        // - Animations de capture avec fade-out
        // - Pulsations de sélection
        // - Animations de victoire/défaite
    }
    
    // TODO: Mettre à jour la logique de jeu
    // - État du plateau avec interactions visuelles animées
    // - Temps des joueurs
    // - Détection de fin de partie avec animations correspondantes
}

// Rendu de la scène de jeu
static void game_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    GameSceneData* data = (GameSceneData*)scene->data;
    
    // Rendre l'arbre UI
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène de jeu
static void game_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène de jeu\n");
    if (!scene || !scene->data) {
        return;
    }
    
    GameSceneData* data = (GameSceneData*)scene->data;
    
    // 🆕 Nettoyer la logique de jeu AVANT le plateau
    if (data->game_logic) {
        printf("🗑️ [GAME_CLEANUP] Nettoyage GameLogic\n");
        game_logic_destroy(data->game_logic);
        data->game_logic = NULL;
    }
    
    // 🔧 FIX: Nettoyer explicitement le plateau avant de détruire l'UI tree
    if (data->playable_area) {
        // Chercher le plateau dans la zone de jeu et le nettoyer
        // Le plateau sera dans game-area -> fanorona-plateau
        UINode* plateau = ui_tree_find_node(data->ui_tree, "fanorona-plateau");
        if (plateau) {
            printf("🗑️ [GAME_CLEANUP] Nettoyage explicite du plateau\n");
            ui_plateau_container_destroy(plateau);
        }
    }
    
    // Nettoyer l'arbre UI
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène de jeu terminé avec nettoyage du plateau\n");
}

// Créer la scène de jeu
Scene* create_game_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène de jeu\n");
        return NULL;
    }
    
    // 🔧 FIX: Use strdup() instead of string literals
    scene->id = strdup("game");
    scene->name = strdup("Jeu Fanorona");
    
    // 🔧 FIX: Check if strdup() succeeded
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène de jeu\n");
        if (scene->id) free(scene->id);
        if (scene->name) free(scene->name);
        free(scene);
        return NULL;
    }
    
    scene->target_window = WINDOW_TYPE_MAIN; // ✅ CONFIRMÉ: Utilise la main window (800x600)
    scene->event_manager = NULL;
    scene->ui_tree = NULL;
    scene->initialized = false;
    scene->active = false;
    
    scene->init = game_scene_init;
    scene->update = game_scene_update;
    scene->render = game_scene_render;
    scene->cleanup = game_scene_cleanup;
    scene->data = NULL;
    
    printf("🎮 Game scene created with proper memory allocation\n");
    printf("   📏 Layout: sidebar (266px) + zone de jeu (534px)\n");
    printf("   🎯 Prête pour transition depuis menu_scene\n");
    
    return scene;
}

// Connexion des événements pour la scène de jeu
void game_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans game_scene_connect_events\n");
        return;
    }
    
    GameSceneData* data = (GameSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    // 🔧 FIX: Vérifier que la scène est initialisée
    if (!scene->initialized || !data->ui_tree) {
        printf("❌ Scène game non initialisée correctement\n");
        return;
    }
    
    // Créer un EventManager dédié à la scène
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène de jeu\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène de jeu\n");
            return;
        }
    }
    
    // 🔧 FIX CRITIQUE: Connecter l'EventManager à l'UITree AVANT l'enregistrement
    data->ui_tree->event_manager = scene->event_manager;
    
    // 🔧 FIX: Enregistrer tous les éléments UI avec l'EventManager de la scène
    printf("🔧 Enregistrement des éléments UI avec l'EventManager...\n");
    ui_tree_register_all_events(data->ui_tree);
    printf("✅ Éléments UI enregistrés\n");
    
    // Stocker l'UITree dans la scène
    scene->ui_tree = data->ui_tree;
    
    // Stocker la référence du core
    data->core = core;
    
    printf("✅ Scène de jeu prête avec éléments connectés\n");
}
