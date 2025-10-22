#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène choice
typedef struct ChoiceSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* local_link;
    UINode* online_btn;
} ChoiceSceneData;

// Callback pour le bouton en ligne (pas encore implémenté)
static void online_clicked(UINode* node, void* user_data) {
    printf("🌐 Jeu en ligne sélectionné (pas encore implémenté)\n");
    (void)node; (void)user_data;
}

// Initialisation de la scène choice
static void choice_scene_init(Scene* scene) {
    printf("🎮 Initialisation de la scène Choix de mode\n");
    
    ui_set_hitbox_visualization(false);
    
    ChoiceSceneData* data = (ChoiceSceneData*)malloc(sizeof(ChoiceSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour ChoiceSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->local_link = NULL;
    data->online_btn = NULL;
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Charger le background
    SDL_Texture* background_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
        }
    }
    
    // Container principal
    UINode* app = UI_DIV(data->ui_tree, "choice-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal avec header
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "choice-modal", 500, 380);
    ui_container_add_header(modal_container, "CHOISIR UN MODE DE JEU");
    
    // Container pour les boutons
    UINode* buttons_container = UI_DIV(data->ui_tree, "choice-buttons-container");
    SET_SIZE(buttons_container, 350, 180);
    
    ui_set_display_flex(buttons_container);
    FLEX_COLUMN(buttons_container);
    ui_set_justify_content(buttons_container, "center");
    ui_set_align_items(buttons_container, "center");
    ui_set_flex_gap(buttons_container, 20);
    
    // === BOUTON LOCAL (UI Link vers menu_scene) ===
    data->local_link = ui_create_link(data->ui_tree, "local-link", "JOUER EN LOCAL", "menu", SCENE_TRANSITION_REPLACE);
    if (data->local_link) {
        SET_SIZE(data->local_link, 300, 50);
        ui_set_text_align(data->local_link, "center");
        
        // Style neon vert
        atomic_set_background_color(data->local_link->element, 0, 64, 0, 200);
        atomic_set_border(data->local_link->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(data->local_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->local_link->element, 10, 15, 10, 15);
        
        ui_link_set_target_window(data->local_link, WINDOW_TYPE_MINI);
        
        // Animation d'entrée
        ui_animate_slide_in_left(data->local_link, 0.8f, 200.0f);
        
        APPEND(buttons_container, data->local_link);
        printf("✅ Lien 'Local' créé avec transition vers menu_scene\n");
    }
    
    // === BOUTON EN LIGNE (Neon button classique) ===
    data->online_btn = ui_neon_button(data->ui_tree, "online-btn", "JOUER EN LIGNE", online_clicked, NULL);
    if (data->online_btn) {
        SET_SIZE(data->online_btn, 300, 50);
        ui_set_text_align(data->online_btn, "center");
        ui_neon_button_set_glow_color(data->online_btn, 0, 191, 255); // Bleu
        
        // Animation d'entrée
        ui_animate_slide_in_right(data->online_btn, 1.0f, 200.0f);
        
        APPEND(buttons_container, data->online_btn);
        printf("✅ Bouton 'En ligne' créé (placeholder)\n");
    }
    
    // Ajouter le container de boutons au modal
    ui_container_add_content(modal_container, buttons_container);
    ALIGN_SELF_Y(buttons_container);
    
    // Construire la hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Animation du modal
    ui_animate_fade_in(modal_container, 0.6f);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Choix créée avec :\n");
    printf("   🏠 Bouton Local → menu_scene\n");
    printf("   🌐 Bouton En ligne (placeholder)\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène choice
static void choice_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    ChoiceSceneData* data = (ChoiceSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        ui_neon_button_update_all(data->ui_tree, delta_time);
        
        if (data->local_link) {
            ui_link_update(data->local_link, delta_time);
        }
    }
}

// Rendu de la scène choice
static void choice_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    ChoiceSceneData* data = (ChoiceSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène choice
static void choice_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Choix\n");
    if (!scene || !scene->data) return;
    
    ChoiceSceneData* data = (ChoiceSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Choix terminé\n");
}

// Créer la scène choice
Scene* create_choice_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Choice\n");
        return NULL;
    }
    
    scene->id = strdup("choice");
    scene->name = strdup("Choix de mode");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène Choice\n");
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
    
    scene->init = choice_scene_init;
    scene->update = choice_scene_update;
    scene->render = choice_scene_render;
    scene->cleanup = choice_scene_cleanup;
    scene->data = NULL;
    
    printf("✅ Choice scene created\n");
    return scene;
}

// Connecter les événements
void choice_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans choice_scene_connect_events\n");
        return;
    }
    
    ChoiceSceneData* data = (ChoiceSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène choice\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène choice\n");
            return;
        }
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        printf("🔗 EventManager connecté à la scène choice\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    // Connecter le lien local au SceneManager
    if (data->local_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->local_link, scene_manager);
            ui_link_set_activation_delay(data->local_link, 0.5f);
            printf("🔗 Lien 'Local' connecté au SceneManager\n");
        }
    }
    
    printf("✅ Scène choice prête\n");
}
