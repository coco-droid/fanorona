#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/asset_manager.h"
#include "../utils/log_console.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declaration pour éviter l'include circulaire
typedef struct GameCore GameCore;
extern EventManager* game_core_get_event_manager(GameCore* core);

// Données pour la scène home
typedef struct HomeSceneData {
    bool initialized;
    UITree* ui_tree;
    SDL_Texture* background_texture;
    SDL_Texture* logo_texture;
    GameCore* core;
    UINode* play_button;
    UINode* quit_button;
} HomeSceneData;

// 🆕 Callbacks simplifiés sans dépendances
static void play_button_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🎯 FEEDBACK VISUEL SIMPLIFIÉ
    atomic_set_background_color(atomic_element, 100, 200, 100, 255);
    atomic_set_text_color_rgba(atomic_element, 0, 0, 0, 255);
    
    // 🔧 FIX: Utiliser les valeurs directes du style au lieu des fonctions
    int current_width = atomic_element->style.width;
    int current_height = atomic_element->style.height;
    atomic_set_size(atomic_element, current_width - 4, current_height - 2);
    
    // 🔧 LOG SIMPLE
    printf("🎮 Play button clicked with visual feedback\n");
    
    (void)event;
}

static void quit_button_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🎯 FEEDBACK VISUEL SIMPLIFIÉ
    atomic_set_background_color(atomic_element, 220, 100, 100, 255);
    atomic_set_text_color_rgba(atomic_element, 255, 255, 255, 255);
    
    // 🔧 FIX: Utiliser les valeurs directes du style
    int current_width = atomic_element->style.width;
    int current_height = atomic_element->style.height;
    atomic_set_size(atomic_element, current_width - 4, current_height - 2);
    
    // 🔧 LOG SIMPLE
    printf("🚪 Quit button clicked with visual feedback\n");
    
    (void)event;
}

// 🆕 Callback pour hover simplifié
static void button_hovered(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🎯 FEEDBACK SILENCIEUX
    atomic_set_background_color(atomic_element, 255, 255, 255, 50);
    
    // 🔧 FIX: Utiliser les valeurs directes du style
    int current_width = atomic_element->style.width;
    int current_height = atomic_element->style.height;
    atomic_set_size(atomic_element, current_width + 2, current_height + 1);
    
    (void)event;
}

// 🆕 Callback pour unhover simplifié
static void button_unhovered(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🎯 RESTAURATION SILENCIEUSE
    atomic_set_background_color(atomic_element, 0, 0, 0, 0);
    
    // 🔧 FIX: Utiliser les valeurs directes du style
    int current_width = atomic_element->style.width;
    int current_height = atomic_element->style.height;
    atomic_set_size(atomic_element, current_width - 2, current_height - 1);
    
    atomic_set_text_color_rgba(atomic_element, 255, 255, 255, 255);
    
    (void)event;
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
    data->core = NULL; // 🆕 Sera défini plus tard
    data->play_button = NULL; // 🆕 Initialiser à NULL
    data->quit_button = NULL; // 🆕 Initialiser à NULL
    
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
    
    SET_SIZE(logo, 400, 150); // Réduire la taille du logo
    
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
    
    // Bouton Play - AVEC ÉVÉNEMENTS CONNECTÉS
    UINode* play_button = ui_button(data->ui_tree, "play-button", "JOUER", NULL, NULL);
    data->play_button = play_button; // 🆕 Sauvegarder la référence
    if (play_button) {
        SET_SIZE(play_button, 150, 40);
        
        // Configuration visuelle
        ui_button_set_background_image(play_button, "home_bg_btn.png");
        SET_BG_SIZE(play_button, "cover");
        SET_BG_REPEAT(play_button, "no-repeat");
        ui_set_text_color(play_button, "rgb(255, 255, 255)");
        ui_button_fix_text_rendering(play_button);
        
        // 🆕 CONNECTER LES ÉVÉNEMENTS AVEC FEEDBACK VISUEL
        atomic_set_click_handler(play_button->element, play_button_clicked);
        atomic_set_hover_handler(play_button->element, button_hovered);
        atomic_set_unhover_handler(play_button->element, button_unhovered); // Nouveau
        
        ui_log_event("UIComponent", "ButtonSetup", play_button->id, "Click, hover and unhover handlers attached");
        printf("✅ Bouton Play créé avec événements visuels connectés\n");
    }
    
    // Bouton Quit - AVEC ÉVÉNEMENTS CONNECTÉS
    UINode* quit_button = ui_button(data->ui_tree, "quit-button", "QUITTER", NULL, NULL);
    data->quit_button = quit_button; // 🆕 Sauvegarder la référence
    if (quit_button) {
        SET_SIZE(quit_button, 150, 40);
        
        // Configuration visuelle
        ui_button_set_background_image(quit_button, "home_bg_btn.png");
        SET_BG_SIZE(quit_button, "cover");
        SET_BG_REPEAT(quit_button, "no-repeat");
        ui_set_text_color(quit_button, "rgb(255, 255, 255)");
        ui_button_fix_text_rendering(quit_button);
        
        // 🆕 CONNECTER LES ÉVÉNEMENTS AVEC FEEDBACK VISUEL
        atomic_set_click_handler(quit_button->element, quit_button_clicked);
        atomic_set_hover_handler(quit_button->element, button_hovered);
        atomic_set_unhover_handler(quit_button->element, button_unhovered); // Nouveau
        
        ui_log_event("UIComponent", "ButtonSetup", quit_button->id, "Click, hover and unhover handlers attached");
        printf("✅ Bouton Quit créé avec événements visuels connectés\n");
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
    
    // 🔧 FIX: NE PLUS FAIRE Clear ici - le core s'en charge
    // SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255);
    // SDL_RenderClear(renderer);
    
    // Rendre SEULEMENT l'arbre UI
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
    
    // 🔧 FIX PRINCIPAL: NE PLUS FAIRE Present ici - le core s'en charge !
    // SDL_RenderPresent(renderer);
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
    
    // Nouveaux champs pour la structure Scene mise à jour
    scene->id = "home";                   // Identifiant unique
    scene->name = "Home";                 // Nom d'affichage
    scene->target_window = WINDOW_TYPE_MAIN; // Fenêtre cible
    scene->event_manager = NULL;          // Sera initialisé plus tard
    scene->ui_tree = NULL;               // Sera créé dans init
    scene->initialized = false;          // Pas encore initialisé
    scene->active = false;               // Pas encore actif
    
    // Fonctions de callback
    scene->init = home_scene_init;
    scene->update = home_scene_update;
    scene->render = home_scene_render;
    scene->cleanup = home_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

// Connexion des événements adaptée à la nouvelle architecture
void home_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans home_scene_connect_events\n");
        return;
    }
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    // Obtenir l'EventManager du Core
    EventManager* event_manager = game_core_get_event_manager(core);
    if (!event_manager) {
        printf("❌ Event manager NULL\n");
        return;
    }
    
    // Stocker l'EventManager dans la scène selon la nouvelle architecture
    scene->event_manager = event_manager;
    
    // Connecter l'EventManager à l'UITree
    if (data->ui_tree) {
        data->ui_tree->event_manager = event_manager;
        printf("🔗 EventManager connecté à l'UITree\n");
        
        // Enregistrer tous les éléments UI avec des gestionnaires d'événements
        ui_tree_register_all_events(data->ui_tree);
        
        // Stocker l'UITree dans la scène selon la nouvelle architecture
        scene->ui_tree = data->ui_tree;
        
        printf("✅ Tous les événements connectés via l'UITree\n");
    } else {
        printf("❌ UITree est NULL\n");
        return;
    }
    
    // Marquer la scène comme initialisée et active
    scene->initialized = true;
    scene->active = true;
    
    // Stocker la référence du core dans les données de la scène
    data->core = core;
    
    // Log pour confirmation
    log_console_write("HomeScene", "EventsConnected", "home_scene.c", 
                     "[home_scene.c] All UI elements registered with EventManager in new architecture");
}