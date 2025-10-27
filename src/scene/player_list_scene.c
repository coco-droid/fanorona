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

#define MAX_PLAYERS_PER_PAGE 3
#define MAX_DISCOVERED_PLAYERS 20

typedef struct NetworkPlayer {
    char name[64];
    char remote_id[32];
    AvatarID avatar;
    bool is_available;
} NetworkPlayer;

typedef struct PlayerListSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* players_container;
    UINode* prev_button;
    UINode* next_button;
    UINode* cancel_link;
    UINode* page_indicator;
    NetworkPlayer discovered_players[MAX_DISCOVERED_PLAYERS];
    int total_players;
    int current_page;
    int total_pages;
} PlayerListSceneData;

// Simulation: découvrir des joueurs réseau
static void simulate_discover_players(PlayerListSceneData* data) {
    const char* names[] = {"Alex", "Bob", "Charlie", "Diana", "Eve", "Frank", "Grace", "Henry"};
    int count = 8;
    
    data->total_players = count;
    for (int i = 0; i < count && i < MAX_DISCOVERED_PLAYERS; i++) {
        strncpy(data->discovered_players[i].name, names[i], 63);
        snprintf(data->discovered_players[i].remote_id, 31, "player_%d", i + 1);
        data->discovered_players[i].avatar = (AvatarID)((i % 6) + 1);
        data->discovered_players[i].is_available = true;
    }
    
    data->total_pages = (data->total_players + MAX_PLAYERS_PER_PAGE - 1) / MAX_PLAYERS_PER_PAGE;
    data->current_page = 0;
}

static void invite_player_callback(void* element, SDL_Event* event) {
    (void)event;
    AtomicElement* atomic = (AtomicElement*)element;
    int player_index = (int)(intptr_t)atomic->user_data;
    
    printf("📩 Invitation envoyée au joueur #%d\n", player_index);
    // TODO: Logique réseau réelle
}

static void render_player_page(PlayerListSceneData* data) {
    if (!data->players_container) return;
    
    // Clear existing children
    while (data->players_container->children_count > 0) {
        ui_tree_remove_child(data->players_container, data->players_container->children[0]);
    }
    
    int start_idx = data->current_page * MAX_PLAYERS_PER_PAGE;
    int end_idx = start_idx + MAX_PLAYERS_PER_PAGE;
    if (end_idx > data->total_players) end_idx = data->total_players;
    
    for (int i = start_idx; i < end_idx; i++) {
        NetworkPlayer* player = &data->discovered_players[i];
        
        UINode* player_row = UI_DIV(data->ui_tree, "player-row");
        SET_SIZE(player_row, 420, 60);
        ui_set_display_flex(player_row);
        ui_set_flex_direction(player_row, "row");
        ui_set_justify_content(player_row, "space-between");
        ui_set_align_items(player_row, "center");
        atomic_set_background_color(player_row->element, 0, 0, 0, 30);
        atomic_set_padding(player_row->element, 8, 10, 8, 10);
        atomic_set_margin(player_row->element, 5, 0, 0, 0);
        
        // Avatar
        UINode* avatar = UI_DIV(data->ui_tree, "player-avatar");
        SET_SIZE(avatar, 45, 45);
        atomic_set_border_radius(avatar->element, 22);
        atomic_set_border(avatar->element, 2, 0, 255, 0, 255);
        
        GameWindow* window = use_mini_window();
        if (window) {
            SDL_Renderer* renderer = window_get_renderer(window);
            const char* avatar_path = avatar_id_to_filename(player->avatar);
            SDL_Texture* tex = asset_load_texture(renderer, avatar_path);
            if (tex) {
                atomic_set_background_image(avatar->element, tex);
                atomic_set_background_size_str(avatar->element, "cover");
            }
        }
        
        // Name
        UINode* name_text = UI_TEXT(data->ui_tree, "player-name-text", player->name);
        ui_set_text_color(name_text, "rgb(255, 255, 255)");
        ui_set_text_size(name_text, 12);
        ui_set_text_style(name_text, true, false);
        SET_SIZE(name_text, 200, 45);
        
        // Invite button
        UINode* invite_btn = ui_create_link(data->ui_tree, "invite-btn", "INVITER", NULL, SCENE_TRANSITION_REPLACE);
        SET_SIZE(invite_btn, 100, 35);
        ui_set_text_align(invite_btn, "center");
        atomic_set_background_color(invite_btn->element, 0, 128, 0, 200);
        atomic_set_border(invite_btn->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(invite_btn->element, 255, 255, 255, 255);
        atomic_set_padding(invite_btn->element, 5, 8, 5, 8);
        invite_btn->element->user_data = (void*)(intptr_t)i;
        atomic_set_click_handler(invite_btn->element, invite_player_callback);
        
        APPEND(player_row, avatar);
        APPEND(player_row, name_text);
        APPEND(player_row, invite_btn);
        APPEND(data->players_container, player_row);
    }
    
    // Update page indicator
    if (data->page_indicator) {
        char page_text[32];
        snprintf(page_text, 31, "Page %d/%d", data->current_page + 1, data->total_pages);
        atomic_set_text(data->page_indicator->element, page_text);
    }
}

static void next_page_clicked(void* element, SDL_Event* event) {
    (void)element; (void)event;
    extern PlayerListSceneData* g_current_player_list_data;
    if (g_current_player_list_data && g_current_player_list_data->current_page < g_current_player_list_data->total_pages - 1) {
        g_current_player_list_data->current_page++;
        render_player_page(g_current_player_list_data);
    }
}

static void prev_page_clicked(void* element, SDL_Event* event) {
    (void)element; (void)event;
    extern PlayerListSceneData* g_current_player_list_data;
    if (g_current_player_list_data && g_current_player_list_data->current_page > 0) {
        g_current_player_list_data->current_page--;
        render_player_page(g_current_player_list_data);
    }
}

PlayerListSceneData* g_current_player_list_data = NULL;

static void player_list_scene_init(Scene* scene) {
    printf("📡 Initialisation de la scène Liste de Joueurs\n");
    
    ui_set_hitbox_visualization(false);
    
    PlayerListSceneData* data = (PlayerListSceneData*)calloc(1, sizeof(PlayerListSceneData));
    if (!data) return;
    
    data->initialized = true;
    g_current_player_list_data = data;
    
    simulate_discover_players(data);
    
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
    
    UINode* app = UI_DIV(data->ui_tree, "player-list-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "player-list-modal", 500, 450);
    
    UINode* content_parent = UI_DIV(data->ui_tree, "player-list-content");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 12);
    
    // Header
    UINode* header = UI_TEXT(data->ui_tree, "player-list-header", "JOUEURS DISPONIBLES");
    ui_set_text_color(header, "rgb(255, 165, 0)");
    ui_set_text_size(header, 18);
    ui_set_text_align(header, "center");
    ui_set_text_style(header, true, false);
    atomic_set_margin(header->element, 24, 0, 0, 0);
    
    // Players container
    data->players_container = UI_DIV(data->ui_tree, "players-list-container");
    SET_SIZE(data->players_container, 430, 200);
    ui_set_display_flex(data->players_container);
    FLEX_COLUMN(data->players_container);
    ui_set_align_items(data->players_container, "stretch");
    
    render_player_page(data);
    
    // Pagination controls
    UINode* pagination = UI_DIV(data->ui_tree, "pagination");
    SET_SIZE(pagination, 350, 35);
    ui_set_display_flex(pagination);
    ui_set_flex_direction(pagination, "row");
    ui_set_justify_content(pagination, "space-between");
    ui_set_align_items(pagination, "center");
    
    data->prev_button = ui_create_link(data->ui_tree, "prev-btn", "PRÉCÉDENT", NULL, SCENE_TRANSITION_REPLACE);
    SET_SIZE(data->prev_button, 100, 30);
    ui_set_text_align(data->prev_button, "center");
    atomic_set_background_color(data->prev_button->element, 64, 64, 64, 200);
    atomic_set_border(data->prev_button->element, 2, 128, 128, 128, 255);
    atomic_set_text_color_rgba(data->prev_button->element, 255, 255, 255, 255);
    atomic_set_padding(data->prev_button->element, 4, 6, 4, 6);
    atomic_set_click_handler(data->prev_button->element, prev_page_clicked);
    
    data->page_indicator = UI_TEXT(data->ui_tree, "page-indicator", "Page 1/3");
    ui_set_text_color(data->page_indicator, "rgb(255, 255, 255)");
    ui_set_text_size(data->page_indicator, 11);
    ui_set_text_align(data->page_indicator, "center");
    
    data->next_button = ui_create_link(data->ui_tree, "next-btn", "SUIVANT", NULL, SCENE_TRANSITION_REPLACE);
    SET_SIZE(data->next_button, 100, 30);
    ui_set_text_align(data->next_button, "center");
    atomic_set_background_color(data->next_button->element, 64, 64, 64, 200);
    atomic_set_border(data->next_button->element, 2, 128, 128, 128, 255);
    atomic_set_text_color_rgba(data->next_button->element, 255, 255, 255, 255);
    atomic_set_padding(data->next_button->element, 4, 6, 4, 6);
    atomic_set_click_handler(data->next_button->element, next_page_clicked);
    
    APPEND(pagination, data->prev_button);
    APPEND(pagination, data->page_indicator);
    APPEND(pagination, data->next_button);
    
    // Cancel button
    data->cancel_link = ui_create_link(data->ui_tree, "cancel-search", "ANNULER LA RECHERCHE", "choice", SCENE_TRANSITION_REPLACE);
    SET_SIZE(data->cancel_link, 220, 35);
    ui_set_text_align(data->cancel_link, "center");
    atomic_set_background_color(data->cancel_link->element, 128, 0, 0, 200);
    atomic_set_border(data->cancel_link->element, 2, 255, 0, 0, 255);
    atomic_set_text_color_rgba(data->cancel_link->element, 255, 255, 255, 255);
    atomic_set_padding(data->cancel_link->element, 6, 10, 6, 10);
    atomic_set_margin(data->cancel_link->element, 8, 0, 0, 0);
    ui_link_set_target_window(data->cancel_link, WINDOW_TYPE_MINI);
    
    APPEND(content_parent, header);
    APPEND(content_parent, data->players_container);
    APPEND(content_parent, pagination);
    APPEND(content_parent, data->cancel_link);
    
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_animate_fade_in(modal_container, 0.8f);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Liste Joueurs créée (%d joueurs, %d pages)\n", data->total_players, data->total_pages);
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

static void player_list_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    PlayerListSceneData* data = (PlayerListSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        if (data->cancel_link) ui_link_update(data->cancel_link, delta_time);
        if (data->prev_button) ui_link_update(data->prev_button, delta_time);
        if (data->next_button) ui_link_update(data->next_button, delta_time);
    }
}

static void player_list_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    PlayerListSceneData* data = (PlayerListSceneData*)scene->data;
    if (data->ui_tree) ui_tree_render(data->ui_tree, renderer);
}

static void player_list_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Liste Joueurs\n");
    if (!scene || !scene->data) return;
    
    PlayerListSceneData* data = (PlayerListSceneData*)scene->data;
    if (g_current_player_list_data == data) g_current_player_list_data = NULL;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
}

Scene* create_player_list_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->id = strdup("player_list");
    scene->name = strdup("Liste Joueurs");
    
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
    
    scene->init = player_list_scene_init;
    scene->update = player_list_scene_update;
    scene->render = player_list_scene_render;
    scene->cleanup = player_list_scene_cleanup;
    scene->data = NULL;
    
    printf("📡 Player list scene created\n");
    return scene;
}

void player_list_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    
    PlayerListSceneData* data = (PlayerListSceneData*)scene->data;
    if (!data) return;
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) return;
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    extern SceneManager* game_core_get_scene_manager(GameCore* core);
    SceneManager* scene_manager = game_core_get_scene_manager(core);
    
    if (scene_manager && data->cancel_link) {
        ui_link_connect_to_manager(data->cancel_link, scene_manager);
        ui_link_set_activation_delay(data->cancel_link, 0.5f);
    }
}
