#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../ui/native/atomic.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../config.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène AI
typedef struct AISceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* easy_btn;
    UINode* medium_btn;
    UINode* hard_btn;
    UINode* start_game_link;
    AIDifficulty selected_difficulty;
    SDL_Renderer* last_renderer; // 🆕 Suivi du renderer
} AISceneData;

// Callbacks pour les boutons de difficulté - FIX: Signatures atomic
static void easy_difficulty_clicked(void* element, SDL_Event* event) {
    (void)element; (void)event; // 🔧 FIX: Mark unused
    config_set_ai_difficulty(AI_DIFFICULTY_EASY);
    printf("🤖 Difficulté réglée sur FACILE\n");
    // Visual feedback handled by neon button internal state or we could force update
}

static void medium_difficulty_clicked(void* element, SDL_Event* event) {
    (void)element; (void)event; // 🔧 FIX: Mark unused
    config_set_ai_difficulty(AI_DIFFICULTY_MEDIUM);
    printf("🤖 Difficulté réglée sur MOYEN\n");
}

static void hard_difficulty_clicked(void* element, SDL_Event* event) {
    (void)element; (void)event; // 🔧 FIX: Mark unused
    config_set_ai_difficulty(AI_DIFFICULTY_HARD);
    printf("🤖 Difficulté réglée sur DIFFICILE\n");
}

// Callback pour démarrer le jeu avec configuration automatique de l'IA
static void start_game_with_ai_callback(UINode* link) {
    printf("🚀 Lancement de la partie contre l'IA...\n");
    extern void ui_link_activate(UINode* link);
    ui_link_activate(link);
}

// Variable globale temporaire pour accès au callback
AISceneData* g_current_ai_scene_data = NULL;

// 🆕 Fonction helper pour construire l'UI
static void ai_scene_build_ui(Scene* scene, SDL_Renderer* renderer) {
    AISceneData* data = (AISceneData*)scene->data;
    if (!data || !renderer) return;

    printf("🏗️ Construction de l'UI AI Scene pour le renderer %p\n", (void*)renderer);

    // 🆕 FIX: Nettoyer l'event manager AVANT de détruire l'arbre
    if (scene->event_manager) {
        event_manager_clear_all(scene->event_manager);
    }

    // Nettoyer l'ancienne UI
    if (data->ui_tree) {
        ui_tree_stop_all_animations(data->ui_tree);
        data->ui_tree->event_manager = NULL;
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
        data->easy_btn = NULL;
        data->medium_btn = NULL;
        data->hard_btn = NULL;
        data->start_game_link = NULL;
    }

    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Reconnecter l'event manager
    if (scene->event_manager) {
        data->ui_tree->event_manager = scene->event_manager;
    }
    
    // Charger le background
    SDL_Texture* background_texture = asset_load_texture(renderer, "fix_bg.png");
    
    // Container principal
    UINode* app = UI_DIV(data->ui_tree, "ai-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "ai-modal-container", 500, 450);
    
    // Container de contenu principal
    UINode* content_parent = UI_DIV(data->ui_tree, "ai-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 20);
    
    // Header
    UINode* ai_header = UI_TEXT(data->ui_tree, "ai-header", "CONFIGURATION IA");
    ui_set_text_color(ai_header, "rgb(255, 165, 0)");
    ui_set_text_size(ai_header, 20);
    ui_set_text_align(ai_header, "center");
    ui_set_text_style(ai_header, true, false);
    atomic_set_margin(ai_header->element, 24, 0, 0, 0);
    
    // Section difficulté
    UINode* difficulty_section = UI_DIV(data->ui_tree, "difficulty-section");
    SET_SIZE(difficulty_section, 450, 100);
    ui_set_display_flex(difficulty_section);
    FLEX_COLUMN(difficulty_section);
    ui_set_align_items(difficulty_section, "center");
    ui_set_flex_gap(difficulty_section, 15);
    
    // Titre de section
    UINode* difficulty_title = UI_TEXT(data->ui_tree, "difficulty-title", "NIVEAU DE DIFFICULTÉ");
    ui_set_text_color(difficulty_title, "rgb(255, 165, 0)");
    ui_set_text_align(difficulty_title, "center");
    ui_set_text_size(difficulty_title, 16);
    
    // Container pour boutons de difficulté
    UINode* difficulty_buttons = UI_DIV(data->ui_tree, "difficulty-buttons");
    SET_SIZE(difficulty_buttons, 420, 45);
    ui_set_display_flex(difficulty_buttons);
    ui_set_flex_direction(difficulty_buttons, "row");
    ui_set_justify_content(difficulty_buttons, "space-around");
    ui_set_align_items(difficulty_buttons, "center");
    
    // Boutons de difficulté
    data->easy_btn = ui_neon_button(data->ui_tree, "easy-btn", "FACILE", NULL, NULL);
    SET_SIZE(data->easy_btn, 120, 40);
    ui_set_text_align(data->easy_btn, "center");
    ui_neon_button_set_glow_color(data->easy_btn, 64, 64, 64);
    atomic_set_click_handler(data->easy_btn->element, easy_difficulty_clicked);
    
    data->medium_btn = ui_neon_button(data->ui_tree, "medium-btn", "MOYEN", NULL, NULL);
    SET_SIZE(data->medium_btn, 120, 40);
    ui_set_text_align(data->medium_btn, "center");
    ui_neon_button_set_glow_color(data->medium_btn, 255, 255, 0);
    atomic_set_click_handler(data->medium_btn->element, medium_difficulty_clicked);
    
    data->hard_btn = ui_neon_button(data->ui_tree, "hard-btn", "DIFFICILE", NULL, NULL);
    SET_SIZE(data->hard_btn, 120, 40);
    ui_set_text_align(data->hard_btn, "center");
    ui_neon_button_set_glow_color(data->hard_btn, 64, 64, 64);
    atomic_set_click_handler(data->hard_btn->element, hard_difficulty_clicked);
    
    APPEND(difficulty_buttons, data->easy_btn);
    APPEND(difficulty_buttons, data->medium_btn);
    APPEND(difficulty_buttons, data->hard_btn);
    
    APPEND(difficulty_section, difficulty_title);
    APPEND(difficulty_section, difficulty_buttons);
    
    // Bouton démarrer
    data->start_game_link = ui_create_link(data->ui_tree, "start-game-link", "DÉMARRER LA PARTIE", "game", SCENE_TRANSITION_CLOSE_AND_OPEN);
    if (data->start_game_link) {
        SET_SIZE(data->start_game_link, 280, 45);
        ui_set_text_align(data->start_game_link, "center");
        // 🆕 FIX: Style doré pour bouton Démarrer
        atomic_set_background_color(data->start_game_link->element, 20, 20, 20, 220);
        atomic_set_border(data->start_game_link->element, 2, 255, 215, 0, 255);
        atomic_set_text_color_rgba(data->start_game_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->start_game_link->element, 10, 15, 10, 15);
        ui_link_set_click_handler(data->start_game_link, start_game_with_ai_callback);
        ui_link_set_target_window(data->start_game_link, WINDOW_TYPE_MAIN);
        ui_animate_pulse(data->start_game_link, 2.0f);
    }
    
    APPEND(content_parent, ai_header);
    APPEND(content_parent, difficulty_section);
    APPEND(content_parent, data->start_game_link);
    
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_animate_fade_in(modal_container, 0.8f);
    ui_animate_slide_in_left(difficulty_section, 1.0f, 200.0f);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    // Enregistrer les événements
    if (scene->event_manager) {
        ui_tree_register_all_events(data->ui_tree);
        // Enregistrer explicitement les boutons neon
        if (data->easy_btn) atomic_register_with_event_manager(data->easy_btn->element, scene->event_manager);
        if (data->medium_btn) atomic_register_with_event_manager(data->medium_btn->element, scene->event_manager);
        if (data->hard_btn) atomic_register_with_event_manager(data->hard_btn->element, scene->event_manager);
    }
    
    // Connecter le lien
    if (data->core && data->start_game_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(data->core);
        if (scene_manager) {
            ui_link_connect_to_manager(data->start_game_link, scene_manager);
        }
    }
    
    scene->ui_tree = data->ui_tree;
}

// Initialisation de la scène AI
static void ai_scene_init(Scene* scene) {
    printf("🤖 Initialisation de la scène Configuration IA\n");
    
    ui_set_hitbox_visualization(false);
    
    AISceneData* data = (AISceneData*)malloc(sizeof(AISceneData));
    if (!data) return;
    
    data->initialized = true;
    data->core = NULL;
    data->selected_difficulty = AI_DIFFICULTY_MEDIUM;
    data->last_renderer = NULL;
    data->ui_tree = NULL;
    
    g_current_ai_scene_data = data;
    scene->data = data;
    
    GameWindow* window = use_mini_window();
    if (window && window->renderer) {
        ai_scene_build_ui(scene, window->renderer);
        data->last_renderer = window->renderer;
    }
}

// Mise à jour de la scène AI
static void ai_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    AISceneData* data = (AISceneData*)scene->data;
    
    // Mettre à jour les animations
    ui_update_animations(delta_time);
    
    // Mettre à jour l'arbre UI
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        ui_neon_button_update_all(data->ui_tree, delta_time);
        
        if (data->start_game_link) {
            ui_link_update(data->start_game_link, delta_time);
        }
    }
}

// Rendu de la scène AI
static void ai_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    AISceneData* data = (AISceneData*)scene->data;
    
    if (renderer != data->last_renderer) {
        ai_scene_build_ui(scene, renderer);
        data->last_renderer = renderer;
    }
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène AI
static void ai_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Configuration IA\n");
    if (!scene || !scene->data) return;
    
    AISceneData* data = (AISceneData*)scene->data;
    
    // 🆕 FIX: Clear event manager to prevent dangling pointers
    if (scene->event_manager) {
        event_manager_clear_all(scene->event_manager);
    }
    
    if (g_current_ai_scene_data == data) {
        g_current_ai_scene_data = NULL;
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = NULL;
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    scene->initialized = false; // Force re-init
    
    printf("✅ Nettoyage de la scène Configuration IA terminé\n");
}

// Créer la scène AI
Scene* create_ai_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->id = strdup("ai");
    scene->name = strdup("Configuration IA");
    
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
    
    scene->init = ai_scene_init;
    scene->update = ai_scene_update;
    scene->render = ai_scene_render;
    scene->cleanup = ai_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

// Connexion des événements - 🔧 FIX: Améliorer l'enregistrement des événements
void ai_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    
    AISceneData* data = (AISceneData*)scene->data;
    if (!data) return;
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        
        // Enregistrer explicitement les boutons neon
        if (data->easy_btn) atomic_register_with_event_manager(data->easy_btn->element, scene->event_manager);
        if (data->medium_btn) atomic_register_with_event_manager(data->medium_btn->element, scene->event_manager);
        if (data->hard_btn) atomic_register_with_event_manager(data->hard_btn->element, scene->event_manager);
        
        scene->ui_tree = data->ui_tree;
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    if (data->start_game_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        if (scene_manager) {
            ui_link_connect_to_manager(data->start_game_link, scene_manager);
        }
    }
}
