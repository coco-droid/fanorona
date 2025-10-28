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

// 🆕 STRUCTURE PROFILEDATA pour le multistep form
typedef struct ProfileData {
    // Joueur 1
    char player1_name[128];
    AvatarID player1_avatar;
    bool player1_completed;
    
    // Joueur 2
    char player2_name[128];
    AvatarID player2_avatar;
    bool player2_completed;
    
    // État du form
    int current_step;  // 1 = Joueur 1, 2 = Joueur 2
    bool is_local_multiplayer;
} ProfileData;

// Données pour la scène Profile
typedef struct ProfileSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* avatar_selector;
    UINode* back_link;
    UINode* next_link;
    UINode* name_input;
    UINode* profile_header;  // 🆕 Référence au header pour le modifier
    ProfileData* profile_data;  // 🆕 Données du multistep form
} ProfileSceneData;

// 🆕 FONCTIONS HELPER pour ProfileData
static ProfileData* create_profile_data(void) {
    ProfileData* data = (ProfileData*)calloc(1, sizeof(ProfileData));
    if (data) {
        // Initialiser Joueur 1
        strcpy(data->player1_name, "");
        data->player1_avatar = AVATAR_WARRIOR;
        data->player1_completed = false;
        
        // Initialiser Joueur 2
        strcpy(data->player2_name, "");
        data->player2_avatar = AVATAR_WARRIOR;
        data->player2_completed = false;
        
        // État initial
        data->current_step = 1;
        data->is_local_multiplayer = true;  // Pour l'instant, toujours local
        
        printf("ProfileData creee - Etape initiale: Joueur 1\n");
    }
    return data;
}

static void log_profile_data(ProfileData* data) {
    if (!data) return;
    
    printf("\n=== PROFILE DATA STATUS ===\n");
    printf("Current Step: %d (%s)\n", 
           data->current_step, 
           data->current_step == 1 ? "Joueur 1" : "Joueur 2");
    printf("Mode: %s\n", data->is_local_multiplayer ? "Local Multiplayer" : "Autre");
    
    printf("\nJOUEUR 1:\n");
    printf("   Nom: '%s'\n", data->player1_name[0] ? data->player1_name : "[VIDE]");
    printf("   Avatar: %d (%s)\n", data->player1_avatar, avatar_id_to_filename(data->player1_avatar));
    printf("   Complete: %s\n", data->player1_completed ? "OUI" : "NON");
    
    printf("\nJOUEUR 2:\n");
    printf("   Nom: '%s'\n", data->player2_name[0] ? data->player2_name : "[VIDE]");
    printf("   Avatar: %d (%s)\n", data->player2_avatar, avatar_id_to_filename(data->player2_avatar));
    printf("   Complete: %s\n", data->player2_completed ? "OUI" : "NON");
    printf("==============================\n\n");
}

// 🆕 METTRE À JOUR L'UI selon l'étape actuelle
static void update_ui_for_current_step(ProfileSceneData* scene_data) {
    if (!scene_data || !scene_data->profile_data) return;
    
    ProfileData* data = scene_data->profile_data;
    
    if (data->current_step == 1) {
        // Étape Joueur 1
        atomic_set_text(scene_data->profile_header->element, "CRÉATION DE PROFIL - JOUEUR 1");
        ui_text_input_set_placeholder(scene_data->name_input, "Nom du Joueur 1");
        atomic_set_text(scene_data->next_link->element, "SUIVANT");
        
        // 🔧 FIX: Restaurer le design du bouton après changement de texte
        atomic_set_background_color(scene_data->next_link->element, 0, 128, 0, 200);
        atomic_set_border(scene_data->next_link->element, 2, 0, 255, 0, 255);
        atomic_set_text_color_rgba(scene_data->next_link->element, 255, 255, 255, 255);
        atomic_set_padding(scene_data->next_link->element, 6, 10, 6, 10);
        
        printf("UI mise a jour pour Joueur 1\n");
    } else {
        // Étape Joueur 2
        atomic_set_text(scene_data->profile_header->element, "CRÉATION DE PROFIL - JOUEUR 2");
        ui_text_input_set_placeholder(scene_data->name_input, "Nom du Joueur 2");
        atomic_set_text(scene_data->next_link->element, "START");
        
        // 🔧 FIX: Restaurer le design avec une couleur différente pour START
        atomic_set_background_color(scene_data->next_link->element, 0, 64, 128, 200);  // Bleu foncé
        atomic_set_border(scene_data->next_link->element, 2, 0, 150, 255, 255);
        atomic_set_text_color_rgba(scene_data->next_link->element, 255, 255, 255, 255);
        atomic_set_padding(scene_data->next_link->element, 6, 10, 6, 10);
        
        printf("UI mise a jour pour Joueur 2 (bouton START)\n");
    }
}

// 🔧 FIX: Callback pour le UI Link suivant avec logique adaptée selon le mode
static void next_link_callback(UINode* link) {
    (void)link;
    
    UITree* tree = link->tree;
    if (!tree) return;
    
    // Trouver les éléments nécessaires
    UINode* avatar_selector = ui_tree_find_node(tree, "profile-avatar-selector");
    UINode* name_input = ui_tree_find_node(tree, "profile-name-input");
    
    extern ProfileSceneData* g_current_profile_scene_data;
    ProfileSceneData* scene_data = g_current_profile_scene_data;
    
    if (!scene_data || !scene_data->profile_data) {
        printf("❌ Impossible de récupérer les données de scène\n");
        return;
    }
    
    ProfileData* data = scene_data->profile_data;
    GameMode current_mode = config_get_mode();
    
    // Récupérer les valeurs actuelles
    const char* current_name = ui_text_input_get_text(name_input);
    AvatarID current_avatar = avatar_selector ? ui_avatar_selector_get_selected(avatar_selector) : AVATAR_WARRIOR;
    
    // 🆕 MODE MULTIJOUEUR EN LIGNE: Un seul profil puis redirection selon rôle
    if (current_mode == GAME_MODE_ONLINE_MULTIPLAYER) {
        // Sauvegarder le profil du joueur local
        strncpy(data->player1_name, current_name ? current_name : "", sizeof(data->player1_name) - 1);
        data->player1_name[sizeof(data->player1_name) - 1] = '\0';
        data->player1_avatar = current_avatar;
        data->player1_completed = true;
        
        // Configurer dans la config globale
        config_set_player1_full_profile(data->player1_name, data->player1_avatar);
        
        printf("🌐 Profil MULTIJOUEUR EN LIGNE sauvegardé:\n");
        printf("   👤 Joueur local: %s (Avatar %d)\n", data->player1_name, data->player1_avatar);
        
        // 🆕 REDIRECTION selon le rôle réseau
        bool is_invite = config_is_network_invite();
        const char* target_scene = is_invite ? "lobby" : "player_list";
        
        printf("🔀 Rôle réseau: %s → Redirection vers %s\n", 
               is_invite ? "INVITÉ" : "HÔTE", target_scene);
        
        if (scene_data->next_link) {
            ui_link_set_target(scene_data->next_link, target_scene);
            ui_link_set_transition(scene_data->next_link, SCENE_TRANSITION_REPLACE);
            ui_link_set_target_window(scene_data->next_link, WINDOW_TYPE_MINI);
            
            extern SceneManager* game_core_get_scene_manager(GameCore* core);
            if (scene_data->core) {
                SceneManager* scene_manager = game_core_get_scene_manager(scene_data->core);
                if (scene_manager) {
                    scene_manager_transition_to_scene(scene_manager, target_scene, SCENE_TRANSITION_REPLACE);
                    printf("✅ Transition vers %s déclenchée\n", target_scene);
                }
            }
        }
        return;
    }
    
    // 🆕 MODE VS IA: Un seul profil puis transition vers pieces_scene
    if (current_mode == GAME_MODE_VS_AI) {
        // Sauvegarder le profil du joueur humain
        strncpy(data->player1_name, current_name ? current_name : "", sizeof(data->player1_name) - 1);
        data->player1_name[sizeof(data->player1_name) - 1] = '\0';
        data->player1_avatar = current_avatar;
        data->player1_completed = true;
        
        // Configurer un profil IA par défaut pour le joueur 2
        strcpy(data->player2_name, "IA");
        data->player2_avatar = AVATAR_SAGE;  // Avatar par défaut pour l'IA
        data->player2_completed = true;
        
        // Sauvegarder dans la config
        config_set_player1_full_profile(data->player1_name, data->player1_avatar);
        config_set_player2_full_profile(data->player2_name, data->player2_avatar);
        
        printf("🤖 Profil VS IA sauvegardé:\n");
        printf("   👤 Joueur: %s (Avatar %d)\n", data->player1_name, data->player1_avatar);
        printf("   🤖 IA: %s (Avatar %d)\n", data->player2_name, data->player2_avatar);
        
        // 🔧 FIX: Transition vers pieces_scene d'abord (choix couleur avant difficulté)
         if (scene_data->next_link) {
            ui_link_set_target(scene_data->next_link, "pieces");
            ui_link_set_transition(scene_data->next_link, SCENE_TRANSITION_REPLACE);
            ui_link_set_target_window(scene_data->next_link, WINDOW_TYPE_MINI);
            
            extern SceneManager* game_core_get_scene_manager(GameCore* core);
            if (scene_data->core) {
                SceneManager* scene_manager = game_core_get_scene_manager(scene_data->core);
                if (scene_manager) {
                    scene_manager_transition_to_scene(scene_manager, "pieces", SCENE_TRANSITION_REPLACE);
                    printf("✅ Transition vers pieces_scene (choix couleur avant config IA)\n");
                }
            }
         }
         return;
     }
    
    // 🔧 MODE MULTIJOUEUR LOCAL: Logique multistep existante
    if (data->current_step == 1) {
        // Sauvegarder les données du Joueur 1
        strncpy(data->player1_name, current_name ? current_name : "", sizeof(data->player1_name) - 1);
        data->player1_name[sizeof(data->player1_name) - 1] = '\0';
        data->player1_avatar = current_avatar;
        data->player1_completed = true;
        
        printf("💾 Données Joueur 1 sauvegardées\n");
        log_profile_data(data);
        
        // Passer à l'étape 2
        data->current_step = 2;
        
        // Vider les champs pour le Joueur 2
        ui_text_input_set_text(name_input, "");
        ui_avatar_selector_reset_to_defaults(avatar_selector);
        
        // Mettre à jour l'UI
        update_ui_for_current_step(scene_data);
        
        printf("➡️ Passage à l'étape Joueur 2\n");
        
    } else {
        // Sauvegarder les données du Joueur 2 et terminer
        strncpy(data->player2_name, current_name ? current_name : "", sizeof(data->player2_name) - 1);
        data->player2_name[sizeof(data->player2_name) - 1] = '\0';
        data->player2_avatar = current_avatar;
        data->player2_completed = true;
        
        printf("💾 Données Joueur 2 sauvegardées\n");
        log_profile_data(data);
        
        // 🆕 ENREGISTRER dans la configuration globale
        config_set_player1_full_profile(data->player1_name, data->player1_avatar);
        config_set_player2_full_profile(data->player2_name, data->player2_avatar);
        
        printf("🏁 MULTISTEP FORM COMPLÉTÉ!\n");
        printf("🚀 Transition vers pieces_scene...\n");
        
        // 🔧 FIX: Rediriger vers pieces_scene au lieu de game_scene
        if (scene_data->next_link) {
            ui_link_set_target(scene_data->next_link, "pieces");
            ui_link_set_transition(scene_data->next_link, SCENE_TRANSITION_REPLACE);
            ui_link_set_target_window(scene_data->next_link, WINDOW_TYPE_MINI);
            
            // Déclencher la transition immédiatement
            extern SceneManager* game_core_get_scene_manager(GameCore* core);
            if (scene_data->core) {
                SceneManager* scene_manager = game_core_get_scene_manager(scene_data->core);
                if (scene_manager) {
                    scene_manager_transition_to_scene(scene_manager, "pieces", SCENE_TRANSITION_REPLACE);
                    printf("✅ Transition vers pieces_scene déclenchée\n");
                }
            }
        }
    }
}

// Variable globale pour l'accès aux données de scène (HACK temporaire)
ProfileSceneData* g_current_profile_scene_data = NULL;

// Initialisation de la scène Profile
static void profile_scene_init(Scene* scene) {
    printf("👤 Initialisation de la scène Profile\n");
    
    ui_set_hitbox_visualization(false);
    
    ProfileSceneData* data = (ProfileSceneData*)malloc(sizeof(ProfileSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour ProfileSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->avatar_selector = NULL;
    data->back_link = NULL;
    data->next_link = NULL;
    data->name_input = NULL;
    data->profile_header = NULL;
    data->profile_data = create_profile_data();
    
    // 🔧 HACK: Stocker globalement pour accès depuis le callback
    g_current_profile_scene_data = data;

    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // 🆕 Adapter l'interface selon le mode de jeu
    GameMode current_mode = config_get_mode();
    if (current_mode == GAME_MODE_VS_AI) {
        printf("🤖 Mode VS IA détecté - interface simplifiée (1 joueur)\n");
    } else if (current_mode == GAME_MODE_ONLINE_MULTIPLAYER) {
        bool is_invite = config_is_network_invite();
        printf("🌐 Mode MULTIJOUEUR EN LIGNE détecté - interface simplifiée (1 joueur)\n");
        printf("   🎭 Rôle: %s\n", is_invite ? "INVITÉ" : "HÔTE");
    } else {
        printf("👥 Mode multijoueur détecté - interface complète (2 joueurs)\n");
    }
    
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
    UINode* app = UI_DIV(data->ui_tree, "profile-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal centré
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "profile-modal", 500, 450);
    
    // Conteneur parent pour tout le contenu
    UINode* content_parent = UI_DIV(data->ui_tree, "profile-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 20);
    
    // Header "CRÉATION DE PROFIL" (adapté selon le mode)
    const char* initial_header;
    if (current_mode == GAME_MODE_VS_AI) {
        initial_header = "VOTRE PROFIL - CONTRE IA";
    } else if (current_mode == GAME_MODE_ONLINE_MULTIPLAYER) {
        bool is_invite = config_is_network_invite();
        initial_header = is_invite ? "VOTRE PROFIL - INVITÉ" : "VOTRE PROFIL - HÔTE";
    } else {
        initial_header = "CRÉATION DE PROFIL - JOUEUR 1";
    }
    
    data->profile_header = UI_TEXT(data->ui_tree, "profile-header", initial_header);
    ui_set_text_color(data->profile_header, "rgb(255, 165, 0)");
    ui_set_text_size(data->profile_header, 20);
    ui_set_text_align(data->profile_header, "center");
    ui_set_text_style(data->profile_header, true, false);
    atomic_set_margin(data->profile_header->element, 24, 0, 0, 0);
    
    // Avatar Selector
    data->avatar_selector = UI_AVATAR_SELECTOR(data->ui_tree, "profile-avatar-selector");
    if (data->avatar_selector) {
        AVATAR_RESET_DEFAULTS(data->avatar_selector);
        printf("✨ Avatar selector créé et réinitialisé\n");
    }
    
    // 🆕 Text Input avec placeholder adapté
    const char* initial_placeholder;
    if (current_mode == GAME_MODE_VS_AI) {
        initial_placeholder = "Votre nom";
    } else if (current_mode == GAME_MODE_ONLINE_MULTIPLAYER) {
        initial_placeholder = "Votre nom";
    } else {
        initial_placeholder = "Nom du Joueur 1";
    }
    
    data->name_input = ui_text_input(data->ui_tree, "profile-name-input", initial_placeholder);
    if (data->name_input) {
        SET_SIZE(data->name_input, 300, 40);
        ui_set_text_align(data->name_input, "left");
        atomic_set_background_color(data->name_input->element, 255, 255, 255, 220);
        atomic_set_border(data->name_input->element, 2, 255, 165, 0, 255);
        ui_text_input_set_max_length(data->name_input, 50);
        ui_text_input_set_scene_id(data->name_input, "input_name");
        printf("📝 Text input créé avec placeholder adapté au mode\n");
    }
    
    // 🆕 Container pour les boutons (RETOUR + SUIVANT)
    UINode* buttons_container = UI_DIV(data->ui_tree, "profile-buttons-container");
    SET_SIZE(buttons_container, 350, 50);
    ui_set_display_flex(buttons_container);
    FLEX_ROW(buttons_container);
    ui_set_justify_content(buttons_container, "space-between");
    ui_set_align_items(buttons_container, "center");
    
    // Bouton retour
    data->back_link = ui_create_link(data->ui_tree, "back-link", "RETOUR", "menu", SCENE_TRANSITION_REPLACE);
    if (data->back_link) {
        SET_SIZE(data->back_link, 150, 35);
        ui_set_text_align(data->back_link, "center");
        atomic_set_background_color(data->back_link->element, 64, 64, 64, 200);
        atomic_set_border(data->back_link->element, 2, 128, 128, 128, 255);
        atomic_set_text_color_rgba(data->back_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->back_link->element, 6, 10, 6, 10);
        ui_link_set_target_window(data->back_link, WINDOW_TYPE_MINI);
        APPEND(buttons_container, data->back_link);
    }
    
    // UI Link avec texte adapté selon le mode
    const char* initial_button_text;
    if (current_mode == GAME_MODE_VS_AI) {
        initial_button_text = "CONFIGURATION IA";
    } else if (current_mode == GAME_MODE_ONLINE_MULTIPLAYER) {
        bool is_invite = config_is_network_invite();
        initial_button_text = is_invite ? "REJOINDRE LOBBY" : "CHERCHER ADVERSAIRE";
    } else {
        initial_button_text = "SUIVANT";
    }
    
    data->next_link = ui_create_link(data->ui_tree, "next-link", initial_button_text, NULL, SCENE_TRANSITION_REPLACE);
    if (data->next_link) {
        SET_SIZE(data->next_link, 200, 35);
        ui_set_text_align(data->next_link, "center");
        
        // Style adapté selon le mode
        if (current_mode == GAME_MODE_VS_AI) {
            atomic_set_background_color(data->next_link->element, 255, 140, 0, 200);
            atomic_set_border(data->next_link->element, 2, 255, 165, 0, 255);
        } else if (current_mode == GAME_MODE_ONLINE_MULTIPLAYER) {
            atomic_set_background_color(data->next_link->element, 0, 64, 128, 200);
            atomic_set_border(data->next_link->element, 2, 0, 191, 255, 255);
        } else {
            atomic_set_background_color(data->next_link->element, 0, 128, 0, 200);
            atomic_set_border(data->next_link->element, 2, 0, 255, 0, 255);
        }
        
        atomic_set_text_color_rgba(data->next_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->next_link->element, 6, 10, 6, 10);
        
        ui_link_set_target(data->next_link, NULL);
        ui_link_set_click_handler(data->next_link, next_link_callback);
        ui_link_set_activation_delay(data->next_link, 0.0f);
        
        APPEND(buttons_container, data->next_link);
        printf("🔗 UI Link créé avec style adapté au mode %s\n", config_mode_to_string(current_mode));
    }
    
    // Assembler dans le conteneur parent
    APPEND(content_parent, data->profile_header);  // 🔧 FIX: Utiliser la référence stockée
    APPEND(content_parent, data->avatar_selector);
    APPEND(content_parent, data->name_input);
    APPEND(content_parent, buttons_container);

    // Ajouter au modal
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    // Hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Animation d'entrée
    ui_animate_fade_in(modal_container, 0.8f);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Profile créée et adaptée au mode: %s\n", 
           config_mode_to_string(current_mode));
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène Profile
static void profile_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        if (data->avatar_selector) {
            ui_avatar_selector_update(data->avatar_selector, delta_time);
        }
        
        if (data->back_link) {
            ui_link_update(data->back_link, delta_time);
        }
        
        if (data->next_link) {
            ui_link_update(data->next_link, delta_time);
        }
    }
}

// Rendu de la scène Profile
static void profile_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène Profile
static void profile_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Profile\n");
    if (!scene || !scene->data) return;
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    
    if (data->profile_data) {
        printf("🗑️ Nettoyage ProfileData\n");
        free(data->profile_data);
        data->profile_data = NULL;
    }
    
    if (g_current_profile_scene_data == data) {
        g_current_profile_scene_data = NULL;
    }
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Profile terminé\n");
}

// Créer la scène Profile
Scene* create_profile_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Profile\n");
        return NULL;
    }
    
    scene->id = strdup("profile");
    scene->name = strdup("Création de Profil");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène Profile\n");
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
    
    printf("👤 Profile scene created\n");
    return scene;
}

// Connexion des événements
void profile_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans profile_scene_connect_events\n");
        return;
    }
    
    ProfileSceneData* data = (ProfileSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène Profile\n");
            return;
        }
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        
        if (data->avatar_selector) {
            ui_avatar_selector_register_events(data->avatar_selector, scene->event_manager);
            printf("🔗 Avatar selector events registered with scene EventManager\n");
        }
        
        printf("🔗 EventManager connecté à la scène Profile\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    if (data->back_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
            printf("🔗 Lien 'Retour' connecté au SceneManager\n");
        }
    }
    
    if (data->next_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->next_link, scene_manager);
            printf("🔗 UI Link 'SUIVANT' connecté (callback seulement)\n");
        }
    }
    
    printf("✅ Scène Profile prête avec avatar selector, text input et UI Link SUIVANT\n");
}