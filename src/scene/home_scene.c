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

// Callbacks pour les boutons
static void play_button_clicked(UINode* node, void* user_data) {
    (void)node; // Éviter le warning unused parameter
    (void)user_data; // Éviter le warning unused parameter
    printf("🎮 Bouton Play cliqué ! Démarrage du jeu...\n");
    // TODO: Changer vers la scène de jeu
}

static void quit_button_clicked(UINode* node, void* user_data) {
    (void)node; // Éviter le warning unused parameter
    (void)user_data; // Éviter le warning unused parameter
    printf("🚪 Bouton Quit cliqué ! Fermeture du jeu...\n");
    // TODO: Fermer l'application
}

// Initialisation de la scène home
static void home_scene_init(Scene* scene) {
    printf("🏠 Initialisation de la scène Home avec UI DOM-like\n");
    
    // Activer les logs d'événements pour debugging
    ui_set_event_logging(true);
    
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
            data->background_texture = asset_load_texture(renderer, "fix_bg.png");
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
    SET_SIZE(app, 700, 500);
    
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
    ui_set_flex_gap(app, 30); // Réduire le gap pour que les boutons restent à l'écran
    
    // Logo Fanorona au centre (réduire la taille pour laisser place aux boutons)
    UINode* logo = UI_IMAGE(data->ui_tree, "fanorona-logo", data->logo_texture);
    if (!logo) {
        printf("❌ Erreur: Impossible de créer le logo\n");
        return;
    }
    
    SET_SIZE(logo, 400, 200); // Réduire la taille du logo
    
    if (data->logo_texture) {
        printf("🖼️ Logo PNG chargé avec composant image (fond transparent)\n");
        // S'assurer que le logo n'a pas de background par défaut (déjà transparent)
        atomic_set_background_color(logo->element, 0, 0, 0, 0); // Explicitement transparent
    } else {
        // Fallback: créer un texte à la place
        logo = UI_DIV(data->ui_tree, "fanorona-logo-fallback");
        if (logo) {
            SET_SIZE(logo, 400, 100); // Plus petit aussi
            SET_BG(logo, "rgb(255,255,255)");
            UINode* logo_text = UI_TEXT(data->ui_tree, "logo-text", "FANORONA");
            if (logo_text) {
                ui_set_text_align(logo_text, "center");
                ui_set_text_color(logo_text, "rgb(0,0,0)");
                CENTER(logo_text);
                APPEND(logo, logo_text);
            }
            printf("📝 Logo en texte de secours utilisé\n");
        }
    }
    
    // Container pour les boutons
    UINode* button_container = UI_DIV(data->ui_tree, "button-container");
    if (!button_container) {
        printf("❌ Erreur: Impossible de créer le container de boutons\n");
        return;
    }
    
    ui_set_display_flex(button_container);
    FLEX_COLUMN(button_container);
    ui_set_justify_content(button_container, "center");
    ui_set_align_items(button_container, "center");
    ui_set_flex_gap(button_container, 15); // Gap plus petit entre les boutons
    
    // Bouton Play - SANS couleur de fond, avec image PNG
    UINode* play_button = ui_button(data->ui_tree, "play-button", "JOUER", play_button_clicked, NULL);
    if (play_button) {
        SET_SIZE(play_button, 200, 60); // Plus grand pour le PNG
        
        // Configurer l'image de fond avec les propriétés CSS
        ui_button_set_background_image(play_button, "home_bg_btn.png");
        SET_BG_SIZE(play_button, "cover");    // Couvrir tout le bouton
        SET_BG_REPEAT(play_button, "no-repeat"); // Pas de répétition
        
        ui_set_text_color(play_button, "rgb(255, 255, 255)"); // Texte blanc
        ui_button_fix_text_rendering(play_button); // Corriger l'affichage du texte
        
        // Débugger le texte du bouton
        DEBUG_TEXT(play_button);
        
        printf("✅ Bouton Play créé avec background PNG en mode cover\n");
    }
    
    // Bouton Quit - SANS couleur de fond, avec image PNG
    UINode* quit_button = ui_button(data->ui_tree, "quit-button", "QUITTER", quit_button_clicked, NULL);
    if (quit_button) {
        SET_SIZE(quit_button, 200, 60); // Plus grand pour le PNG
        
        // Configurer l'image de fond avec les propriétés CSS
        ui_button_set_background_image(quit_button, "home_bg_btn.png");
        SET_BG_SIZE(quit_button, "cover");    // Couvrir tout le bouton
        SET_BG_REPEAT(quit_button, "no-repeat"); // Pas de répétition
        
        ui_set_text_color(quit_button, "rgb(255, 255, 255)"); // Texte blanc
        ui_button_fix_text_rendering(quit_button); // Corriger l'affichage du texte
        
        // Débugger le texte du bouton
        DEBUG_TEXT(quit_button);
        
        printf("✅ Bouton Quit créé avec background PNG en mode cover\n");
    }
    
    // Construire la hiérarchie de manière sécurisée
    if (data->ui_tree && data->ui_tree->root) {
        APPEND(data->ui_tree->root, app);
        APPEND(app, logo);
        APPEND(app, button_container);
        
        // Ajouter les boutons au container
        if (play_button) APPEND(button_container, play_button);
        if (quit_button) APPEND(button_container, quit_button);
        
        // Calculer les z-index implicites après avoir construit la hiérarchie
        ui_calculate_implicit_z_index(data->ui_tree);
        
    } else {
        printf("❌ Erreur: Arbre UI ou racine non initialisé\n");
        return;
    }
    
    printf("✅ Interface Home créée avec :\n");
    printf("   🖼️  Logo Fanorona centré (taille réduite)\n");
    printf("   🎮  Bouton Play (avec PNG background)\n");
    printf("   🚪  Bouton Quit (avec PNG background)\n");
    printf("   📊  Z-index calculés automatiquement\n");
    printf("   🔍  Logs d'événements activés\n");
    
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
    if (!scene || !scene->data) {
        printf("⚠️ Scene ou scene->data est NULL, nettoyage ignoré\n");
        return;
    }
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    
    // Vérifier que les données sont valides
    if (!data->initialized) {
        printf("⚠️ HomeSceneData non initialisé, nettoyage partiel\n");
        free(scene->data);
        scene->data = NULL;
        return;
    }
    
    // Libérer les textures
    if (data->background_texture) {
        SDL_DestroyTexture(data->background_texture);
        data->background_texture = NULL;
        printf("✅ Background texture libérée\n");
    }
    if (data->logo_texture) {
        SDL_DestroyTexture(data->logo_texture);
        data->logo_texture = NULL;
        printf("✅ Logo texture libérée\n");
    }
    
    // Nettoyer l'arbre UI
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
        printf("✅ UI tree détruit\n");
    }
    
    // Nettoyer les polices TTF
    ui_cleanup_fonts();
    
    // Réinitialiser l'arbre global
    ui_set_global_tree(NULL);
    
    // Marquer comme non initialisé avant de libérer
    data->initialized = false;
    
    // Libérer les données de la scène
    free(scene->data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Home terminé\n");
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