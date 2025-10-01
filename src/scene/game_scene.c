#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Données pour la scène de jeu
typedef struct GameSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* sidebar;
    UINode* playable_area;
} GameSceneData;

// Initialisation de la scène de jeu
static void game_scene_init(Scene* scene) {
    printf("🎮 Initialisation de la scène de jeu avec layout sidebar + zone de jeu\n");
    
    // Désactiver la visualisation des hitboxes pour la scène de jeu
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
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Container principal (plein écran pour main window)
    UINode* app = UI_DIV(data->ui_tree, "game-app");
    if (!app) {
        printf("❌ Erreur: Impossible de créer le container principal\n");
        free(data);
        return;
    }
    
    // Taille pour main window (plus grande)
    SET_POS(app, 0, 0);
    SET_SIZE(app, 800, 600);
    
    // Fond neutre pour la scène de jeu
    atomic_set_background_color(app->element, 45, 45, 45, 255); // Gris foncé
    
    // Configuration flexbox horizontale (sidebar à gauche, jeu à droite)
    ui_set_display_flex(app);
    ui_set_flex_direction(app, "row");
    ui_set_justify_content(app, "flex-start");
    ui_set_align_items(app, "stretch"); // Étirer sur toute la hauteur
    
    // === SIDEBAR (1/3 de l'écran) ===
    data->sidebar = UI_SIDEBAR(data->ui_tree, "game-sidebar");
    if (data->sidebar) {
        SET_SIZE(data->sidebar, 266, 600); // 1/3 de 800px = ~266px
        
        // Connecter les callbacks des boutons
        // TODO: Connecter les vrais callbacks quand le système d'événements sera prêt
        
        APPEND(app, data->sidebar);
        printf("📋 Sidebar créée (266x600) avec tous les composants\n");
    }
    
    // === ZONE DE JEU (2/3 de l'écran) ===
    data->playable_area = ui_cnt_playable_with_size(data->ui_tree, "game-playable", 534, 600); // 2/3 de 800px = 534px
    if (data->playable_area) {
        APPEND(app, data->playable_area);
        printf("🎮 Zone de jeu créée (534x600) avec plateau centré\n");
    }
    
    // Construire la hiérarchie
    APPEND(data->ui_tree->root, app);
    
    // Calculer les z-index implicites
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface de jeu créée avec :\n");
    printf("   📏 Layout horizontal: sidebar (1/3) + zone de jeu (2/3)\n");
    printf("   📋 Sidebar complète avec titre, joueurs et contrôles\n");
    printf("   🎮 Zone de jeu avec plateau Fanorona centré\n");
    printf("   🖼️ Optimisé pour main window (800x600)\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène de jeu
static void game_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    GameSceneData* data = (GameSceneData*)scene->data;
    
    // Mettre à jour l'arbre UI
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
    }
    
    // TODO: Mettre à jour la logique de jeu
    // - État du plateau
    // - Temps des joueurs
    // - Animations en cours
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
    
    // Nettoyer l'arbre UI
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène de jeu terminé\n");
}

// Créer la scène de jeu
Scene* create_game_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène de jeu\n");
        return NULL;
    }
    
    scene->id = "game";
    scene->name = "Jeu Fanorona";
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
    
    printf("🎮 Game scene configurée pour MAIN WINDOW (800x600)\n");
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
    
    // Créer un EventManager dédié à la scène
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène de jeu\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène de jeu\n");
            return;
        }
    }
    
    // Connecter l'EventManager à l'UITree
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        
        // Enregistrer tous les éléments UI
        ui_tree_register_all_events(data->ui_tree);
        
        // Stocker l'UITree dans la scène
        scene->ui_tree = data->ui_tree;
        
        printf("🔗 EventManager dédié connecté à la scène de jeu\n");
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Marquer comme initialisé et actif
    scene->initialized = true;
    scene->active = true;
    
    printf("✅ Scène de jeu prête avec son propre système d'événements\n");
}
