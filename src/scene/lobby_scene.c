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
#include "../net/network.h"  // 🆕 ADD

// États du lobby
typedef enum {
    LOBBY_STATE_SEARCHING,   // "RECHERCHE..."
    LOBBY_STATE_WAITING,     // "ATTENTE..."
    LOBBY_STATE_CONNECTED    // "CONNECTÉ"
} LobbyState;

// Données pour la scène Lobby
typedef struct LobbySceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* status_text;
    UINode* player2_avatar;
    UINode* player2_name;
    UINode* back_link;
    LobbyState current_state;
    float state_timer;
    NetworkManager* network;  // 🆕 ADD
} LobbySceneData;

static void lobby_scene_init(Scene* scene) {
    printf("🌐 Initialisation de la scène Lobby Multijoueur\n");
    
    ui_set_hitbox_visualization(false);
    
    LobbySceneData* data = (LobbySceneData*)malloc(sizeof(LobbySceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour LobbySceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->status_text = NULL;
    data->player2_avatar = NULL;
    data->player2_name = NULL;
    data->back_link = NULL;
    data->current_state = LOBBY_STATE_SEARCHING;
    data->state_timer = 0.0f;
    data->network = NULL;  // 🆕 ADD
    
    // 🆕 NETWORK EXPOSURE: If invite mode, expose player on network
    if (config_is_network_invite()) {
        printf("🌐 Mode INVITÉ: Exposition du profil sur le réseau...\n");
        
        data->network = network_create();
        if (data->network && network_init(data->network)) {
            const char* player_name = config_get_player1_name();
            
            if (network_host_game(data->network, "Partie Fanorona", player_name)) {
                printf("✅ Profil exposé: '%s'\n", player_name);
                data->current_state = LOBBY_STATE_WAITING;
                atomic_set_text(data->status_text->element, "ATTENTE INVITATION...");
                ui_set_text_color(data->status_text, "rgb(255, 165, 0)");
            } else {
                printf("❌ Échec exposition réseau: %s\n", 
                       network_get_last_error(data->network));
            }
        }
    }
    
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Charger background
    SDL_Texture* background_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
        }
    }
    
    UINode* app = UI_DIV(data->ui_tree, "lobby-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "lobby-modal", 500, 450);
    
    UINode* content_parent = UI_DIV(data->ui_tree, "lobby-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 15);
    
    // Sous-titre "PAIRAGE MULTIJOUEUR"
    UINode* lobby_subtitle = UI_TEXT(data->ui_tree, "lobby-subtitle", "PAIRAGE MULTIJOUEUR");
    ui_set_text_color(lobby_subtitle, "rgb(255, 165, 0)");
    ui_set_text_size(lobby_subtitle, 18);
    ui_set_text_align(lobby_subtitle, "center");
    ui_set_text_style(lobby_subtitle, true, false);
    atomic_set_margin(lobby_subtitle->element, 24, 0, 0, 0);
    
    // Container horizontal pour les joueurs
    UINode* players_row = UI_DIV(data->ui_tree, "players-row");
    SET_SIZE(players_row, 420, 120);
    ui_set_display_flex(players_row);
    ui_set_flex_direction(players_row, "row");
    ui_set_justify_content(players_row, "space-between");
    ui_set_align_items(players_row, "center");
    atomic_set_padding(players_row->element, 10, 10, 10, 10);
    
    // === JOUEUR 1 (LOCAL) ===
    UINode* player1_container = UI_DIV(data->ui_tree, "player1-container");
    SET_SIZE(player1_container, 90, 100);
    ui_set_display_flex(player1_container);
    FLEX_COLUMN(player1_container);
    ui_set_align_items(player1_container, "center");
    ui_set_justify_content(player1_container, "center");
    ui_set_flex_gap(player1_container, 8);
    
    // Avatar joueur 1
    UINode* player1_avatar = UI_DIV(data->ui_tree, "player1-avatar");
    SET_SIZE(player1_avatar, 60, 60);
    atomic_set_border_radius(player1_avatar->element, 30);
    atomic_set_border(player1_avatar->element, 3, 0, 255, 0, 255);
    
    // Charger avatar du joueur 1 depuis config
    AvatarID p1_avatar = config_get_player1_avatar();
    const char* p1_avatar_path = avatar_id_to_filename(p1_avatar);
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        SDL_Texture* p1_tex = asset_load_texture(renderer, p1_avatar_path);
        if (p1_tex) {
            atomic_set_background_image(player1_avatar->element, p1_tex);
            atomic_set_background_size_str(player1_avatar->element, "cover");
        }
    }
    
    // Nom joueur 1
    const char* p1_name = config_get_player1_name();
    UINode* player1_name = UI_TEXT(data->ui_tree, "player1-name", p1_name);
    ui_set_text_color(player1_name, "rgb(255, 255, 255)");
    ui_set_text_size(player1_name, 11);
    ui_set_text_align(player1_name, "center");
    ui_set_text_style(player1_name, true, false);
    
    APPEND(player1_container, player1_avatar);
    APPEND(player1_container, player1_name);
    
    // === TEXTE DE STATUT (CENTRE) ===
    UINode* status_container = UI_DIV(data->ui_tree, "status-container");
    SET_SIZE(status_container, 120, 100);
    ui_set_display_flex(status_container);
    FLEX_COLUMN(status_container);
    ui_set_align_items(status_container, "center");
    ui_set_justify_content(status_container, "center");
    
    data->status_text = UI_TEXT(data->ui_tree, "status-text", "RECHERCHE...");
    ui_set_text_color(data->status_text, "rgb(255, 255, 0)");
    ui_set_text_size(data->status_text, 13);
    ui_set_text_align(data->status_text, "center");
    ui_set_text_style(data->status_text, true, false);
    ui_animate_pulse(data->status_text, 1.5f);
    
    APPEND(status_container, data->status_text);
    
    // === JOUEUR 2 (DISTANT / INCONNU) ===
    UINode* player2_container = UI_DIV(data->ui_tree, "player2-container");
    SET_SIZE(player2_container, 90, 100);
    ui_set_display_flex(player2_container);
    FLEX_COLUMN(player2_container);
    ui_set_align_items(player2_container, "center");
    ui_set_justify_content(player2_container, "center");
    ui_set_flex_gap(player2_container, 8);
    
    // Avatar joueur 2 (point d'interrogation par défaut)
    data->player2_avatar = UI_DIV(data->ui_tree, "player2-avatar");
    SET_SIZE(data->player2_avatar, 60, 60);
    atomic_set_border_radius(data->player2_avatar->element, 30);
    atomic_set_border(data->player2_avatar->element, 2, 128, 128, 128, 255);
    atomic_set_background_color(data->player2_avatar->element, 64, 64, 64, 200);
    
    // Point d'interrogation (texte centré)
    UINode* question_mark = UI_TEXT(data->ui_tree, "question-mark", "?");
    ui_set_text_color(question_mark, "rgb(128, 128, 128)");
    ui_set_text_size(question_mark, 32);
    ui_set_text_align(question_mark, "center");
    ui_set_text_style(question_mark, true, false);
    SET_SIZE(question_mark, 60, 60);
    APPEND(data->player2_avatar, question_mark);
    
    // Nom joueur 2
    data->player2_name = UI_TEXT(data->ui_tree, "player2-name", "INCONNU");
    ui_set_text_color(data->player2_name, "rgb(128, 128, 128)");
    ui_set_text_size(data->player2_name, 11);
    ui_set_text_align(data->player2_name, "center");
    ui_set_text_style(data->player2_name, true, false);
    
    APPEND(player2_container, data->player2_avatar);
    APPEND(player2_container, data->player2_name);
    
    // Assembler la rangée de joueurs
    APPEND(players_row, player1_container);
    APPEND(players_row, status_container);
    APPEND(players_row, player2_container);
    
    // 🆕 Bouton "TROUVER UN ADVERSAIRE"
    UINode* find_button = ui_create_link(data->ui_tree, "find-opponent-btn", "TROUVER UN ADVERSAIRE", 
                                         NULL, SCENE_TRANSITION_REPLACE);
    if (find_button) {
        SET_SIZE(find_button, 250, 40);
        ui_set_text_align(find_button, "center");
        atomic_set_background_color(find_button->element, 0, 128, 0, 200);
        atomic_set_border(find_button->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(find_button->element, 255, 255, 255, 255);
        atomic_set_padding(find_button->element, 8, 12, 8, 12);
        atomic_set_margin(find_button->element, 15, 0, 0, 0);
    }
    
    // 🆕 Bouton "RETOUR"
    UINode* back_button = ui_create_link(data->ui_tree, "back-button", "RETOUR", 
                                        "choice", SCENE_TRANSITION_REPLACE);
    if (back_button) {
        SET_SIZE(back_button, 150, 35);
        ui_set_text_align(back_button, "center");
        atomic_set_background_color(back_button->element, 64, 64, 64, 200);
        atomic_set_border(back_button->element, 2, 128, 128, 128, 255);
        atomic_set_text_color_rgba(back_button->element, 255, 255, 255, 255);
        atomic_set_padding(back_button->element, 6, 10, 6, 10);
        atomic_set_margin(back_button->element, 10, 0, 0, 0);
        ui_link_set_target_window(back_button, WINDOW_TYPE_MINI);
        data->back_link = back_button;
    }
    
    // Assembler tout
    APPEND(content_parent, lobby_subtitle);
    APPEND(content_parent, players_row);
    APPEND(content_parent, find_button);
    APPEND(content_parent, back_button);
    
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_animate_fade_in(modal_container, 0.8f);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Lobby créée - Attente du Joueur 2\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

static void lobby_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    LobbySceneData* data = (LobbySceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    // 🆕 NETWORK UPDATE: Check for connection from host
    if (data->network) {
        network_update(data->network, delta_time);
        
        // Check for incoming invitation
        if (network_has_pending_messages(data->network)) {
            ProtocolMessage* msg = network_poll_message(data->network);
            if (msg && msg->header.type == MSG_CONNECT_REQUEST) {
                printf("📩 Invitation reçue!\n");
                
                data->current_state = LOBBY_STATE_CONNECTED;
                atomic_set_text(data->status_text->element, "CONNECTÉ");
                ui_set_text_color(data->status_text, "rgb(0, 255, 0)");
                
                // TODO: Extract remote player profile from message
                atomic_set_text(data->player2_name->element, "Hôte");
                ui_set_text_color(data->player2_name, "rgb(255, 255, 255)");
                
                protocol_message_destroy(msg);
            }
        }
    } else {
        // Fallback to old simulation if no network
        // Simulation d'évolution d'état (à remplacer par vraie logique réseau)
        data->state_timer += delta_time;
    
        if (data->state_timer > 3.0f && data->current_state == LOBBY_STATE_SEARCHING) {
            data->current_state = LOBBY_STATE_WAITING;
            atomic_set_text(data->status_text->element, "ATTENTE...");
            ui_set_text_color(data->status_text, "rgb(255, 165, 0)");
            data->state_timer = 0.0f;
        }
    
        if (data->state_timer > 5.0f && data->current_state == LOBBY_STATE_WAITING) {
            data->current_state = LOBBY_STATE_CONNECTED;
            atomic_set_text(data->status_text->element, "CONNECTÉ");
            ui_set_text_color(data->status_text, "rgb(0, 255, 0)");
        
            // Mettre à jour joueur 2 (simulé)
            atomic_set_text(data->player2_name->element, "Adversaire");
            ui_set_text_color(data->player2_name, "rgb(255, 255, 255)");
        
            data->state_timer = 0.0f;
        }
    }
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        if (data->back_link) {
            ui_link_update(data->back_link, delta_time);
        }
    }
}

static void lobby_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    LobbySceneData* data = (LobbySceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

static void lobby_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Lobby\n");
    if (!scene || !scene->data) return;
    
    LobbySceneData* data = (LobbySceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    // 🆕 CLEANUP NETWORK
    if (data->network) {
        network_disconnect(data->network);
        network_destroy(data->network);
        data->network = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Lobby terminé\n");
}

Scene* create_lobby_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Lobby\n");
        return NULL;
    }
    
    scene->id = strdup("lobby");
    scene->name = strdup("Lobby Multijoueur");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène Lobby\n");
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
    
    scene->init = lobby_scene_init;
    scene->update = lobby_scene_update;
    scene->render = lobby_scene_render;
    scene->cleanup = lobby_scene_cleanup;
    scene->data = NULL;
    
    printf("🌐 Lobby scene created\n");
    return scene;
}

void lobby_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans lobby_scene_connect_events\n");
        return;
    }
    
    LobbySceneData* data = (LobbySceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène Lobby\n");
            return;
        }
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        printf("🔗 EventManager connecté à la scène Lobby\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    if (data->back_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
            ui_link_set_activation_delay(data->back_link, 0.5f);
            printf("🔗 Lien 'Annuler' connecté au SceneManager\n");
        }
    }
    
    printf("✅ Scène Lobby prête avec système d'attente multijoueur\n");
}
