#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Données pour la scène home
typedef struct HomeSceneData {
    bool initialized;
    UITree* ui_tree;
    SDL_Texture* background_texture;
    SDL_Texture* logo_texture;
} HomeSceneData;

// Callbacks pour les boutons - supprimés car pas de boutons
// static void play_button_clicked(UINode* node, void* user_data) { ... }
// static void quit_button_clicked(UINode* node, void* user_data) { ... }

// Initialisation de la scène home
static void home_scene_init(Scene* scene) {
    printf("🏠 Initialisation de la scène Home avec UI DOM-like\n");
    
    HomeSceneData* data = (HomeSceneData*)malloc(sizeof(HomeSceneData));
    if (!data) {
        printf("Erreur: Impossible d'allouer la mémoire pour HomeSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->background_texture = NULL;
    data->logo_texture = NULL;
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Charger les assets
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            // Charger les textures avec des chemins complets
            data->background_texture = asset_load_texture(renderer, "home_bg.jpeg");
            data->logo_texture = asset_load_texture(renderer, "fanorona_text.png");
            
            printf("🔍 Chargement des assets :\n");
            printf("   Background: %s\n", data->background_texture ? "✅ OK" : "❌ ÉCHEC");
            printf("   Logo: %s\n", data->logo_texture ? "✅ OK" : "❌ ÉCHEC");
        }
    }
    
    // === CRÉATION DE L'INTERFACE ===
    
    // Container principal (plein écran)
    UINode* app = UI_DIV(data->ui_tree, "home-app");
    if (!app) {
        printf("❌ Erreur: Impossible de créer le container principal\n");
        return;
    }
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, 600, 500);
    
    // Définir l'image de fond du container principal
    if (data->background_texture) {
        atomic_set_background_image(app->element, data->background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)"); // Bleu ciel par défaut
    }
    
    // Container principal en flexbox column pour organiser verticalement
    ui_set_display_flex(app);
    FLEX_COLUMN(app);
    ui_set_justify_content(app, "center");
    ui_set_align_items(app, "center");
    ui_set_flex_gap(app, 40);
    
    // Logo Fanorona au centre
    UINode* logo = UI_DIV(data->ui_tree, "fanorona-logo");
    if (!logo) {
        printf("❌ Erreur: Impossible de créer le logo\n");
        return;
    }
    
    SET_SIZE(logo, 400, 100); // Taille pour le logo
    
    if (data->logo_texture) {
        atomic_set_background_image(logo->element, data->logo_texture);
    } else {
        // Fallback: dessiner "FANORONA" en style simple
        SET_BG(logo, "rgb(255,255,255)");
        UINode* logo_text = UI_TEXT(data->ui_tree, "logo-text", "FANORONA");
        if (logo_text) {
            ui_set_text_align(logo_text, "center");
            ui_set_text_color(logo_text, "rgb(0,0,0)");
            CENTER(logo_text);
            APPEND(logo, logo_text);
        }
    }
    
    // Construire la hiérarchie de manière sécurisée
    if (data->ui_tree && data->ui_tree->root) {
        APPEND(data->ui_tree->root, app);
        APPEND(app, logo);
    } else {
        printf("❌ Erreur: Arbre UI ou racine non initialisé\n");
        return;
    }
    
    printf("✅ Interface Home créée avec :\n");
    printf("   🖼️  Logo Fanorona centré\n");
    printf("    Images de fond : %s\n", 
           data->background_texture && data->logo_texture ? "Chargées" : "Partiellement chargées");
    
    scene->data = data;
}

// Mise à jour de la scène home
static void home_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    
    // Mettre à jour l'arbre UI
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
    }
    
    // NOTE: Les événements SDL sont maintenant gérés dans la boucle principale
    // Ne pas faire SDL_PollEvent ici pour éviter les conflits
}

// Rendu de la scène home
static void home_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    
    // Clear avec une couleur de fond par défaut
    SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255); // Bleu ciel
    SDL_RenderClear(renderer);
    
    // Rendre l'arbre UI (qui inclut le background et tous les éléments)
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
    
    // IMPORTANT: Présenter le rendu à l'écran
    SDL_RenderPresent(renderer);
}

// Nettoyage de la scène home
static void home_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Home\n");
    if (scene->data) {
        HomeSceneData* data = (HomeSceneData*)scene->data;
        
        // Libérer les textures
        if (data->background_texture) {
            SDL_DestroyTexture(data->background_texture);
            data->background_texture = NULL;
        }
        if (data->logo_texture) {
            SDL_DestroyTexture(data->logo_texture);
            data->logo_texture = NULL;
        }
        
        // Nettoyer l'arbre UI
        if (data->ui_tree) {
            ui_tree_destroy(data->ui_tree);
        }
        
        // Réinitialiser l'arbre global
        ui_set_global_tree(NULL);
        
        free(scene->data);
        scene->data = NULL;
    }
}

// Créer la scène home
Scene* create_home_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("Erreur: Impossible d'allouer la mémoire pour la scène Home\n");
        return NULL;
    }
    
    scene->name = "Home";
    scene->init = home_scene_init;
    scene->update = home_scene_update;
    scene->render = home_scene_render;
    scene->cleanup = home_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}