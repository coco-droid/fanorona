#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../config.h"
#include "../pions/pions.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène profile
typedef struct ProfileSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    AvatarID selected_avatar;
    UINode* main_avatar;
    UINode* username_input;
    UINode* confirm_link;
    bool is_player2_turn;  // 🆕 Flag pour savoir quel joueur remplit le profil
} ProfileSceneData;

// === HELPER FUNCTIONS ===

static UINode* create_avatar_circle(UITree* tree, const char* id, AvatarID avatar_id, 
                                   int size, int border_width, bool is_main) {
    UINode* avatar = UI_DIV(tree, id);
    if (!avatar) return NULL;
    
    SET_SIZE(avatar, size, size);
    
    const char* avatar_path = avatar_id_to_filename(avatar_id);
    
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            SDL_Texture* avatar_texture = asset_load_texture(renderer, avatar_path);
            if (avatar_texture) {
                atomic_set_background_image(avatar->element, avatar_texture);
                atomic_set_background_size_str(avatar->element, "cover");
            }
        }
    }
    
    if (is_main) {
        atomic_set_border(avatar->element, border_width, 255, 215, 0, 255);
    } else {
        atomic_set_border(avatar->element, border_width, 255, 255, 255, 180);
    }
    
    return avatar;
}

// === CALLBACK UNIQUE POUR LA SÉLECTION D'AVATAR ===

static void avatar_selected(void* element, SDL_Event* event) {
    (void)event;
    
    AtomicElement* atomic = (AtomicElement*)element;
   ProfileSceneData* data = (ProfileSceneData*)atomic_get_custom_data(atomic, "parent_data");
    
    if (!data || !atomic->id) return;
    
    // Map des avatars avec leurs IDs fixes
    AvatarID avatar_map[] = {
        AVATAR_WARRIOR,     // avatar-mini-1
        AVATAR_STRATEGIST,  // avatar-mini-2
        AVATAR_DIPLOMAT,    // avatar-mini-3
        AVATAR_EXPLORER,    // avatar-mini-4
        AVATAR_MERCHANT,    // avatar-mini-5
        AVATAR_SAGE         // avatar-mini-6
    };
    
    int avatar_index = 0;
    if (sscanf(atomic->id, "avatar-mini-%d", &avatar_index) == 1) {
        if (avatar_index >= 1 && avatar_index <= 6) {
            data->selected_avatar = avatar_map[avatar_index - 1];
            
            printf("🎭 Avatar sélectionné: ID=%d (%s)\n", 
                   data->selected_avatar, avatar_id_to_filename(data->selected_avatar));
            
            // Mettre à jour l'avatar principal
            if (data->main_avatar) {
                GameWindow* window = use_mini_window();
                if (window) {
                    SDL_Renderer* renderer = window_get_renderer(window);
                    if (renderer) {
                        SDL_Texture* avatar_texture = asset_load_texture(renderer, 
                                                     avatar_id_to_filename(data->selected_avatar));
                        if (avatar_texture) {
                            atomic_set_background_image(data->main_avatar->element, avatar_texture);
                        }
                    }
                }
            }
        }
    }
}

// === CALLBACK UNIQUE POUR LE LIEN DE CONFIRMATION ===

static void on_confirm_profile(UINode* link) {
    if (!link || !link->element || !link->element->user_data) return;
    
    ProfileSceneData* data = (ProfileSceneData*)link->element->user_data;
    
    const char* username = ui_text_input_get_text_by_id("input_name");
    if (!username || strlen(username) == 0) {
        printf("❌ Nom vide - transition annulée\n");
        ui_link_set_target(link, NULL);
        return;
    }
    
    GameMode mode = config_get_mode();
    
    // 🎯 JOUEUR 1
    if (!data->is_player2_turn) {
        config_set_player1_full_profile(username, data->selected_avatar);
        printf("✅ [J1] Profil sauvegardé: '%s' (Avatar %d)\n", username, (int)data->selected_avatar);
        
        if (mode == GAME_MODE_LOCAL_MULTIPLAYER) {
            // 🔧 FIX: Ne PAS modifier l'UI ici - laisser la transition le faire
            // La scène sera détruite et recréée automatiquement
            
            // Juste configurer le lien pour rester sur profile
            ui_link_set_target(link, "profile");
            ui_link_set_transition(link, SCENE_TRANSITION_REPLACE);
            ui_link_set_target_window(link, WINDOW_TYPE_MINI);
            
            printf("🔄 Transition vers profile pour Joueur 2\n");
            return;
        }
        
        // Mode solo/IA → transition directe vers game
        ui_link_set_target(link, "game");
        ui_link_set_transition(link, SCENE_TRANSITION_CLOSE_AND_OPEN);
        ui_link_set_target_window(link, WINDOW_TYPE_MAIN);
        printf("🚀 Transition vers game (solo/IA)\n");
        return;
    }
    
    // 🎯 JOUEUR 2
    config_set_player2_full_profile(username, data->selected_avatar);
    printf("✅ [J2] Profil sauvegardé: '%s' (Avatar %d)\n", username, (int)data->selected_avatar);
    
    // Transition automatique vers game
    ui_link_set_target(link, "game");
    ui_link_set_transition(link, SCENE_TRANSITION_CLOSE_AND_OPEN);
    ui_link_set_target_window(link, WINDOW_TYPE_MAIN);
}

// === INITIALISATION DE LA SCÈNE ===

static void profile_scene_init(Scene* scene) {
    printf("📋 Initialisation de la scène Profile (architecture propre)\n");
    
    ui_set_hitbox_visualization(false);
    
    ProfileSceneData* data = (ProfileSceneData*)malloc(sizeof(ProfileSceneData));
    if (!data) return;
    
    data->initialized = true;
    data->core = NULL;
    data->main_avatar = NULL;
    data->username_input = NULL;
    data->confirm_link = NULL;
    
    // 🆕 Déterminer quel joueur remplit le profil
    data->is_player2_turn = config_is_profile_player2_turn();
    printf("🔍 [PROFILE_INIT] is_player2_turn = %s (J1_configured=%s, J2_configured=%s)\n",
           data->is_player2_turn ? "TRUE (J2)" : "FALSE (J1)",
           config_is_player1_configured() ? "YES" : "NO",
           config_is_player2_configured() ? "YES" : "NO");
    data->selected_avatar = AVATAR_WARRIOR;
    
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // === BACKGROUND ===
    SDL_Texture* background_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
        }
    }
    
    UINode* app = UI_DIV(data->ui_tree, "profile-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // === CONTAINER MODAL ===
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "profile-modal", 550, 480);
    if (!modal_container) {
        free(data);
        return;
    }
    
    // === TITRE ===
    UINode* title = ui_text(data->ui_tree, "profile-title", "CREATION DE PROFIL");
    if (title) {
        ui_set_text_color(title, "rgb(255, 215, 0)");
        ui_set_text_size(title, 24);
        ui_set_text_align(title, "center");
        ui_set_text_style(title, true, false);
        ALIGN_SELF_X(title);
        ui_animate_fade_in(title, 0.8f);
    }
    
    // === AVATAR PRINCIPAL ===
    data->main_avatar = create_avatar_circle(data->ui_tree, "avatar-main", AVATAR_WARRIOR, 45, 2, false);
    if (data->main_avatar) {
        ALIGN_SELF_X(data->main_avatar);
        ui_animate_pulse(data->main_avatar, 2.0f);
    }
    
    // === CONTAINER MINI-AVATARS ===
    UINode* avatars_container = UI_DIV(data->ui_tree, "avatars-container");
    if (avatars_container) {
        SET_SIZE(avatars_container, 400, 50);
        ui_set_display_flex(avatars_container);
        FLEX_ROW(avatars_container);
        ui_set_justify_content(avatars_container, "space-between");
        ui_set_align_items(avatars_container, "center");
        ALIGN_SELF_X(avatars_container);
        
        AvatarID avatar_ids[] = {
            AVATAR_WARRIOR, AVATAR_STRATEGIST, AVATAR_DIPLOMAT,
            AVATAR_EXPLORER, AVATAR_MERCHANT, AVATAR_SAGE
        };
        
        for (int i = 0; i < 6; i++) {
            char avatar_id[32];
            snprintf(avatar_id, sizeof(avatar_id), "avatar-mini-%d", i + 1);
            
            UINode* mini_avatar = create_avatar_circle(data->ui_tree, avatar_id, avatar_ids[i], 45, 2, false);
            if (mini_avatar) {
               atomic_set_custom_data(mini_avatar->element, "parent_data", data);
                atomic_set_click_handler(mini_avatar->element, avatar_selected);
                ui_animate_fade_in(mini_avatar, 0.5f + (i * 0.1f));
                APPEND(avatars_container, mini_avatar);
            }
        }
    }
    
    // === TEXT INPUT (conditionnel) ===
    const char* placeholder = data->is_player2_turn ? "Joueur 2" : "Joueur 1";
    
    data->username_input = ui_text_input(data->ui_tree, "username-input", placeholder);
    if (data->username_input) {
        SET_SIZE(data->username_input, 300, 35);
        ALIGN_SELF_X(data->username_input);
        ui_text_input_set_max_length(data->username_input, 20);
        
        // 🔧 FIX: Clean registry only when switching from J1 to J2
        if (data->is_player2_turn) {
            ui_text_input_cleanup_registry();
        }
        
        ui_text_input_set_scene_id(data->username_input, "input_name");
        ui_animate_fade_in(data->username_input, 1.5f);
    }
    
    // === UI LINK STYLISÉ COMME NEON BUTTON (conditionnel) ===
    const char* button_text = data->is_player2_turn ? "COMMENCER" : "SUIVANT";
    const char* target_scene = data->is_player2_turn ? "game" : "profile";
    SceneTransitionOption transition = data->is_player2_turn ? 
        SCENE_TRANSITION_CLOSE_AND_OPEN : SCENE_TRANSITION_REPLACE;
    
    data->confirm_link = ui_create_link(data->ui_tree, "confirm-link", button_text, 
                                       target_scene, transition);
    if (data->confirm_link) {
        SET_SIZE(data->confirm_link, 200, 45);
        ui_set_text_align(data->confirm_link, "center");
        ALIGN_SELF_X(data->confirm_link);
        
        // Style neon button
        atomic_set_background_color(data->confirm_link->element, 0, 64, 0, 200);
        atomic_set_border(data->confirm_link->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(data->confirm_link->element, 255, 255, 255, 255);
        
        WindowType target_window = data->is_player2_turn ? WINDOW_TYPE_MAIN : WINDOW_TYPE_MINI;
        ui_link_set_target_window(data->confirm_link, target_window);
        ui_link_set_click_handler(data->confirm_link, on_confirm_profile);
        
        data->confirm_link->element->user_data = data;
        
        ui_animate_slide_in_left(data->confirm_link, 1.0f, 200.0f);
    }
    
    // === ASSEMBLER LA HIÉRARCHIE ===
    UINode* content_column = UI_DIV(data->ui_tree, "profile-content-column");
    if (content_column) {
        SET_SIZE(content_column, 500, 420);
        ui_set_display_flex(content_column);
        FLEX_COLUMN(content_column);
        ui_set_justify_content(content_column, "flex-start");
        ui_set_align_items(content_column, "center");
        ui_set_flex_gap(content_column, 15);
        
        APPEND(content_column, title);
        APPEND(content_column, data->main_avatar);
        APPEND(content_column, avatars_container);
        APPEND(content_column, data->username_input);
        APPEND(content_column, data->confirm_link);
        
        ui_container_add_content(modal_container, content_column);
    }
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Profile scene initialisée pour %s\n", data->is_player2_turn ? "Joueur 2" : "Joueur 1");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène profile
static void profile_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    ui_update_animations(delta_time);
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        ui_neon_button_update_all(data->ui_tree, delta_time);
        if (data->confirm_link) {
            ui_link_update(data->confirm_link, delta_time);
        }
    }
}

// Rendu de la scène profile
static void profile_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène profile
static void profile_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Profile\n");
    if (!scene || !scene->data) return;
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    ui_text_input_cleanup_registry();
    free(data);
    scene->data = NULL;
    printf("✅ Nettoyage terminé\n");
}

Scene* create_profile_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    scene->id = strdup("profile");
    scene->name = strdup("Création de Profil");
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
    scene->init = profile_scene_init;
    scene->update = profile_scene_update;
    scene->render = profile_scene_render;
    scene->cleanup = profile_scene_cleanup;
    scene->data = NULL;
    return scene;
}

void profile_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
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
    if (data->confirm_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        if (scene_manager) {
            ui_link_connect_to_manager(data->confirm_link, scene_manager);
            ui_link_set_activation_delay(data->confirm_link, 0.3f);
        }
    }
}
