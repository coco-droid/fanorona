#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../config.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène Profile
typedef struct ProfileSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* avatar_selector;
    UINode* back_link;
    UINode* next_button;  // 🆕 Nouveau bouton suivant
} ProfileSceneData;

// 🆕 Callback pour le bouton suivant
static void next_button_clicked(void* element, SDL_Event* event) {
    (void)event;
    
    AtomicElement* atomic = (AtomicElement*)element;
    ProfileSceneData* data = (ProfileSceneData*)atomic->user_data;
    
    if (data && data->avatar_selector) {
        // Réinitialiser le composant avatar
        ui_avatar_selector_reset_to_defaults(data->avatar_selector);
        printf("🔄 Avatar selector réinitialisé via bouton SUIVANT\n");
    }
}

// Initialisation de la scène Profile
static void profile_scene_init(Scene* scene) {
    printf("👤 Initialisation de la scène Profile\n");
    
    ui_set_hitbox_visualization(false);
    
    ProfileSceneData* data = (ProfileSceneData*)malloc(sizeof(ProfileSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour ProfileSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->avatar_selector = NULL;
    data->back_link = NULL;
    data->next_button = NULL;  // 🆕 Initialiser

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
    UINode* app = UI_DIV(data->ui_tree, "profile-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal centré
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "profile-modal", 500, 450);
    
    // Conteneur parent pour tout le contenu
    UINode* content_parent = UI_DIV(data->ui_tree, "profile-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 20);
    
    // Header "CRÉATION DE PROFIL"
    UINode* profile_header = UI_TEXT(data->ui_tree, "profile-header", "CRÉATION DE PROFIL");
    ui_set_text_color(profile_header, "rgb(255, 165, 0)");
    ui_set_text_size(profile_header, 20);
    ui_set_text_align(profile_header, "center");
    ui_set_text_style(profile_header, true, false);
    atomic_set_margin(profile_header->element, 24, 0, 0, 0);
    
    // Avatar Selector
    data->avatar_selector = UI_AVATAR_SELECTOR(data->ui_tree, "profile-avatar-selector");
    if (data->avatar_selector) {
        AVATAR_RESET_DEFAULTS(data->avatar_selector);
        printf("✨ Avatar selector créé et réinitialisé\n");
    }
    
    // 🆕 Container pour les boutons (RETOUR + SUIVANT)
    UINode* buttons_container = UI_DIV(data->ui_tree, "profile-buttons-container");
    SET_SIZE(buttons_container, 350, 50);
    ui_set_display_flex(buttons_container);
    FLEX_ROW(buttons_container);
    ui_set_justify_content(buttons_container, "space-between");
    ui_set_align_items(buttons_container, "center");
    
    // Bouton retour
    data->back_link = ui_create_link(data->ui_tree, "back-link", "RETOUR", "menu", SCENE_TRANSITION_REPLACE);
    if (data->back_link) {
        SET_SIZE(data->back_link, 150, 35);
        ui_set_text_align(data->back_link, "center");
        atomic_set_background_color(data->back_link->element, 64, 64, 64, 200);
        atomic_set_border(data->back_link->element, 2, 128, 128, 128, 255);
        atomic_set_text_color_rgba(data->back_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->back_link->element, 6, 10, 6, 10);
        ui_link_set_target_window(data->back_link, WINDOW_TYPE_MINI);
        APPEND(buttons_container, data->back_link);
    }
    
    // 🆕 Bouton suivant
    data->next_button = ui_button(data->ui_tree, "next-button", "SUIVANT", NULL, NULL);
    if (data->next_button) {
        SET_SIZE(data->next_button, 150, 35);
        ui_set_text_align(data->next_button, "center");
        atomic_set_background_color(data->next_button->element, 0, 128, 0, 200);
        atomic_set_border(data->next_button->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(data->next_button->element, 255, 255, 255, 255);
        atomic_set_padding(data->next_button->element, 6, 10, 6, 10);
        
        // Connecter le callback de clic
        atomic_set_click_handler(data->next_button->element, next_button_clicked);
        data->next_button->element->user_data = data;
        
        APPEND(buttons_container, data->next_button);
    }
    
    // Assembler dans le conteneur parent
    APPEND(content_parent, profile_header);
    APPEND(content_parent, data->avatar_selector);
    APPEND(content_parent, buttons_container);  // 🔧 Remplacer data->back_link par buttons_container

    // Ajouter au modal
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    // Hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Animation d'entrée
    ui_animate_fade_in(modal_container, 0.8f);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Profile créée avec avatar selector\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène Profile
static void profile_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        if (data->avatar_selector) {
            ui_avatar_selector_update(data->avatar_selector, delta_time);
        }
        
        if (data->back_link) {
            ui_link_update(data->back_link, delta_time);
        }
        
        // 🆕 Pas besoin d'update pour le button next (géré par ui_tree_update)
    }
}

// Rendu de la scène Profile
static void profile_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène Profile
static void profile_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Profile\n");
    if (!scene || !scene->data) return;
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Profile terminé\n");
}

// Créer la scène Profile
Scene* create_profile_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Profile\n");
        return NULL;
    }
    
    scene->id = strdup("profile");
    scene->name = strdup("Création de Profil");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène Profile\n");
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
    
    scene->init = profile_scene_init;
    scene->update = profile_scene_update;
    scene->render = profile_scene_render;
    scene->cleanup = profile_scene_cleanup;
    scene->data = NULL;
    
    printf("👤 Profile scene created\n");
    return scene;
}

// Connexion des événements
void profile_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans profile_scene_connect_events\n");
        return;
    }
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène Profile\n");
            return;
        }
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        
        // 🔧 FIX: Enregistrer les événements de l'avatar selector
        if (data->avatar_selector) {
            ui_avatar_selector_register_events(data->avatar_selector, scene->event_manager);
            printf("🔗 Avatar selector events registered with scene EventManager\n");
        }
        
        printf("🔗 EventManager connecté à la scène Profile\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    // Connecter le lien retour
    if (data->back_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
            printf("🔗 Lien 'Retour' connecté au SceneManager\n");
        }
    }
    
    // 🆕 Le bouton SUIVANT est automatiquement enregistré via ui_tree_register_all_events()
    
    printf("✅ Scène Profile prête avec avatar selector et bouton SUIVANT\n");
}