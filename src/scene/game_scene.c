#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h" // 🆕 Import ui_link
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../types.h"           // 🔧 FIX: Import types
#include "../config.h"          // 🔧 FIX: Import config
#include "../core/core.h"       // 🔧 FIX: Correct path for GameCore definition
#include "../pions/pions.h"     // 🔧 FIX: Import pions
#include "../logic/logic.h"     // 🔧 FIX: Import logic
#include "../stats/game_stats.h" // 🔧 FIX: Add missing include for PlayerStats
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène de jeu
typedef struct GameSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* sidebar;
    UINode* playable_area;
    GameLogic* game_logic;       // 🆕 Ajout de la logique de jeu
    SDL_Renderer* last_renderer; // 🆕 Suivi du renderer pour rechargement
} GameSceneData;

// 🆕 Callback pour le bouton OUI de la modale
static void on_confirm_quit_yes(void* element, SDL_Event* event) {
    (void)element; (void)event;
    printf("🚪 Confirmation QUIT acceptée - Retour au menu\n");
    
    // Activer le lien caché qui gère la transition propre
    extern UITree* ui_get_global_tree(void);
    UITree* tree = ui_get_global_tree();
    if (tree) {
        UINode* link = ui_tree_find_node(tree, "quit-action-link");
        if (link) {
            ui_link_activate(link);
        } else {
            printf("❌ Lien 'quit-action-link' introuvable\n");
        }
    }
}

// 🆕 Callback pour le bouton NON de la modale
static void on_confirm_quit_no(void* element, SDL_Event* event) {
    (void)element; (void)event;
    printf("↩️ Confirmation QUIT annulée - Retour au jeu\n");
    
    extern UITree* ui_get_global_tree(void);
    UITree* tree = ui_get_global_tree();
    if (tree) {
        UINode* modal = ui_tree_find_node(tree, "quit-confirm-modal");
        if (modal) {
            atomic_set_display(modal->element, DISPLAY_NONE);
            
            // Enlever la pause
            UINode* plateau = ui_tree_find_node(tree, "fanorona-plateau");
            if (plateau) {
                void* logic_ptr = ui_plateau_get_game_logic(plateau);
                if (logic_ptr) {
                    game_logic_set_pause((GameLogic*)logic_ptr, false);
                }
            }
        }
    }
}

// 🆕 Fonction helper pour construire/reconstruire l'UI
static void game_scene_build_ui(Scene* scene, SDL_Renderer* renderer) {
    GameSceneData* data = (GameSceneData*)scene->data;
    if (!data || !renderer) return;

    printf("🏗️ Construction de l'UI de jeu pour le renderer %p\n", (void*)renderer);

    // Nettoyer l'ancienne UI si elle existe
    if (data->ui_tree) {
        // 🆕 FIX: Detach event manager from tree to prevent Double Free in ui_tree_destroy
        // L'arbre ne doit pas détruire le manager car la scène en est propriétaire
        data->ui_tree->event_manager = NULL;
        
        // 🆕 FIX: Reset pointers to nodes inside the tree being destroyed
        data->sidebar = NULL;
        data->playable_area = NULL;
        
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }

    // 🆕 FIX: Reuse existing manager (clear instead of destroy) to keep GameCore pointers valid
    if (scene->event_manager) {
        event_manager_clear_all(scene->event_manager);
    } else {
        scene->event_manager = event_manager_create();
    }

    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    if (!data->ui_tree) return; // 🆕 Safety check
    ui_set_global_tree(data->ui_tree);
    
    // Reconnecter l'event manager s'il existe
    if (scene->event_manager) {
        data->ui_tree->event_manager = scene->event_manager;
    }

    // 🔧 FIX: Récupérer les dimensions depuis la main window
    int app_width = DEFAULT_MAIN_WINDOW_WIDTH;
    int app_height = DEFAULT_MAIN_WINDOW_HEIGHT;
    
    // Container principal (dimensions de main window)
    UINode* app = UI_DIV(data->ui_tree, "game-app");
    if (!app) return;
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, app_width, app_height);
    
    // Fond neutre pour la scène de jeu
    atomic_set_background_color(app->element, 45, 45, 45, 255);
    
    // Configuration flexbox horizontale
    ui_set_display_flex(app);
    ui_set_flex_direction(app, "row");
    ui_set_justify_content(app, "flex-start");
    ui_set_align_items(app, "stretch");
    
    // === SIDEBAR (1/3 exact) ===
    int sidebar_width = app_width / 3;
    data->sidebar = UI_SIDEBAR(data->ui_tree, "game-sidebar");
    if (data->sidebar) {
        SET_SIZE(data->sidebar, sidebar_width, app_height);
        ui_set_z_index(data->sidebar, 10);
        ui_animate_slide_in_left(data->sidebar, 0.6f, 300.0f);
        
        // 🔧 FIX: Ajouter les joueurs DIRECTEMENT avec les vraies données
        if (data->game_logic && data->game_logic->player1 && data->game_logic->player2) {
            ui_sidebar_add_player_containers(
                data->sidebar, 
                data->game_logic->player1, 
                data->game_logic->player2
            );
        }
        
        APPEND(app, data->sidebar);
    }
    
    // === ZONE DE JEU (2/3 exact) ===
    int playable_width = (app_width * 2) / 3;
    data->playable_area = ui_cnt_playable_with_size(data->ui_tree, "game-playable", playable_width, app_height);
    if (data->playable_area) {
        ui_set_z_index(data->playable_area, 5);
        ui_animate_fade_in(data->playable_area, 1.2f);
        
        UINode* plateau = ui_tree_find_node(data->ui_tree, "fanorona-plateau");
        if (plateau && data->game_logic) {
            ui_plateau_set_game_logic(plateau, data->game_logic);
            ui_plateau_set_players(plateau, data->game_logic->player1, data->game_logic->player2);
            
            // 🆕 FIX: Use persistent board from GameLogic instead of creating a new one
            if (data->game_logic->board) {
                ui_plateau_set_shared_board(plateau, data->game_logic->board);
            }
            
            // 🆕 CONNECTER LES GESTIONNAIRES D'ÉVÉNEMENTS DU PLATEAU
            ui_plateau_set_mouse_handlers(plateau);
        }
        
        APPEND(app, data->playable_area);
    }
    
    // 🆕 AJOUT: Lien caché pour l'action de quitter (transition propre)
    // Utilise SCENE_TRANSITION_CLOSE_AND_OPEN pour fermer la fenêtre de jeu et rouvrir le menu (Mini)
    // Cela garantit que les textures du menu sont rechargées correctement
    UINode* quit_action = ui_create_link(data->ui_tree, "quit-action-link", "", "menu", SCENE_TRANSITION_CLOSE_AND_OPEN);
    if (quit_action) {
        atomic_set_display(quit_action->element, DISPLAY_NONE); // Invisible
        ui_link_set_target_window(quit_action, WINDOW_TYPE_MINI); // Le menu est en Mini
        APPEND(app, quit_action);
        
        // Connecter au manager si disponible (sera aussi fait dans connect_events)
        if (data->core && data->core->scene_manager) {
             ui_link_connect_to_manager(quit_action, data->core->scene_manager);
        }
    }
    
    // 🆕 AJOUT: Modale de confirmation
    UINode* modal = ui_confirm_modal(data->ui_tree, "quit-confirm-modal", 
        "ABANDONNER LA PARTIE ?", 
        "Voulez-vous vraiment quitter ?\nLa progression sera perdue.",
        on_confirm_quit_yes, on_confirm_quit_no);
    
    // 🔧 FIX: Ajouter 'app' à la racine D'ABORD
    APPEND(data->ui_tree->root, app);

    if (modal) {
        // 🔧 FIX: Ajouter le modal à la RACINE (pas dans app) pour qu'il soit un overlay indépendant
        // 'app' est en flex-row, ce qui cassait le positionnement du modal
        APPEND(data->ui_tree->root, modal); 
        printf("✅ [GAME_SCENE] Modal 'quit-confirm-modal' ajouté à la racine (overlay)\n");
    } else {
        printf("❌ [GAME_SCENE] Échec création modal 'quit-confirm-modal'\n");
    }
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    // Enregistrer tous les événements de l'arbre
    if (scene->event_manager) {
        ui_tree_register_all_events(data->ui_tree);
    }
    
    // Reconnecter les boutons de la sidebar au core si disponible
    if (data->core && data->core->scene_manager) {
        // 🔧 NOTE: quit-btn n'est plus un lien direct, il ouvre la modale
        // UINode* quit_btn = ui_tree_find_node(data->ui_tree, "quit-btn");
        // if (quit_btn) ui_link_connect_to_manager(quit_btn, data->core->scene_manager);
        
        // Connecter le lien caché d'action
        UINode* quit_action = ui_tree_find_node(data->ui_tree, "quit-action-link");
        if (quit_action) ui_link_connect_to_manager(quit_action, data->core->scene_manager);
        
        UINode* settings_btn = ui_tree_find_node(data->ui_tree, "settings-btn");
        if (settings_btn) ui_link_connect_to_manager(settings_btn, data->core->scene_manager);
    }
    
    scene->ui_tree = data->ui_tree;
    printf("✅ UI reconstruite avec succès (Textures rechargées)\n");
}

// Initialisation de la scène de jeu
static void game_scene_init(Scene* scene) {
    printf("🎮 Initialisation de la scène de jeu\n");
    
    ui_set_hitbox_visualization(false);
    
    GameSceneData* data = (GameSceneData*)malloc(sizeof(GameSceneData));
    if (!data) return;
    
    data->initialized = true;
    data->core = NULL;
    data->sidebar = NULL;
    data->playable_area = NULL;
    data->game_logic = NULL;
    data->last_renderer = NULL;
    data->ui_tree = NULL;
    
    scene->data = data; // Assigner data tôt pour que build_ui puisse l'utiliser
    
    // 🆕 Créer la logique de jeu AVANT l'UI (une seule fois)
    data->game_logic = game_logic_create();
    if (data->game_logic) {
        game_logic_start_new_game(data->game_logic);
        printf("✅ GameLogic initialisée\n");
    }
    
    // Construire l'UI si le renderer est disponible
    GameWindow* window = use_main_window();
    if (window && window->renderer) {
        game_scene_build_ui(scene, window->renderer);
        data->last_renderer = window->renderer;
    }
}

// Mise à jour de la scène de jeu
static void game_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    GameSceneData* data = (GameSceneData*)scene->data;
    
    // Mettre à jour la logique de jeu
    if (data->game_logic) {
        game_logic_update(data->game_logic, delta_time);
        
        // 🔧 FIX: Update timer display every frame, not just on turn change
        if (data->sidebar) {
            GamePlayer* current = game_logic_get_current_player_info(data->game_logic);
            if (current) {
                ui_sidebar_update_current_turn(data->sidebar, current);
            }
            
            // 🆕 CRITICAL FIX: Update timers every frame for real-time display
            if (data->game_logic->player1) {
                ui_sidebar_update_player_timer(data->sidebar, data->game_logic->player1);
            }
            if (data->game_logic->player2) {
                ui_sidebar_update_player_timer(data->sidebar, data->game_logic->player2);
            }
            
            // 🔧 CRITICAL FIX: Force capture sync every few frames
            static int sync_counter = 0;
            sync_counter++;
            if (sync_counter >= 60) { // Every second at 60fps
                sync_counter = 0;
                
                // Force capture count synchronization from board state
                if (data->game_logic->board) {
                    // Calculate actual captures by counting missing pieces
                    int initial_pieces = 22; // Each player starts with 22 pieces
                    int white_pieces = 0, black_pieces = 0;
                    
                    for (int i = 0; i < NODES; i++) {
                        Piece* piece = data->game_logic->board->nodes[i].piece;
                        if (piece && piece->alive) {
                            if (piece->owner == WHITE) white_pieces++;
                            else if (piece->owner == BLACK) black_pieces++;
                        }
                    }
                    
                    // Calculate captures (pieces that were captured)
                    int p1_captures, p2_captures;
                    if (data->game_logic->player1->logical_color == WHITE) {
                        p1_captures = initial_pieces - black_pieces; // White captured black pieces
                        p2_captures = initial_pieces - white_pieces; // Black captured white pieces
                    } else {
                        p1_captures = initial_pieces - white_pieces; // Black captured white pieces
                        p2_captures = initial_pieces - black_pieces; // White captured black pieces
                    }
                    
                    // Update if different
                    if (p1_captures != data->game_logic->player1->captures_made) {
                        player_set_captures(data->game_logic->player1, p1_captures);
                    }
                    
                    if (p2_captures != data->game_logic->player2->captures_made) {
                        player_set_captures(data->game_logic->player2, p2_captures);
                    }
                }
            }
        }
    }
    
    // 🆕 AJOUT: Mettre à jour les animations (inclut les nouvelles animations de pièces)
    ui_update_animations(delta_time);
    
    // Mettre à jour l'arbre UI avec effets de scale
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
    }
    
    // 🔧 MODIFICATION: Update plateau animations avec le nouveau système
    if (data->playable_area) {
        UINode* plateau = ui_tree_find_node(data->ui_tree, "fanorona-plateau");
        if (plateau) {
            ui_plateau_update_visual_feedback(plateau, delta_time);
        }
    }
}

// Rendu de la scène de jeu
static void game_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    GameSceneData* data = (GameSceneData*)scene->data;
    
    // 🆕 DÉTECTION DE CHANGEMENT DE RENDERER (Fenêtre recréée)
    if (renderer != data->last_renderer) {
        printf("🔄 Changement de renderer détecté (%p -> %p) - Rechargement des textures...\n", 
               (void*)data->last_renderer, (void*)renderer);
        
        // Reconstruire l'UI (recharge les textures sur le nouveau renderer)
        game_scene_build_ui(scene, renderer);
        data->last_renderer = renderer;
        
        // 🆕 AUTO-UNPAUSE: Si le jeu était en pause (ex: retour de settings), on reprend
        if (data->game_logic && data->game_logic->state == GAME_STATE_PAUSED) {
            game_logic_set_pause(data->game_logic, false);
            printf("▶️ Jeu repris automatiquement après retour au jeu\n");
        }
    }
    
    // Rendre l'arbre UI
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène de jeu
static void game_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène de jeu\n");
    if (!scene || !scene->data) {
        return;
    }
    
    GameSceneData* data = (GameSceneData*)scene->data;
    
    // 🆕 FIX: Clear event manager to remove references to destroyed UI elements
    if (scene->event_manager) {
        event_manager_clear_all(scene->event_manager);
    }
    
    // 🆕 Nettoyer la logique de jeu AVANT le plateau
    if (data->game_logic) {
        printf("🗑️ [GAME_CLEANUP] Nettoyage GameLogic\n");
        game_logic_destroy(data->game_logic);
        data->game_logic = NULL;
    }
    
    // Nettoyer l'arbre UI
    if (data->ui_tree) {
        // 🆕 FIX: Detacher l'event manager pour éviter sa destruction par l'arbre
        data->ui_tree->event_manager = NULL;
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène de jeu terminé\n");
}

// Créer la scène de jeu
Scene* create_game_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->id = strdup("game");
    scene->name = strdup("Jeu Fanorona");
    
    if (!scene->id || !scene->name) {
        if (scene->id) free(scene->id);
        if (scene->name) free(scene->name);
        free(scene);
        return NULL;
    }
    
    scene->target_window = WINDOW_TYPE_MAIN; // ✅ CONFIRMÉ: Utilise la main window (800x600)
    scene->event_manager = NULL;
    scene->ui_tree = NULL;
    scene->initialized = false;
    scene->active = false;
    
    scene->init = game_scene_init;
    scene->update = game_scene_update;
    scene->render = game_scene_render;
    scene->cleanup = game_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

// Connexion des événements pour la scène de jeu
void game_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    
    GameSceneData* data = (GameSceneData*)scene->data;
    if (!data) return;
    
    // 🔧 FIX: Vérifier que la scène est initialisée
    if (!scene->initialized) {
        return;
    }
    
    // Créer un EventManager dédié à la scène
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) return;
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Si l'UI existe déjà, connecter les événements
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        
        // Connecter les boutons sidebar
        if (core->scene_manager) {
            // 🔧 NOTE: quit-btn n'est plus un lien direct
            // UINode* quit_btn = ui_tree_find_node(data->ui_tree, "quit-btn");
            // if (quit_btn) ui_link_connect_to_manager(quit_btn, core->scene_manager);
            
            // Connecter le lien caché d'action
            UINode* quit_action = ui_tree_find_node(data->ui_tree, "quit-action-link");
            if (quit_action) ui_link_connect_to_manager(quit_action, core->scene_manager);
            
            UINode* settings_btn = ui_tree_find_node(data->ui_tree, "settings-btn");
            if (settings_btn) ui_link_connect_to_manager(settings_btn, core->scene_manager);
        }
        
        // 🗑️ REMOVED: Don't register plateau events here - already done in game_scene_build_ui
        // This was causing double registration and list corruption
    }
    
    // Stocker l'UITree dans la scène
    scene->ui_tree = data->ui_tree;
    
    printf("✅ Scène de jeu connectée aux événements\n");
}
