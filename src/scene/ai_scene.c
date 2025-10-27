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
    UINode* start_game_link;
    AIDifficulty selected_difficulty;
} AISceneData;

// Callbacks pour les boutons de difficulté - FIX: Signatures atomic
static void easy_difficulty_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("🟢 Difficulté FACILE sélectionnée - AUCUNE TRANSITION\n");
    
    data->selected_difficulty = AI_DIFFICULTY_EASY;
    // 🔧 FIX: NE PAS APPELER config_set_ai_difficulty ici
    
    // Feedback visuel uniquement
    ui_neon_button_set_glow_color(data->easy_btn, 0, 255, 0);
    ui_neon_button_set_glow_color(data->medium_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->hard_btn, 64, 64, 64);
    
    (void)event;
}

static void medium_difficulty_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("🟡 Difficulté MOYENNE sélectionnée - AUCUNE TRANSITION\n");
    
    data->selected_difficulty = AI_DIFFICULTY_MEDIUM;
    // 🔧 FIX: NE PAS APPELER config_set_ai_difficulty ici
    
    // Feedback visuel uniquement
    ui_neon_button_set_glow_color(data->easy_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->medium_btn, 255, 255, 0);
    ui_neon_button_set_glow_color(data->hard_btn, 64, 64, 64);
    
    (void)event;
}

static void hard_difficulty_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    AISceneData* data = (AISceneData*)atomic_element->user_data;
    printf("🔴 Difficulté DIFFICILE sélectionnée - AUCUNE TRANSITION\n");
    
    data->selected_difficulty = AI_DIFFICULTY_HARD;
    // 🔧 FIX: NE PAS APPELER config_set_ai_difficulty ici
    
    // Feedback visuel uniquement
    ui_neon_button_set_glow_color(data->easy_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->medium_btn, 64, 64, 64);
    ui_neon_button_set_glow_color(data->hard_btn, 255, 0, 0);
    
    (void)event;
}

// Callback pour démarrer le jeu avec configuration automatique de l'IA
static void start_game_with_ai_callback(UINode* link) {
    (void)link;
    
    // 🔧 FIX: Récupérer la difficulté depuis AISceneData au lieu de config
    extern AISceneData* g_current_ai_scene_data;  // Hack temporaire
    AIDifficulty selected_difficulty = AI_DIFFICULTY_MEDIUM;  // Défaut
    
    if (g_current_ai_scene_data) {
        selected_difficulty = g_current_ai_scene_data->selected_difficulty;
    }
    
    // 🆕 MAINTENANT sauvegarder la difficulté dans config
    config_set_ai_difficulty(selected_difficulty);
    printf("🎯 Difficulté confirmée et sauvegardée: %s\n", config_difficulty_to_string(selected_difficulty));
    
    // 🤖 Configuration automatique de l'IA avec couleur opposée
    PieceColor player1_color = config_get_player1_piece_color();
    PieceColor ai_color = (player1_color == PIECE_COLOR_BLACK) ? PIECE_COLOR_WHITE : PIECE_COLOR_BLACK;
    
    // 🆕 Générer un avatar différent du joueur 1
    AvatarID player1_avatar = config_get_player1_avatar();
    AvatarID ai_avatar;
    
    do {
        ai_avatar = (AvatarID)(1 + (rand() % 6));
    } while (ai_avatar == player1_avatar && ai_avatar != AVATAR_SAGE); // Éviter même avatar
    
    // Si tous les avatars sont pris, forcer SAGE
    if (ai_avatar == player1_avatar) {
        ai_avatar = AVATAR_SAGE;
    }
    
    // Configurer le profil IA
    config_set_player2_full_profile("Computer", ai_avatar);
    config_set_player_piece_colors(player1_color, ai_color);
    
    printf("🤖 IA configurée automatiquement:\n");
    printf("   👤 Joueur: %s (%s, Avatar %d)\n", 
           config_get_player1_name(), 
           piece_color_to_string(player1_color), 
           player1_avatar);
    printf("   🤖 IA: Computer (%s, Avatar %d)\n", 
           piece_color_to_string(ai_color), 
           ai_avatar);
    printf("   🎯 Difficulté: %s\n", config_difficulty_to_string(selected_difficulty));
    printf("   ✅ Joueur 2 (IA) complètement configuré\n");
    
    printf("🚀 Transition vers game_scene...\n");
}

// Variable globale temporaire pour accès au callback
AISceneData* g_current_ai_scene_data = NULL;

// Initialisation de la scène AI
static void ai_scene_init(Scene* scene) {
    printf("🤖 Initialisation de la scène Configuration IA\n");
    
    ui_set_hitbox_visualization(false);
    
    AISceneData* data = (AISceneData*)malloc(sizeof(AISceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour AISceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->selected_difficulty = AI_DIFFICULTY_MEDIUM;
    
    // 🔧 FIX: Stocker globalement pour accès callback
    g_current_ai_scene_data = data;
    
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
    
    // Container modal - 🔧 FIX: Utiliser les MÊMES dimensions que menu_scene
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "ai-modal-container", 500, 450);
    
    // Container de contenu principal - 🔧 FIX: Ajuster pour 450px height
    UINode* content_parent = UI_DIV(data->ui_tree, "ai-content-parent");
    SET_SIZE(content_parent, 450, 350);  // 🔧 FIX: Augmenter de 280 → 350 pour container 450px
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 20);  // 🔧 FIX: Restaurer gap de 20 avec plus d'espace
    
    // Header "CONFIGURATION IA" - 🔧 FIX: Restaurer marge normale
    UINode* ai_header = UI_TEXT(data->ui_tree, "ai-header", "CONFIGURATION IA");
    ui_set_text_color(ai_header, "rgb(255, 165, 0)");
    ui_set_text_size(ai_header, 20);
    ui_set_text_align(ai_header, "center");
    ui_set_text_style(ai_header, true, false);
    atomic_set_margin(ai_header->element, 24, 0, 0, 0);  // 🔧 FIX: Restaurer marge 24px
    
    // Section difficulté - 🔧 FIX: Restaurer taille normale
    UINode* difficulty_section = UI_DIV(data->ui_tree, "difficulty-section");
    SET_SIZE(difficulty_section, 450, 100);  // 🔧 FIX: Restaurer 100px height
    ui_set_display_flex(difficulty_section);
    FLEX_COLUMN(difficulty_section);
    ui_set_align_items(difficulty_section, "center");
    ui_set_flex_gap(difficulty_section, 15);  // 🔧 FIX: Restaurer gap 15px
    
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
    data->easy_btn->element->user_data = data;
    
    data->medium_btn = ui_neon_button(data->ui_tree, "medium-btn", "MOYEN", NULL, NULL);
    SET_SIZE(data->medium_btn, 120, 40);
    ui_set_text_align(data->medium_btn, "center");
    ui_neon_button_set_glow_color(data->medium_btn, 255, 255, 0); // Déjà sélectionné par défaut
    atomic_set_click_handler(data->medium_btn->element, medium_difficulty_clicked);
    data->medium_btn->element->user_data = data;
    
    data->hard_btn = ui_neon_button(data->ui_tree, "hard-btn", "DIFFICILE", NULL, NULL);
    SET_SIZE(data->hard_btn, 120, 40);
    ui_set_text_align(data->hard_btn, "center");
    ui_neon_button_set_glow_color(data->hard_btn, 64, 64, 64);
    atomic_set_click_handler(data->hard_btn->element, hard_difficulty_clicked);
    data->hard_btn->element->user_data = data;
    
    // Assembler les boutons
    APPEND(difficulty_buttons, data->easy_btn);
    APPEND(difficulty_buttons, data->medium_btn);
    APPEND(difficulty_buttons, data->hard_btn);
    
    // Assembler la section
    APPEND(difficulty_section, difficulty_title);
    APPEND(difficulty_section, difficulty_buttons);
    
    // Bouton démarrer - 🔧 FIX: Restaurer taille normale
    data->start_game_link = ui_create_link(data->ui_tree, "start-game-link", "DÉMARRER LA PARTIE", "game", SCENE_TRANSITION_CLOSE_AND_OPEN);
    if (data->start_game_link) {
        SET_SIZE(data->start_game_link, 280, 45);  // 🔧 FIX: Restaurer taille originale
        ui_set_text_align(data->start_game_link, "center");
        
        atomic_set_background_color(data->start_game_link->element, 0, 64, 0, 200);
        atomic_set_border(data->start_game_link->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(data->start_game_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->start_game_link->element, 10, 15, 10, 15);  // 🔧 FIX: Restaurer padding
        
        ui_link_set_click_handler(data->start_game_link, start_game_with_ai_callback);
        ui_link_set_target_window(data->start_game_link, WINDOW_TYPE_MAIN);
        
        ui_animate_pulse(data->start_game_link, 2.0f);
    }
    
    // Assembler le contenu principal
    APPEND(content_parent, ai_header);
    APPEND(content_parent, difficulty_section);
    APPEND(content_parent, data->start_game_link);
    
    // Ajouter au modal
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    // Construire la hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Animations d'entrée
    ui_animate_fade_in(modal_container, 0.8f);
    ui_animate_slide_in_left(difficulty_section, 1.0f, 200.0f);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Configuration IA créée avec dimensions identiques au menu (500x450)\n");
    
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
    
    // 🔧 FIX: Nettoyer référence globale
    if (g_current_ai_scene_data == data) {
        g_current_ai_scene_data = NULL;
    }
    
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

// Connexion des événements - 🔧 FIX: Améliorer l'enregistrement des événements
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
    
    // 🔧 FIX CRITIQUE: Connecter l'EventManager à l'UITree AVANT l'enregistrement
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        
        // 🔧 FIX: Enregistrer EXPLICITEMENT les boutons neon avec l'EventManager
        if (data->easy_btn && data->easy_btn->element) {
            atomic_register_with_event_manager(data->easy_btn->element, scene->event_manager);
            printf("🔗 Bouton FACILE enregistré dans EventManager\n");
        }
        
        if (data->medium_btn && data->medium_btn->element) {
            atomic_register_with_event_manager(data->medium_btn->element, scene->event_manager);
            printf("🔗 Bouton MOYEN enregistré dans EventManager\n");
        }
        
        if (data->hard_btn && data->hard_btn->element) {
            atomic_register_with_event_manager(data->hard_btn->element, scene->event_manager);
            printf("🔗 Bouton DIFFICILE enregistré dans EventManager\n");
        }
        
        // Enregistrer tous les autres éléments UI
        ui_tree_register_all_events(data->ui_tree);
        
        // Stocker l'UITree dans la scène
        scene->ui_tree = data->ui_tree;
        
        printf("🔗 EventManager dédié connecté à la scène AI avec boutons explicitement enregistrés\n");
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Marquer comme initialisé et actif
    scene->initialized = true;
    scene->active = true;
    
    // Connecter le lien de démarrage au SceneManager
    if (data->start_game_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->start_game_link, scene_manager);
            ui_link_set_activation_delay(data->start_game_link, 0.5f);
            printf("🔗 Lien 'Démarrer' connecté au SceneManager\n");
        }
    }
    
    printf("✅ Scène Configuration IA prête avec événements EXPLICITEMENT connectés\n");
}
