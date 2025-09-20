#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"  // 🔧 FIX: Ajouter l'include manquant
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

// Callbacks pour les neon buttons
static void multiplayer_clicked(UINode* node, void* user_data) {
    printf("🌐 Multijoueur sélectionné\n");
    (void)node; (void)user_data;
}

static void ai_clicked(UINode* node, void* user_data) {
    printf("🤖 Jeu contre IA sélectionné\n");
    (void)node; (void)user_data;
}

static void wiki_clicked(UINode* node, void* user_data) {
    printf("📚 Wiki ouvert\n");
    (void)node; (void)user_data;
}

// Initialisation de la scène menu
static void menu_scene_init(Scene* scene) {
    printf("📋 Initialisation de la scène Menu avec container modal et neon buttons\n");
    
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
    
    // === CHARGER LES ASSETS (même background que home) ===
    SDL_Texture* background_texture = NULL;
    SDL_Texture* logo_texture = NULL;
    
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
            logo_texture = asset_load_texture(renderer, "fanorona_text.png");
            
            printf("🔍 Assets menu chargés :\n");
            printf("   Background: %s\n", background_texture ? "✅ OK" : "❌ ÉCHEC");
            printf("   Logo: %s\n", logo_texture ? "✅ OK" : "❌ ÉCHEC");
        }
    }
    
    // Container principal (plein écran) avec même background que home
    UINode* app = UI_DIV(data->ui_tree, "menu-app");
    if (!app) {
        printf("❌ Erreur: Impossible de créer le container principal\n");
        free(data);
        return;
    }
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    // Utiliser le même background que home
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
        printf("🖼️ Background identique à home appliqué\n");
    } else {
        SET_BG(app, "rgb(135, 206, 250)"); // Bleu ciel par défaut
    }
    
    // === CONTAINER MODAL CENTRÉ (PLUS GRAND) ===
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "modal-container", 500, 450);
    if (!modal_container) {
        printf("❌ Erreur: Impossible de créer le container modal\n");
        free(data);
        return;
    }
    
    // === LOGO EN HAUT DU CONTAINER ===
    UINode* logo = NULL;
    if (logo_texture) {
        logo = UI_IMAGE(data->ui_tree, "menu-logo", logo_texture);
        if (logo) {
            SET_SIZE(logo, 400, 100);
            atomic_set_background_color(logo->element, 0, 0, 0, 0); // Transparent
            ui_container_add_content(modal_container, logo);
            printf("🖼️ Logo ajouté au container modal\n");
        }
    } else {
        // Fallback texte
        logo = UI_TEXT(data->ui_tree, "menu-logo-text", "FANORONA");
        if (logo) {
            ui_set_text_color(logo, "rgb(255, 165, 0)"); // Orange
            ui_set_text_size(logo, 24);
            ui_set_text_align(logo, "center");
            ui_container_add_content(modal_container, logo);
            printf("📝 Logo texte de secours ajouté au container\n");
        }
    }

    // === TEXTE "STRATEGIE ET TRADITION" ===
    UINode* subtitle = UI_TEXT(data->ui_tree, "menu-subtitle", "STRATEGIE ET TRADITION");
    if (subtitle) {
        ui_set_text_color(subtitle, "rgb(255, 255, 255)"); // Blanc pour contraste
        ui_set_text_size(subtitle, 16);
        ui_set_text_align(subtitle, "center");
        ui_set_text_style(subtitle, false, true); // Italique
        ui_container_add_content(modal_container, subtitle);
        printf("📝 Sous-titre ajouté au container\n");
    }
    
    // === CONTAINER POUR LES NEON BUTTONS ===
    UINode* buttons_container = UI_DIV(data->ui_tree, "neon-buttons-container");
    if (buttons_container) {
        // Configuration flexbox pour disposition en colonne
        ui_set_display_flex(buttons_container);
        FLEX_COLUMN(buttons_container);
        ui_set_justify_content(buttons_container, "center");
        ui_set_align_items(buttons_container, "center");
        ui_set_flex_gap(buttons_container, 15);
        
        // === NEON BUTTONS ===
        
        // 1. Bouton Multijoueur
        UINode* multiplayer_btn = ui_neon_button(data->ui_tree, "multiplayer-btn", "JOUER EN MULTIJOUEUR", multiplayer_clicked, NULL);
        if (multiplayer_btn) {
            SET_SIZE(multiplayer_btn, 280, 45);
            ui_set_text_align(multiplayer_btn, "center");
            APPEND(buttons_container, multiplayer_btn);
            printf("✨ Neon Button 'Multijoueur' créé\n");
        }
        
        // 2. Bouton IA
        UINode* ai_btn = ui_neon_button(data->ui_tree, "ai-btn", "JOUER CONTRE L'IA", ai_clicked, NULL);
        if (ai_btn) {
            SET_SIZE(ai_btn, 280, 45);
            ui_set_text_align(ai_btn, "center");
            APPEND(buttons_container, ai_btn);
            printf("✨ Neon Button 'IA' créé\n");
        }
        
        // 3. Bouton Wiki
        UINode* wiki_btn = ui_neon_button(data->ui_tree, "wiki-btn", "WIKI", wiki_clicked, NULL);
        if (wiki_btn) {
            SET_SIZE(wiki_btn, 280, 45);
            ui_set_text_align(wiki_btn, "center");
            APPEND(buttons_container, wiki_btn);
            printf("✨ Neon Button 'Wiki' créé\n");
        }
        
        // Ajouter le container de boutons au modal
        ui_container_add_content(modal_container, buttons_container);
    }
    
    // Construire la hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Calculer les z-index implicites
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Menu créée avec :\n");
    printf("   🖼️  Background identique à home\n");
    printf("   📦  Container modal centré (500x450, noir transparent, bordure orange)\n");
    printf("   🖼️  Logo Fanorona en haut du container\n");
    printf("   📝  Texte 'Stratégie et Tradition' en italique\n");
    printf("   ✨  3 Neon Buttons avec animation hover automatique\n");
    printf("   📊  Z-index calculés automatiquement\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène menu avec animations neon
static void menu_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // Mettre à jour l'arbre UI
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        // Mettre à jour spécifiquement les animations neon
        ui_neon_button_update_all(data->ui_tree, delta_time);
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
    scene->target_window = WINDOW_TYPE_MINI; // 🔧 FIX: Change from MAIN to MINI to match the current window
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
    
    // Créer un EventManager dédié à la scène au lieu d'utiliser celui du Core
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène menu\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène menu\n");
            return;
        }
    }
    
    // Connecter l'EventManager dédié à l'UITree
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        
        // Enregistrer tous les éléments UI
        ui_tree_register_all_events(data->ui_tree);
        
        // Stocker l'UITree dans la scène
        scene->ui_tree = data->ui_tree;
        
        printf("🔗 EventManager dédié connecté à la scène menu\n");
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Marquer comme initialisé et actif
    scene->initialized = true;
    scene->active = true;
    
    printf("✅ Scène menu prête avec son propre système d'événements\n");
}
