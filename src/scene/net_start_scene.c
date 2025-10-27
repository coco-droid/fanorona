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

// Données pour la scène net_start
typedef struct NetStartSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* host_link;
    UINode* join_link;
    UINode* back_link;
} NetStartSceneData;

// Callback pour lancer une partie (hôte)
static void host_link_clicked(void* element, SDL_Event* event) {
    (void)event;
    
    config_set_network_role(false);  // false = hôte
    config_set_mode(GAME_MODE_ONLINE_MULTIPLAYER);
    config_reset_player_configs();
    printf("🚀 Mode HÔTE activé - Lance une partie réseau\n");
    
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* link = (UINode*)atomic->user_data;
    
    if (link) {
        extern void ui_link_activate(UINode* link);
        ui_link_activate(link);
    }
}

// Callback pour rejoindre une partie (invité)
static void join_link_clicked(void* element, SDL_Event* event) {
    (void)event;
    
    config_set_network_role(true);  // true = invité
    config_set_mode(GAME_MODE_ONLINE_MULTIPLAYER);
    config_reset_player_configs();
    printf("🔗 Mode INVITÉ activé - Rejoint une partie réseau\n");
    
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* link = (UINode*)atomic->user_data;
    
    if (link) {
        extern void ui_link_activate(UINode* link);
        ui_link_activate(link);
    }
}

static void net_start_scene_init(Scene* scene) {
    printf("🌐 Initialisation de la scène Démarrage Réseau\n");
    
    ui_set_hitbox_visualization(false);
    
    NetStartSceneData* data = (NetStartSceneData*)malloc(sizeof(NetStartSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour NetStartSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->host_link = NULL;
    data->join_link = NULL;
    data->back_link = NULL;
    
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
    
    UINode* app = UI_DIV(data->ui_tree, "net-start-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "net-start-modal", 500, 450);
    
    UINode* content_parent = UI_DIV(data->ui_tree, "net-start-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 8);
    
    // Header
    UINode* net_start_header = UI_TEXT(data->ui_tree, "net-start-header", "PARTIE EN LIGNE");
    ui_set_text_color(net_start_header, "rgb(255, 165, 0)");
    ui_set_text_size(net_start_header, 20);
    ui_set_text_align(net_start_header, "center");
    ui_set_text_style(net_start_header, true, false);
    atomic_set_margin(net_start_header->element, 24, 0, 0, 0);
    
    UINode* buttons_container = UI_DIV(data->ui_tree, "net-start-buttons-container");
    SET_SIZE(buttons_container, 350, 180);
    ui_set_display_flex(buttons_container);
    FLEX_COLUMN(buttons_container);
    ui_set_justify_content(buttons_container, "center");
    ui_set_align_items(buttons_container, "center");
    ui_set_flex_gap(buttons_container, 20);
    
    // BOUTON LANCER UNE PARTIE (hôte)
    data->host_link = ui_create_link(data->ui_tree, "host-link", "LANCER UNE PARTIE", "profile", SCENE_TRANSITION_REPLACE);
    if (data->host_link) {
        SET_SIZE(data->host_link, 300, 50);
        ui_set_text_align(data->host_link, "center");
        atomic_set_background_color(data->host_link->element, 64, 0, 128, 200);  // Violet foncé
        atomic_set_border(data->host_link->element, 2, 138, 43, 226, 255);  // Violet clair
        atomic_set_text_color_rgba(data->host_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->host_link->element, 10, 15, 10, 15);
        ui_link_set_target_window(data->host_link, WINDOW_TYPE_MINI);
        ui_animate_slide_in_left(data->host_link, 0.8f, 200.0f);
        atomic_set_click_handler(data->host_link->element, host_link_clicked);
        APPEND(buttons_container, data->host_link);
        printf("✅ Lien 'Lancer' créé (HÔTE)\n");
    }
    
    // BOUTON REJOINDRE UNE PARTIE (invité)
    data->join_link = ui_create_link(data->ui_tree, "join-link", "REJOINDRE UNE PARTIE", "profile", SCENE_TRANSITION_REPLACE);
    if (data->join_link) {
        SET_SIZE(data->join_link, 300, 50);
        ui_set_text_align(data->join_link, "center");
        atomic_set_background_color(data->join_link->element, 0, 64, 128, 200);  // Bleu foncé
        atomic_set_border(data->join_link->element, 2, 30, 144, 255, 255);  // Bleu clair
        atomic_set_text_color_rgba(data->join_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->join_link->element, 10, 15, 10, 15);
        ui_link_set_target_window(data->join_link, WINDOW_TYPE_MINI);
        ui_animate_slide_in_right(data->join_link, 1.0f, 200.0f);
        atomic_set_click_handler(data->join_link->element, join_link_clicked);
        APPEND(buttons_container, data->join_link);
        printf("✅ Lien 'Rejoindre' créé (INVITÉ)\n");
    }
    
    // Bouton retour
    UINode* back_link = ui_create_link(data->ui_tree, "back-link", "RETOUR", "choice", SCENE_TRANSITION_REPLACE);
    if (back_link) {
        SET_SIZE(back_link, 150, 35);
        ui_set_text_align(back_link, "center");
        atomic_set_background_color(back_link->element, 64, 64, 64, 200);
        atomic_set_border(back_link->element, 2, 128, 128, 128, 255);
        atomic_set_text_color_rgba(back_link->element, 255, 255, 255, 255);
        atomic_set_padding(back_link->element, 6, 10, 6, 10);
        atomic_set_margin(back_link->element, 8, 0, 0, 0);
        ui_link_set_target_window(back_link, WINDOW_TYPE_MINI);
        data->back_link = back_link;
    }
    
    APPEND(content_parent, net_start_header);
    APPEND(content_parent, buttons_container);
    APPEND(content_parent, back_link);
    
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_animate_fade_in(modal_container, 0.6f);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface NetStart créée avec boutons Hôte/Invité\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

static void net_start_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    NetStartSceneData* data = (NetStartSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        if (data->host_link) {
            ui_link_update(data->host_link, delta_time);
        }
        
        if (data->join_link) {
            ui_link_update(data->join_link, delta_time);
        }
        
        if (data->back_link) {
            ui_link_update(data->back_link, delta_time);
        }
    }
}

static void net_start_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    NetStartSceneData* data = (NetStartSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

static void net_start_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène NetStart\n");
    if (!scene || !scene->data) return;
    
    NetStartSceneData* data = (NetStartSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène NetStart terminé\n");
}

Scene* create_net_start_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène NetStart\n");
        return NULL;
    }
    
    scene->id = strdup("net_start");
    scene->name = strdup("Démarrage Réseau");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène NetStart\n");
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
    
    scene->init = net_start_scene_init;
    scene->update = net_start_scene_update;
    scene->render = net_start_scene_render;
    scene->cleanup = net_start_scene_cleanup;
    scene->data = NULL;
    
    printf("🌐 NetStart scene created\n");
    return scene;
}

void net_start_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans net_start_scene_connect_events\n");
        return;
    }
    
    NetStartSceneData* data = (NetStartSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène net_start\n");
            return;
        }
    }
    
    // 🔧 FIX CRITIQUE: Connecter l'EventManager à l'UITree AVANT l'enregistrement
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        
        // 🔧 FIX: Enregistrer EXPLICITEMENT les boutons avec leurs click handlers
        if (data->host_link && data->host_link->element) {
            atomic_register_with_event_manager(data->host_link->element, scene->event_manager);
            printf("🔗 Bouton 'Lancer' enregistré dans EventManager\n");
        }
        
        if (data->join_link && data->join_link->element) {
            atomic_register_with_event_manager(data->join_link->element, scene->event_manager);
            printf("🔗 Bouton 'Rejoindre' enregistré dans EventManager\n");
        }
        
        // Enregistrer tous les autres éléments UI
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        printf("🔗 EventManager connecté à la scène net_start avec boutons explicitement enregistrés\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    extern SceneManager* game_core_get_scene_manager(GameCore* core);
    SceneManager* scene_manager = game_core_get_scene_manager(core);
    
    if (scene_manager) {
        if (data->host_link) {
            ui_link_connect_to_manager(data->host_link, scene_manager);
            ui_link_set_activation_delay(data->host_link, 0.5f);
            printf("🔗 Lien 'Lancer' connecté au SceneManager\n");
        }
        
        if (data->join_link) {
            ui_link_connect_to_manager(data->join_link, scene_manager);
            ui_link_set_activation_delay(data->join_link, 0.5f);
            printf("🔗 Lien 'Rejoindre' connecté au SceneManager\n");
        }
        
        if (data->back_link) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
            ui_link_set_activation_delay(data->back_link, 0.5f);
            printf("🔗 Lien 'Retour' connecté au SceneManager\n");
        }
    }
    
    printf("✅ Scène net_start prête avec événements EXPLICITEMENT connectés\n");
}
