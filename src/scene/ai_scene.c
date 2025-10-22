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

// Données pour la scène AI
typedef struct AISceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* easy_btn;
    UINode* medium_btn;
    UINode* hard_btn;
    UINode* ai_white_btn;
    UINode* ai_black_btn;
    UINode* start_game_link;
    AIDifficulty selected_difficulty;
    bool ai_plays_white;
} AISceneData;

// Callbacks pour les boutons de difficulté - FIX: Signatures atomic
static void easy_difficulty_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("🟢 Difficulté FACILE sélectionnée\n");
    
    data->selected_difficulty = AI_DIFFICULTY_EASY;
    config_set_ai_difficulty(AI_DIFFICULTY_EASY);
    
    // Feedback visuel
    ui_neon_button_set_glow_color(data->easy_btn, 0, 255, 0);
    ui_neon_button_set_glow_color(data->medium_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->hard_btn, 64, 64, 64);
    
    (void)event;
}

static void medium_difficulty_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("🟡 Difficulté MOYENNE sélectionnée\n");
    
    data->selected_difficulty = AI_DIFFICULTY_MEDIUM;
    config_set_ai_difficulty(AI_DIFFICULTY_MEDIUM);
    
    // Feedback visuel
    ui_neon_button_set_glow_color(data->easy_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->medium_btn, 255, 255, 0);
    ui_neon_button_set_glow_color(data->hard_btn, 64, 64, 64);
    
    (void)event;
}

static void hard_difficulty_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("🔴 Difficulté DIFFICILE sélectionnée\n");
    
    data->selected_difficulty = AI_DIFFICULTY_HARD;
    config_set_ai_difficulty(AI_DIFFICULTY_HARD);
    
    // Feedback visuel
    ui_neon_button_set_glow_color(data->easy_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->medium_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->hard_btn, 255, 0, 0);
    
    (void)event;
}

// Callbacks pour la couleur de l'IA - FIX: Signatures atomic
static void ai_white_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("⚪ IA joue en BLANC sélectionné\n");
    
    data->ai_plays_white = true;
    config_set_ai_color(true);
    config_set_player_names("Joueur", "IA");
    
    // Feedback visuel
    ui_neon_button_set_glow_color(data->ai_white_btn, 255, 255, 255);
    ui_neon_button_set_glow_color(data->ai_black_btn, 64, 64, 64);
    
    (void)event;
}

static void ai_black_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("⚫ IA joue en NOIR sélectionné\n");
    
    data->ai_plays_white = false;
    config_set_ai_color(false);
    config_set_player_names("Joueur", "IA");
    
    // Feedback visuel
    ui_neon_button_set_glow_color(data->ai_white_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->ai_black_btn, 0, 0, 0);
    
    (void)event;
}

// Callback personnalisé pour démarrer le jeu avec l'IA - SANS INITIALISATION AUTOMATIQUE
static void start_game_with_ai_callback(UINode* link) {
    (void)link;  // 🔧 FIX: Mark as unused
    
    printf("🚀 Démarrage du jeu - transition vers game_scene...\n");
    printf("✅ Transition vers game_scene demandée\n");
    printf("   📋 Aucune configuration forcée - respect des choix utilisateur\n");
    printf("   🎯 Transition vers game_scene en cours...\n");
}

// Initialisation de la scène AI
static void ai_scene_init(Scene* scene) {
    printf("🤖 Initialisation de la scène Configuration IA\n");
    
    // Désactiver la visualisation des hitboxes
    ui_set_hitbox_visualization(false);
    
    AISceneData* data = (AISceneData*)malloc(sizeof(AISceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour AISceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->selected_difficulty = AI_DIFFICULTY_MEDIUM;
    data->ai_plays_white = false;
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Charger le background (identique à menu_scene)
    SDL_Texture* background_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
        }
    }
    
    // Container principal (identique à menu_scene)
    UINode* app = UI_DIV(data->ui_tree, "ai-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal avec header (comme menu_scene)
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "ai-modal-container", 500, 420);
    ui_container_add_header(modal_container, "CONFIGURATION IA");
    
    // Container pour les boutons (comme menu_scene mais plus grand)
    UINode* buttons_container = UI_DIV(data->ui_tree, "ai-buttons-container");
    SET_SIZE(buttons_container, 450, 280); // Plus de hauteur pour éviter chevauchements
    
    // Configuration flexbox identique à menu_scene
    ui_set_display_flex(buttons_container);
    FLEX_COLUMN(buttons_container);
    ui_set_justify_content(buttons_container, "space-around"); // Espacement uniforme
    ui_set_align_items(buttons_container, "center");
    ui_set_flex_gap(buttons_container, 25); // Plus d'espace entre sections
    
    // === SECTION DIFFICULTÉ (style menu_scene) ===
    UINode* difficulty_section = UI_DIV(data->ui_tree, "difficulty-section");
    SET_SIZE(difficulty_section, 450, 70); // Hauteur réduite
    ui_set_display_flex(difficulty_section);
    FLEX_COLUMN(difficulty_section);
    ui_set_align_items(difficulty_section, "center");
    ui_set_flex_gap(difficulty_section, 8);
    
    // Titre de section (style amélioré)
    UINode* difficulty_title = UI_TEXT(data->ui_tree, "difficulty-title", "NIVEAU DE DIFFICULTÉ");
    ui_set_text_color(difficulty_title, "rgb(255, 165, 0)"); // Orange comme headers
    ui_set_text_align(difficulty_title, "center");
    
    // Container pour boutons de difficulté (style menu_scene)
    UINode* difficulty_buttons = UI_DIV(data->ui_tree, "difficulty-buttons");
    SET_SIZE(difficulty_buttons, 420, 45); // Largeur ajustée
    ui_set_display_flex(difficulty_buttons);
    ui_set_flex_direction(difficulty_buttons, "row");
    ui_set_justify_content(difficulty_buttons, "space-around");
    ui_set_align_items(difficulty_buttons, "center");
    
    // Boutons de difficulté (style neon_button comme menu_scene)
    data->easy_btn = ui_neon_button(data->ui_tree, "easy-btn", "FACILE", NULL, NULL);
    SET_SIZE(data->easy_btn, 120, 40);
    ui_set_text_align(data->easy_btn, "center"); // FIX: Centrage du texte
    ui_neon_button_set_glow_color(data->easy_btn, 64, 64, 64);
    atomic_set_click_handler(data->easy_btn->element, easy_difficulty_clicked);
    data->easy_btn->element->user_data = data;
    
    data->medium_btn = ui_neon_button(data->ui_tree, "medium-btn", "MOYEN", NULL, NULL);
    SET_SIZE(data->medium_btn, 120, 40);
    ui_set_text_align(data->medium_btn, "center"); // FIX: Centrage du texte
    ui_neon_button_set_glow_color(data->medium_btn, 255, 255, 0);
    atomic_set_click_handler(data->medium_btn->element, medium_difficulty_clicked);
    data->medium_btn->element->user_data = data;
    
    data->hard_btn = ui_neon_button(data->ui_tree, "hard-btn", "DIFFICILE", NULL, NULL);
    SET_SIZE(data->hard_btn, 120, 40);
    ui_set_text_align(data->hard_btn, "center"); // FIX: Centrage du texte
    ui_neon_button_set_glow_color(data->hard_btn, 64, 64, 64);
    atomic_set_click_handler(data->hard_btn->element, hard_difficulty_clicked);
    data->hard_btn->element->user_data = data;
    
    // === SECTION COULEUR (style menu_scene) ===
    UINode* color_section = UI_DIV(data->ui_tree, "color-section");
    SET_SIZE(color_section, 450, 70); // Hauteur réduite
    ui_set_display_flex(color_section);
    FLEX_COLUMN(color_section);
    ui_set_align_items(color_section, "center");
    ui_set_flex_gap(color_section, 8);
    
    // Titre de section
    UINode* color_title = UI_TEXT(data->ui_tree, "color-title", "COULEUR DE L'IA");
    ui_set_text_color(color_title, "rgb(255, 165, 0)"); // Orange comme headers
    ui_set_text_align(color_title, "center");
    
    // Container pour boutons de couleur
    UINode* color_buttons = UI_DIV(data->ui_tree, "color-buttons");
    SET_SIZE(color_buttons, 280, 45);
    ui_set_display_flex(color_buttons);
    ui_set_flex_direction(color_buttons, "row");
    ui_set_justify_content(color_buttons, "space-around");
    ui_set_align_items(color_buttons, "center");
    
    // Boutons de couleur (style neon_button)
    data->ai_white_btn = ui_neon_button(data->ui_tree, "ai-white-btn", "BLANC", NULL, NULL);
    SET_SIZE(data->ai_white_btn, 120, 40);
    ui_set_text_align(data->ai_white_btn, "center"); // FIX: Centrage du texte
    ui_neon_button_set_glow_color(data->ai_white_btn, 64, 64, 64);
    atomic_set_click_handler(data->ai_white_btn->element, ai_white_clicked);
    data->ai_white_btn->element->user_data = data;
    
    data->ai_black_btn = ui_neon_button(data->ui_tree, "ai-black-btn", "NOIR", NULL, NULL);
    SET_SIZE(data->ai_black_btn, 120, 40);
    ui_set_text_align(data->ai_black_btn, "center"); // FIX: Centrage du texte
    ui_neon_button_set_glow_color(data->ai_black_btn, 0, 0, 0);
    atomic_set_click_handler(data->ai_black_btn->element, ai_black_clicked);
    data->ai_black_btn->element->user_data = data;
    
    // Assembler les sections
    APPEND(difficulty_buttons, data->easy_btn);
    APPEND(difficulty_buttons, data->medium_btn);
    APPEND(difficulty_buttons, data->hard_btn);
    APPEND(difficulty_section, difficulty_title);
    APPEND(difficulty_section, difficulty_buttons);
    
    APPEND(color_buttons, data->ai_white_btn);
    APPEND(color_buttons, data->ai_black_btn);
    APPEND(color_section, color_title);
    APPEND(color_section, color_buttons);
    
    // === BOUTON DÉMARRER (style menu_scene avec UI_LINK) ===
    data->start_game_link = ui_create_link(data->ui_tree, "start-game-link", "DÉMARRER LA PARTIE", "game", SCENE_TRANSITION_CLOSE_AND_OPEN);
    if (data->start_game_link) {
        SET_SIZE(data->start_game_link, 280, 45); // Taille consistante
        ui_set_text_align(data->start_game_link, "center");
        
        // Style neon vert (comme menu_scene)
        atomic_set_background_color(data->start_game_link->element, 0, 64, 0, 200);
        atomic_set_border(data->start_game_link->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(data->start_game_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->start_game_link->element, 10, 15, 10, 15);
        
        ui_link_set_click_handler(data->start_game_link, start_game_with_ai_callback);
        ui_link_set_target_window(data->start_game_link, WINDOW_TYPE_MAIN);
        
        // Animation pulse
        ui_animate_pulse(data->start_game_link, 2.0f);
    }
    
    // Assembler le contenu (style menu_scene)
    APPEND(buttons_container, difficulty_section);
    APPEND(buttons_container, color_section);
    APPEND(buttons_container, data->start_game_link);
    
    // Ajouter au modal avec centrage (style menu_scene)
    ui_container_add_content(modal_container, buttons_container);
    ALIGN_SELF_Y(buttons_container); // Centrage vertical
    
    // Construire la hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Animations d'entrée (style menu_scene)
    ui_animate_fade_in(modal_container, 0.8f);
    ui_animate_slide_in_left(difficulty_section, 1.0f, 200.0f);
    ui_animate_slide_in_right(color_section, 1.2f, 200.0f);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Configuration IA créée avec style menu_scene\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
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
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène AI
static void ai_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Configuration IA\n");
    if (!scene || !scene->data) return;
    
    AISceneData* data = (AISceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Configuration IA terminé\n");
}

// Créer la scène AI
Scene* create_ai_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène AI\n");
        return NULL;
    }
    
    scene->id = strdup("ai");
    scene->name = strdup("Configuration IA");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène AI\n");
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
    
    printf("🤖 AI Configuration scene created\n");
    return scene;
}

// Connexion des événements (style menu_scene)
void ai_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans ai_scene_connect_events\n");
        return;
    }
    
    AISceneData* data = (AISceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    // Créer un EventManager dédié (comme menu_scene)
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène AI\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène AI\n");
            return;
        }
    }
    
    // Connecter l'EventManager à l'UITree (comme menu_scene)
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        
        // Enregistrer tous les éléments UI
        ui_tree_register_all_events(data->ui_tree);
        
        // Stocker l'UITree dans la scène
        scene->ui_tree = data->ui_tree;
        
        printf("🔗 EventManager dédié connecté à la scène AI\n");
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Marquer comme initialisé et actif (comme menu_scene)
    scene->initialized = true;
    scene->active = true;
    
    // Connecter le lien de démarrage au SceneManager (comme menu_scene)
    if (data->start_game_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->start_game_link, scene_manager);
            ui_link_set_activation_delay(data->start_game_link, 0.5f);
            printf("🔗 Lien 'Démarrer' connecté au SceneManager\n");
        }
    }
    
    printf("✅ Scène Configuration IA prête avec système d'événements complet\n");
}
