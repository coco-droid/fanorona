#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../core/core.h" // 🆕 FIX: Include core.h for GameCore definition
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../ui/native/atomic.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène menu
typedef struct MenuSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* ai_link;
    UINode* multiplayer_link;
    UINode* wiki_link;
    SDL_Renderer* last_renderer; // 🆕 Suivi du renderer pour rechargement
} MenuSceneData;

// 🆕 Fonction pour styliser le lien comme un neon button
static void style_link_as_neon_button(UINode* link, int r, int g, int b) {
    if (!link || !link->element) return;
    
    // Appliquer le style de base d'un neon button
    SET_SIZE(link, 280, 45);
    ui_set_text_align(link, "center");
    
    // Style neon simulé avec couleur de base
    atomic_set_background_color(link->element, r/4, g/4, b/4, 200); // Couleur de base sombre
    atomic_set_border(link->element, 2, 255, 215, 0, 255); // 🆕 FIX: Bordure dorée (Gold) pour tous les boutons
    atomic_set_text_color_rgba(link->element, 255, 255, 255, 255); // Texte blanc
    atomic_set_padding(link->element, 10, 15, 10, 15);
    
    printf("✨ UI Link stylisé comme neon button avec couleur (%d,%d,%d)\n", r, g, b);
}

// 🆕 Callback hover pour effet neon sur le lien multijoueur
static void multiplayer_link_hovered(void* element, SDL_Event* event) {
    (void)element; (void)event;
}

// 🆕 Callback unhover pour retour normal
static void multiplayer_link_unhovered(void* element, SDL_Event* event) {
    (void)element; (void)event;
}

// 🆕 Callback hover pour effet neon avec scale sur le lien IA
static void ai_link_hovered(void* element, SDL_Event* event) {
    (void)element; (void)event;
}

// 🆕 Callback unhover pour retour normal avec scale
static void ai_link_unhovered(void* element, SDL_Event* event) {
    (void)element; (void)event;
}

// 🆕 Callback hover pour effet neon sur le lien Wiki
static void wiki_link_hovered(void* element, SDL_Event* event) {
    (void)element; (void)event;
}

// 🆕 Callback unhover pour retour normal sur le lien Wiki
static void wiki_link_unhovered(void* element, SDL_Event* event) {
    (void)element; (void)event;
}

// Callback pour activer le mode IA et rediriger vers profile_scene
static void ai_mode_activated_callback(UINode* link) {
    (void)link;
    
    // Activer le mode VS IA AVANT la transition
    config_set_mode(GAME_MODE_VS_AI);
    config_reset_player_configs();  // Réinitialiser les flags J1/J2
    
    printf("🤖 Mode VS IA activé - transition vers profile_scene\n");
    printf("   👤 Seul le joueur humain créera son profil\n");
    printf("   🎯 Après profile_scene → ai_scene pour difficulté\n");
    
    // 🆕 FIX: Activer manuellement le lien pour déclencher la transition
    ui_link_activate(link);
}

// 🆕 Fonction helper pour construire/reconstruire l'UI
static void menu_scene_build_ui(Scene* scene, SDL_Renderer* renderer) {
    MenuSceneData* data = (MenuSceneData*)scene->data;
    if (!data || !renderer) return;

    printf("🏗️ Construction de l'UI Menu pour le renderer %p\n", (void*)renderer);

    // 🆕 FIX: Nettoyer l'event manager AVANT de détruire l'arbre pour éviter les pointeurs morts
    // Cela garantit que les anciens éléments sont retirés de la liste des événements
    if (scene->event_manager) {
        event_manager_clear_all(scene->event_manager);
    }

    // Nettoyer l'ancienne UI si elle existe
    if (data->ui_tree) {
        // 🆕 FIX: Arrêter toutes les animations avant de détruire l'arbre pour éviter les pointeurs morts
        ui_tree_stop_all_animations(data->ui_tree);

        data->ui_tree->event_manager = NULL; // Detach to avoid double free
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Reconnecter l'event manager s'il existe
    if (scene->event_manager) {
        data->ui_tree->event_manager = scene->event_manager;
    }
    
    // === CHARGER LE BACKGROUND ===
    SDL_Texture* background_texture = asset_load_texture(renderer, "fix_bg.png");
    
    // Container principal (plein écran) avec background
    UINode* app = UI_DIV(data->ui_tree, "menu-app");
    if (!app) return;
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // === CONTAINER MODAL AVEC LOGO ET TEXTE AUTOMATIQUES ===
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "modal-container", 500, 450);
    
    // === CONTAINER POUR LES BOUTONS (SIMPLE) ===
    UINode* buttons_container = UI_DIV(data->ui_tree, "buttons-container");
    if (buttons_container) {
        SET_SIZE(buttons_container, 300, 200);
        ui_set_display_flex(buttons_container);
        FLEX_COLUMN(buttons_container);
        ui_set_justify_content(buttons_container, "center");
        ui_set_align_items(buttons_container, "center");
        ui_set_flex_gap(buttons_container, 15);
        
        // 1. UI LINK pour Multijoueur
        data->multiplayer_link = ui_create_link(data->ui_tree, "multiplayer-link", "JOUER EN MULTIJOUEUR", "choice", SCENE_TRANSITION_REPLACE);
        if (data->multiplayer_link) {
            style_link_as_neon_button(data->multiplayer_link, 0, 255, 127);
            ui_animate_fade_in(data->multiplayer_link, 0.8f);
            atomic_set_hover_handler(data->multiplayer_link->element, multiplayer_link_hovered);
            atomic_set_unhover_handler(data->multiplayer_link->element, multiplayer_link_unhovered);
            ui_link_set_target_window(data->multiplayer_link, WINDOW_TYPE_MINI);
            APPEND(buttons_container, data->multiplayer_link);
        }
        
        // 2. UI LINK pour IA
        data->ai_link = ui_create_link(data->ui_tree, "ai-link", "JOUER CONTRE L'IA", "profile", SCENE_TRANSITION_REPLACE);
        if (data->ai_link) {
            style_link_as_neon_button(data->ai_link, 255, 0, 255);
            ui_animate_slide_in_left(data->ai_link, 1.0f, 300.0f);
            ui_link_set_click_handler(data->ai_link, ai_mode_activated_callback);
            atomic_set_hover_handler(data->ai_link->element, ai_link_hovered);
            atomic_set_unhover_handler(data->ai_link->element, ai_link_unhovered);
            ui_link_set_target_window(data->ai_link, WINDOW_TYPE_MINI);
            APPEND(buttons_container, data->ai_link);
        }
        
        // 3. UI LINK pour Wiki
        data->wiki_link = ui_create_link(data->ui_tree, "wiki-link", "WIKI", "wiki", SCENE_TRANSITION_REPLACE);
        if (data->wiki_link) {
            style_link_as_neon_button(data->wiki_link, 0, 191, 255);
            ui_animate_pulse(data->wiki_link, 2.0f);
            atomic_set_hover_handler(data->wiki_link->element, wiki_link_hovered);
            atomic_set_unhover_handler(data->wiki_link->element, wiki_link_unhovered);
            ui_link_set_target_window(data->wiki_link, WINDOW_TYPE_MINI);
            APPEND(buttons_container, data->wiki_link);
        }
        
        ui_container_add_content(modal_container, buttons_container);
        ALIGN_SELF_Y(buttons_container);
    }
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    // Enregistrer les événements
    if (scene->event_manager) {
        ui_tree_register_all_events(data->ui_tree);
    }
    
    // Reconnecter les liens au core si disponible
    if (data->core) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(data->core);
        if (scene_manager) {
            if (data->multiplayer_link) {
                ui_link_connect_to_manager(data->multiplayer_link, scene_manager);
                ui_link_set_activation_delay(data->multiplayer_link, 0.5f);
            }
            if (data->ai_link) {
                ui_link_connect_to_manager(data->ai_link, scene_manager);
                ui_link_set_activation_delay(data->ai_link, 0.5f);
            }
            if (data->wiki_link) {
                ui_link_connect_to_manager(data->wiki_link, scene_manager);
                ui_link_set_activation_delay(data->wiki_link, 0.5f);
            }
        }
    }
    
    scene->ui_tree = data->ui_tree;
    printf("✅ UI Menu reconstruite avec succès\n");
}

// Initialisation de la scène menu
static void menu_scene_init(Scene* scene) {
    printf("📋 Initialisation de la scène Menu\n");
    
    ui_set_hitbox_visualization(false);
    
    MenuSceneData* data = (MenuSceneData*)malloc(sizeof(MenuSceneData));
    if (!data) return;
    
    data->initialized = true;
    data->core = NULL;
    data->ai_link = NULL;
    data->multiplayer_link = NULL;
    data->wiki_link = NULL;
    data->ui_tree = NULL;
    data->last_renderer = NULL;
    
    scene->data = data;
    
    // Construire l'UI si le renderer est disponible
    GameWindow* window = use_mini_window();
    if (window && window->renderer) {
        menu_scene_build_ui(scene, window->renderer);
        data->last_renderer = window->renderer;
    }
}

// Mise à jour de la scène menu avec animations neon
static void menu_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        ui_neon_button_update_all(data->ui_tree, delta_time);
        
        if (data->ai_link) ui_link_update(data->ai_link, delta_time);
        if (data->multiplayer_link) ui_link_update(data->multiplayer_link, delta_time);
        if (data->wiki_link) ui_link_update(data->wiki_link, delta_time);
    }
}

// Rendu de la scène menu
static void menu_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // 🆕 DÉTECTION DE CHANGEMENT DE RENDERER
    if (renderer != data->last_renderer) {
        printf("🔄 Menu: Changement de renderer détecté (%p -> %p) - Rechargement des textures...\n", 
               (void*)data->last_renderer, (void*)renderer);
        menu_scene_build_ui(scene, renderer);
        data->last_renderer = renderer;
    }
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène menu
static void menu_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Menu\n");
    if (!scene || !scene->data) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    if (data->ui_tree) {
        // 🆕 FIX: Detacher l'event manager pour éviter sa destruction par l'arbre
        data->ui_tree->event_manager = NULL;
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    // 🆕 FIX: Marquer comme non initialisé pour forcer un init complet au prochain chargement
    // Cela garantit que l'UI et les événements sont recréés proprement
    scene->initialized = false;
    
    printf("✅ Nettoyage de la scène Menu terminé\n");
}

// Créer la scène menu
Scene* create_menu_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->id = strdup("menu");
    scene->name = strdup("Menu Principal");
    
    if (!scene->id || !scene->name) {
        if (scene->id) free(scene->id);
        if (scene->name) free(scene->name);
        free(scene);
        return NULL;
    }
    
    scene->target_window = WINDOW_TYPE_MINI;
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

void menu_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    if (!data) return;
    
    // Créer un EventManager dédié à la scène
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Si l'UI existe déjà, connecter les événements
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        
        // Connecter les liens au SceneManager
        if (core->scene_manager) {
            if (data->multiplayer_link) {
                ui_link_connect_to_manager(data->multiplayer_link, core->scene_manager);
            }
            if (data->ai_link) {
                ui_link_connect_to_manager(data->ai_link, core->scene_manager);
            }
            if (data->wiki_link) {
                ui_link_connect_to_manager(data->wiki_link, core->scene_manager);
            }
        }
    }
    
    scene->ui_tree = data->ui_tree;
}
