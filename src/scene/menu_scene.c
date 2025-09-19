#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/log_console.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Données pour la scène menu
typedef struct MenuSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
} MenuSceneData;

// Initialisation de la scène menu
static void menu_scene_init(Scene* scene) {
    printf("📋 Initialisation de la scène Menu\n");
    
    MenuSceneData* data = (MenuSceneData*)malloc(sizeof(MenuSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour MenuSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Container principal (plein écran) avec fond bleu
    UINode* app = UI_DIV(data->ui_tree, "menu-app");
    if (!app) {
        printf("❌ Erreur: Impossible de créer le container principal\n");
        free(data);
        return;
    }
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    SET_BG(app, "rgb(0, 100, 200)"); // Fond bleu
    
    // Titre en haut
    UINode* title = UI_TEXT(data->ui_tree, "menu-title", "MENU PRINCIPAL");
    if (title) {
        SET_POS(title, 250, 50);
        ui_set_text_color(title, "rgb(255, 255, 255)");
        ui_set_text_size(title, 24);  // 🔧 FIX: ui_set_font_size -> ui_set_text_size
        APPEND(app, title);
    }
    
    // Retour à la page d'accueil
    UINode* back_button = ui_button(data->ui_tree, "back-button", "RETOUR", NULL, NULL);
    if (back_button) {
        SET_POS(back_button, 50, 400);
        SET_SIZE(back_button, 150, 40);
        ui_set_text_color(back_button, "rgb(255, 255, 255)");
        ui_set_background(back_button, "rgb(50, 50, 150)");  // 🔧 FIX: ui_set_background_color -> ui_set_background
        APPEND(app, back_button);
        
        // Ce bouton sera remplacé plus tard par un lien UI
    }
    
    APPEND(data->ui_tree->root, app);
    
    // Calculer les z-index implicites
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Menu créée avec fond bleu\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène menu
static void menu_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // Mettre à jour l'arbre UI
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
    }
}

// Rendu de la scène menu
static void menu_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // Rendre l'arbre UI
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène menu
static void menu_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Menu\n");
    if (!scene || !scene->data) {
        return;
    }
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // Nettoyer l'arbre UI
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Menu terminé\n");
}

// Créer la scène menu - Assurons-nous que cette fonction est bien définie et exportée
Scene* create_menu_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Menu\n");
        return NULL;
    }
    
    scene->id = "menu";
    scene->name = "Menu Principal";
    scene->target_window = WINDOW_TYPE_MAIN;
    scene->event_manager = NULL;
    scene->ui_tree = NULL;
    scene->initialized = false;
    scene->active = false;
    
    scene->init = menu_scene_init;
    scene->update = menu_scene_update;
    scene->render = menu_scene_render;
    scene->cleanup = menu_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

// Assurons-nous que cette fonction est bien définie et exportée  
void menu_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans menu_scene_connect_events\n");
        return;
    }
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    // Obtenir l'EventManager du Core
    extern EventManager* game_core_get_event_manager(GameCore* core);
    EventManager* event_manager = game_core_get_event_manager(core);
    if (!event_manager) {
        printf("❌ Event manager NULL\n");
        return;
    }
    
    // Connecter l'EventManager à l'UITree
    if (data->ui_tree) {
        data->ui_tree->event_manager = event_manager;
        
        // Enregistrer tous les éléments UI
        ui_tree_register_all_events(data->ui_tree);
        
        scene->event_manager = event_manager;
        scene->ui_tree = data->ui_tree;  // 🔧 AJOUTÉ: Associer l'UITree à la scène
        
        printf("🔗 EventManager connecté à la scène menu\n");
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Marquer comme initialisé et actif
    scene->initialized = true;
    scene->active = true;
    
    printf("✅ Scène menu prête pour les transitions\n");
}
