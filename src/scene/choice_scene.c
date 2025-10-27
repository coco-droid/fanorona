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

// Données pour la scène choice
typedef struct ChoiceSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* local_link;
    UINode* online_link;
    UINode* back_link; // 🔧 Référence au bouton retour
} ChoiceSceneData;

// Callbacks AVANT usage
static void local_link_clicked(void* element, SDL_Event* event) {
    (void)event;
    
    config_set_mode(GAME_MODE_LOCAL_MULTIPLAYER);
    config_reset_player_configs();  // 🆕 Réinitialiser les flags J1/J2
    printf("🎮 Mode MULTIJOUEUR LOCAL activé (flags réinitialisés)\n");
    
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* link = (UINode*)atomic->user_data;
    
    if (link) {
        extern void ui_link_activate(UINode* link);
        ui_link_activate(link);
    }
}

static void online_link_clicked(void* element, SDL_Event* event) {
    (void)event;
    
    config_set_mode(GAME_MODE_ONLINE_MULTIPLAYER);
    config_reset_player_configs();
    printf("🌐 Mode MULTIJOUEUR EN LIGNE activé (flags réinitialisés)\n");
    
    // 🔧 FIX: Activate the link to trigger transition
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* link = (UINode*)atomic->user_data;
    
    if (link) {
        extern void ui_link_activate(UINode* link);
        ui_link_activate(link);
    }
}

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
    data->online_link = NULL;
    data->back_link = NULL; // 🔧 Initialiser à NULL
    
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    SDL_Texture* background_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
        }
    }
    
    UINode* app = UI_DIV(data->ui_tree, "choice-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "choice-modal", 500, 450);
    
    // 🆕 CONTENEUR PARENT POUR TOUT LE CONTENU
    UINode* content_parent = UI_DIV(data->ui_tree, "choice-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 8);
    
    // Header avec marge de 24px
    UINode* choice_header = UI_TEXT(data->ui_tree, "choice-header", "CHOISIR UN MODE DE JEU");
    ui_set_text_color(choice_header, "rgb(255, 165, 0)");
    ui_set_text_size(choice_header, 20);
    ui_set_text_align(choice_header, "center");
    ui_set_text_style(choice_header, true, false);
    atomic_set_margin(choice_header->element, 24, 0, 0, 0); // 🔧 24px margin-top
    
    UINode* buttons_container = UI_DIV(data->ui_tree, "choice-buttons-container");
    SET_SIZE(buttons_container, 350, 180);
    
    ui_set_display_flex(buttons_container);
    FLEX_COLUMN(buttons_container);
    ui_set_justify_content(buttons_container, "center");
    ui_set_align_items(buttons_container, "center");
    ui_set_flex_gap(buttons_container, 20);
    
    // BOUTON LOCAL
    data->local_link = ui_create_link(data->ui_tree, "local-link", "JOUER EN LOCAL", "profile", SCENE_TRANSITION_REPLACE);
    if (data->local_link) {
        SET_SIZE(data->local_link, 300, 50);
        ui_set_text_align(data->local_link, "center");
        
        atomic_set_background_color(data->local_link->element, 0, 64, 0, 200);
        atomic_set_border(data->local_link->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(data->local_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->local_link->element, 10, 15, 10, 15);
        
        ui_link_set_target_window(data->local_link, WINDOW_TYPE_MINI);
        ui_animate_slide_in_left(data->local_link, 0.8f, 200.0f);
        
        atomic_set_click_handler(data->local_link->element, local_link_clicked);
        
        APPEND(buttons_container, data->local_link);
        printf("✅ Lien 'Local' créé avec transition vers profile_scene\n");
    }
    
    // BOUTON EN LIGNE
    data->online_link = ui_create_link(data->ui_tree, "online-link", "JOUER EN LIGNE", "net_start", SCENE_TRANSITION_REPLACE);
    if (data->online_link) {
        SET_SIZE(data->online_link, 300, 50);
        ui_set_text_align(data->online_link, "center");
        
        atomic_set_background_color(data->online_link->element, 0, 47, 64, 200);
        atomic_set_border(data->online_link->element, 2, 0, 191, 255, 255);
        atomic_set_text_color_rgba(data->online_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->online_link->element, 10, 15, 10, 15);
        
        ui_link_set_target_window(data->online_link, WINDOW_TYPE_MINI);
        ui_animate_slide_in_right(data->online_link, 1.0f, 200.0f);
        
        atomic_set_click_handler(data->online_link->element, online_link_clicked);
        
        APPEND(buttons_container, data->online_link);
        printf("✅ Lien 'En ligne' créé avec transition vers net_start_scene\n");
    }
    
    // Bouton retour (comme wiki_scene)
    UINode* back_link = ui_create_link(data->ui_tree, "back-link", "RETOUR", "menu", SCENE_TRANSITION_REPLACE);
    if (back_link) {
        SET_SIZE(back_link, 150, 35);
        ui_set_text_align(back_link, "center");
        atomic_set_background_color(back_link->element, 64, 64, 64, 200);
        atomic_set_border(back_link->element, 2, 128, 128, 128, 255);
        atomic_set_text_color_rgba(back_link->element, 255, 255, 255, 255);
        atomic_set_padding(back_link->element, 6, 10, 6, 10);
        atomic_set_margin(back_link->element, 8, 0, 0, 0);
        ui_link_set_target_window(back_link, WINDOW_TYPE_MINI);
        data->back_link = back_link; // 🔧 Stocker la référence
    }
    
    // Assembler dans content_parent
    APPEND(content_parent, choice_header);
    APPEND(content_parent, buttons_container);
    APPEND(content_parent, back_link);
    
    // Ajouter au modal
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_animate_fade_in(modal_container, 0.6f);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Choix créée avec :\n");
    printf("   🏠 Bouton Local → profile_scene\n");
    printf("   🌐 Bouton En ligne → profile_scene\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

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
        
        if (data->online_link) {
            ui_link_update(data->online_link, delta_time);
        }
        
        if (data->back_link) {
            ui_link_update(data->back_link, delta_time);
        }
    }
}

static void choice_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    ChoiceSceneData* data = (ChoiceSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

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
    
    if (data->local_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->local_link, scene_manager);
            ui_link_set_activation_delay(data->local_link, 0.5f);
            printf("🔗 Lien 'Local' connecté au SceneManager\n");
        }
    }
    
    if (data->online_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->online_link, scene_manager);
            ui_link_set_activation_delay(data->online_link, 0.5f);
            printf("🔗 Lien 'En ligne' connecté au SceneManager\n");
        }
    }
    
    // Connecter le bouton retour
    if (data->back_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
            ui_link_set_activation_delay(data->back_link, 0.5f);
            printf("🔗 Lien 'Retour' connecté au SceneManager\n");
        }
    }
    
    printf("✅ Scène choice prête avec 2 liens vers profile_scene\n");
}
