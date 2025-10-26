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

// Données pour la scène Pieces
typedef struct PiecesSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* black_button;
    UINode* brown_button;
    UINode* back_link;
    UINode* next_link;  // 🆕 Ajout du bouton SUIVANT
} PiecesSceneData;

// 🔧 FIX: Callbacks COMPLETS pour sauvegarder les couleurs
static void black_pieces_clicked(void* element, SDL_Event* event) {
    (void)element;
    (void)event;
    
    printf("🖤 Joueur 1 choisit les pièces NOIRES\n");
    printf("🤎 Joueur 2 aura automatiquement les pièces BRUNES\n");
    
    // 🆕 SAUVEGARDER dans la config globale
    config_set_player_piece_colors(PIECE_COLOR_BLACK, PIECE_COLOR_BROWN);
    
    printf("💾 Couleurs sauvegardées: J1=NOIR, J2=BRUN\n");
}

static void brown_pieces_clicked(void* element, SDL_Event* event) {
    (void)element;
    (void)event;
    
    printf("🤎 Joueur 1 choisit les pièces BRUNES\n");
    printf("🖤 Joueur 2 aura automatiquement les pièces NOIRES\n");
    
    // 🆕 SAUVEGARDER dans la config globale  
    config_set_player_piece_colors(PIECE_COLOR_BROWN, PIECE_COLOR_BLACK);
    
    printf("💾 Couleurs sauvegardées: J1=BRUN, J2=NOIR\n");
}

// Initialisation de la scène Pieces
static void pieces_scene_init(Scene* scene) {
    printf("🎨 Initialisation de la scène Choix de Pièces\n");
    
    ui_set_hitbox_visualization(false);
    
    PiecesSceneData* data = (PiecesSceneData*)malloc(sizeof(PiecesSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour PiecesSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->black_button = NULL;
    data->brown_button = NULL;
    data->back_link = NULL;
    data->next_link = NULL;  // 🆕 Initialiser next_link
    
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
    UINode* app = UI_DIV(data->ui_tree, "pieces-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal centré
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "pieces-modal", 500, 450);
    
    // Conteneur parent pour tout le contenu
    UINode* content_parent = UI_DIV(data->ui_tree, "pieces-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 20);
    
    // Header
    UINode* pieces_header = UI_TEXT(data->ui_tree, "pieces-header", "CHOISIR LES PIÈCES");
    ui_set_text_color(pieces_header, "rgb(255, 165, 0)");
    ui_set_text_size(pieces_header, 20);
    ui_set_text_align(pieces_header, "center");
    ui_set_text_style(pieces_header, true, false);
    atomic_set_margin(pieces_header->element, 24, 0, 0, 0);
    
    // Container pour les boutons de pièces
    UINode* pieces_container = UI_DIV(data->ui_tree, "pieces-buttons-container");
    SET_SIZE(pieces_container, 350, 180);
    ui_set_display_flex(pieces_container);
    FLEX_COLUMN(pieces_container);
    ui_set_justify_content(pieces_container, "center");
    ui_set_align_items(pieces_container, "center");
    ui_set_flex_gap(pieces_container, 20);
    
    // Bouton pièces noires
    data->black_button = ui_create_link(data->ui_tree, "black-pieces-link", "PIÈCES NOIRES", NULL, SCENE_TRANSITION_REPLACE);
    if (data->black_button) {
        SET_SIZE(data->black_button, 300, 50);
        ui_set_text_align(data->black_button, "center");
        atomic_set_background_color(data->black_button->element, 30, 30, 30, 200);
        atomic_set_border(data->black_button->element, 2, 100, 100, 100, 255);
        atomic_set_text_color_rgba(data->black_button->element, 255, 255, 255, 255);
        atomic_set_padding(data->black_button->element, 10, 15, 10, 15);
        
        // 🔧 FIX: Ajouter un callback personnalisé au lieu d'une transition directe
        atomic_set_click_handler(data->black_button->element, black_pieces_clicked);
        
        ui_animate_slide_in_left(data->black_button, 0.8f, 200.0f);
        APPEND(pieces_container, data->black_button);
    }
    
    // Bouton pièces brunes
    data->brown_button = ui_create_link(data->ui_tree, "brown-pieces-link", "PIÈCES BRUNES", NULL, SCENE_TRANSITION_REPLACE);
    if (data->brown_button) {
        SET_SIZE(data->brown_button, 300, 50);
        ui_set_text_align(data->brown_button, "center");
        atomic_set_background_color(data->brown_button->element, 139, 69, 19, 200);
        atomic_set_border(data->brown_button->element, 2, 160, 82, 45, 255);
        atomic_set_text_color_rgba(data->brown_button->element, 255, 255, 255, 255);
        atomic_set_padding(data->brown_button->element, 10, 15, 10, 15);
        
        // 🔧 FIX: Ajouter un callback personnalisé au lieu d'une transition directe
        atomic_set_click_handler(data->brown_button->element, brown_pieces_clicked);
        
        ui_animate_slide_in_right(data->brown_button, 1.0f, 200.0f);
        APPEND(pieces_container, data->brown_button);
    }
    
    // 🆕 Bouton SUIVANT pour aller au jeu
    data->next_link = ui_create_link(data->ui_tree, "next-game-link", "SUIVANT", "game", SCENE_TRANSITION_CLOSE_AND_OPEN);
    if (data->next_link) {
        SET_SIZE(data->next_link, 200, 45);
        ui_set_text_align(data->next_link, "center");
        atomic_set_background_color(data->next_link->element, 0, 64, 128, 200);
        atomic_set_border(data->next_link->element, 2, 0, 150, 255, 255);
        atomic_set_text_color_rgba(data->next_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->next_link->element, 10, 15, 10, 15);
        ui_link_set_target_window(data->next_link, WINDOW_TYPE_MAIN);
        
        // Animation pulse pour attirer l'attention
        ui_animate_pulse(data->next_link, 2.0f);
    }
    
    // 🆕 BOUTON RETOUR vers profile_scene
    data->back_link = ui_create_link(data->ui_tree, "back-profile-link", "RETOUR", "profile", SCENE_TRANSITION_REPLACE);
    if (data->back_link) {
        SET_SIZE(data->back_link, 150, 35);
        ui_set_text_align(data->back_link, "center");
        atomic_set_background_color(data->back_link->element, 64, 64, 64, 200);
        atomic_set_border(data->back_link->element, 2, 128, 128, 128, 255);
        atomic_set_text_color_rgba(data->back_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->back_link->element, 6, 10, 6, 10);
        ui_link_set_target_window(data->back_link, WINDOW_TYPE_MINI);
    }
    
    // Container pour les boutons de navigation (RETOUR + SUIVANT)
    UINode* nav_container = UI_DIV(data->ui_tree, "nav-buttons-container");
    SET_SIZE(nav_container, 350, 50);
    ui_set_display_flex(nav_container);
    FLEX_ROW(nav_container);
    ui_set_justify_content(nav_container, "space-between");
    ui_set_align_items(nav_container, "center");
    
    APPEND(nav_container, data->back_link);
    APPEND(nav_container, data->next_link);
    
    // Assembler dans le conteneur parent
    APPEND(content_parent, pieces_header);
    APPEND(content_parent, pieces_container);
    APPEND(content_parent, nav_container);  // 🔧 FIX: Utiliser nav_container au lieu d'ajouter back_link directement
    
    // Ajouter au modal
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    // Hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Animation d'entrée
    ui_animate_fade_in(modal_container, 0.8f);
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Pieces créée avec choix noir/brun\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène Pieces
static void pieces_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        if (data->black_button) {
            ui_link_update(data->black_button, delta_time);
        }
        
        if (data->brown_button) {
            ui_link_update(data->brown_button, delta_time);
        }
        
        if (data->back_link) {
            ui_link_update(data->back_link, delta_time);
        }
        
        if (data->next_link) {  // 🆕 Mettre à jour le bouton SUIVANT
            ui_link_update(data->next_link, delta_time);
        }
    }
}

// Rendu de la scène Pieces
static void pieces_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène Pieces
static void pieces_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Pieces\n");
    if (!scene || !scene->data) return;
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Pieces terminé\n");
}

// Créer la scène Pieces
Scene* create_pieces_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Pieces\n");
        return NULL;
    }
    
    scene->id = strdup("pieces");
    scene->name = strdup("Choix de Pièces");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène Pieces\n");
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
    
    scene->init = pieces_scene_init;
    scene->update = pieces_scene_update;
    scene->render = pieces_scene_render;
    scene->cleanup = pieces_scene_cleanup;
    scene->data = NULL;
    
    printf("🎨 Pieces scene created\n");
    return scene;
}

// Connexion des événements
void pieces_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans pieces_scene_connect_events\n");
        return;
    }
    
    PiecesSceneData* data = (PiecesSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    // 🔧 FIX: S'assurer que l'EventManager est créé AVANT d'enregistrer les éléments
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène Pieces\n");
            return;
        }
    }
    
    // 🔧 FIX: Connecter l'EventManager à l'UITree AVANT l'enregistrement
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        printf("🔗 EventManager connecté à la scène Pieces\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    // Connecter les liens
    extern SceneManager* game_core_get_scene_manager(GameCore* core);
    SceneManager* scene_manager = game_core_get_scene_manager(core);
    
    if (scene_manager) {
        if (data->black_button) {
            ui_link_connect_to_manager(data->black_button, scene_manager);
            printf("🔗 Lien 'Pièces Noires' connecté au SceneManager\n");
        }
        
        if (data->brown_button) {
            ui_link_connect_to_manager(data->brown_button, scene_manager);
            printf("🔗 Lien 'Pièces Brunes' connecté au SceneManager\n");
        }
        
        if (data->back_link) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
            printf("🔗 Lien 'Retour' connecté au SceneManager\n");
        }
        
        // 🆕 Connecter le bouton SUIVANT
        if (data->next_link) {
            ui_link_connect_to_manager(data->next_link, scene_manager);
            printf("🔗 Lien 'SUIVANT' connecté au SceneManager (transition CLOSE_AND_OPEN vers game)\n");
        }
    }
    
    printf("✅ Scène Pieces prête avec événements connectés et bouton SUIVANT\n");
}
