#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
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
    int selected_avatar;  // Index de l'avatar sélectionné (1-6)
    char username[32];    // Nom d'utilisateur saisi
    UINode* main_avatar;  // Avatar principal affiché
    UINode* username_input; // Champ de texte
} ProfileSceneData;

// Callback pour la sélection d'un avatar
static void avatar_selected(void* element, SDL_Event* event) {
    (void)event;
    
    // 🔧 FIX: Get ProfileSceneData from element user_data
    AtomicElement* atomic = (AtomicElement*)element;
    ProfileSceneData* data = (ProfileSceneData*)atomic->user_data;
    
    if (!data) {
        printf("❌ User data NULL in avatar_selected\n");
        return;
    }
    
    // Get ID from atomic element directly
    if (!atomic->id) return;
    
    // Extraire le numéro d'avatar depuis l'ID (format: "avatar-mini-X")
    int avatar_num = 0;
    if (sscanf(atomic->id, "avatar-mini-%d", &avatar_num) == 1) {
        data->selected_avatar = avatar_num;
        printf("🎭 Avatar %d sélectionné\n", avatar_num);
        
        // Charger la texture de l'avatar sélectionné
        char avatar_path[32];
        snprintf(avatar_path, sizeof(avatar_path), "p%d.png", avatar_num);
        
        GameWindow* window = use_mini_window();
        if (window && data->main_avatar) {
            SDL_Renderer* renderer = window_get_renderer(window);
            if (renderer) {
                SDL_Texture* avatar_texture = asset_load_texture(renderer, avatar_path);
                if (avatar_texture) {
                    atomic_set_background_image(data->main_avatar->element, avatar_texture);
                    printf("🖼️ Avatar principal mis à jour avec %s\n", avatar_path);
                }
            }
        }
    }
}

// Callback pour le bouton confirmer
static void confirm_clicked(UINode* node, void* user_data) {
    ProfileSceneData* data = (ProfileSceneData*)user_data;
    if (!data) return;
    
    printf("✅ Profil confirmé :\n");
    printf("   👤 Nom: %s\n", data->username[0] ? data->username : "(non défini)");
    printf("   🎭 Avatar: p%d.png\n", data->selected_avatar);
    
    // TODO: Sauvegarder le profil et passer à la scène suivante
    (void)node;
}

// Fonction helper pour créer un avatar rond avec bordure
static UINode* create_avatar_circle(UITree* tree, const char* id, const char* avatar_path, 
                                   int size, int border_width, bool is_main) {
    UINode* avatar = UI_DIV(tree, id);
    if (!avatar) return NULL;
    
    SET_SIZE(avatar, size, size);
    
    // Charger l'image de l'avatar
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            SDL_Texture* avatar_texture = asset_load_texture(renderer, avatar_path);
            if (avatar_texture) {
                atomic_set_background_image(avatar->element, avatar_texture);
                atomic_set_background_size_str(avatar->element, "cover");  // 🔧 FIX: Use _str variant
            }
        }
    }
    
    // Bordure jaune pour l'avatar principal, blanche pour les mini-avatars
    if (is_main) {
        atomic_set_border(avatar->element, border_width, 255, 215, 0, 255); // Or
    } else {
        atomic_set_border(avatar->element, border_width, 255, 255, 255, 180); // Blanc translucide
    }
    
    // Simuler un cercle avec border-radius (si votre système le supporte)
    // Sinon, l'image doit être déjà ronde
    
    return avatar;
}

// Initialisation de la scène profile
static void profile_scene_init(Scene* scene) {
    printf("📋 Initialisation de la scène Profile\n");
    
    ui_set_hitbox_visualization(false);
    
    ProfileSceneData* data = (ProfileSceneData*)malloc(sizeof(ProfileSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour ProfileSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->selected_avatar = 1; // Avatar par défaut
    data->username[0] = '\0';
    data->main_avatar = NULL;
    data->username_input = NULL;
    
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
    UINode* app = UI_DIV(data->ui_tree, "profile-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "profile-modal", 550, 480);
    if (!modal_container) {
        printf("❌ Erreur: Impossible de créer le container modal\n");
        free(data);
        return;
    }
    
    // === TITRE "CRÉATION DE PROFIL" ===
    UINode* title = ui_text(data->ui_tree, "profile-title", "CREATION DE PROFIL");
    if (title) {
        ui_set_text_color(title, "rgb(255, 215, 0)"); // Or
        ui_set_text_size(title, 24);
        ui_set_text_align(title, "center");
        ui_set_text_style(title, true, false); // Gras
        ALIGN_SELF_X(title);
        ui_animate_fade_in(title, 0.8f);
    }
    
    // === AVATAR PRINCIPAL (grand rond centré) ===
    data->main_avatar = create_avatar_circle(data->ui_tree, "avatar-main", "p1.png", 45, 2, false); // 🔧 80→45 (même taille que mini-avatars)
    if (data->main_avatar) {
        ALIGN_SELF_X(data->main_avatar);
        ui_animate_pulse(data->main_avatar, 2.0f);
    }
    
    // === CONTAINER POUR LES MINI-AVATARS ===
    UINode* avatars_container = UI_DIV(data->ui_tree, "avatars-container");
    if (avatars_container) {
        SET_SIZE(avatars_container, 400, 50); // 🔧 480x70 → 400x50
        ui_set_display_flex(avatars_container);
        FLEX_ROW(avatars_container);
        ui_set_justify_content(avatars_container, "space-between");
        ui_set_align_items(avatars_container, "center");
        ALIGN_SELF_X(avatars_container);
        
        // Créer les 6 mini-avatars
        for (int i = 1; i <= 6; i++) {
            char avatar_id[32];
            char avatar_path[32];
            snprintf(avatar_id, sizeof(avatar_id), "avatar-mini-%d", i);
            snprintf(avatar_path, sizeof(avatar_path), "p%d.png", i);
            
            UINode* mini_avatar = create_avatar_circle(data->ui_tree, avatar_id, avatar_path, 45, 2, false);
            if (mini_avatar) {
                // 🔧 FIX: Store BOTH node and data in user_data chain
                mini_avatar->element->user_data = data; // ProfileSceneData for callback
                
                // 🔧 FIX: Use proper callback signature
                atomic_set_click_handler(mini_avatar->element, avatar_selected);
                
                ui_animate_fade_in(mini_avatar, 0.5f + (i * 0.1f));
                
                APPEND(avatars_container, mini_avatar);
                printf("🎭 Mini-avatar %d créé avec callback connecté\n", i);
            }
        }
    }
    
    // === CHAMP DE TEXTE POUR LE NOM ===
    // 🔧 FIX: Supprimer le input_container qui causait le DIV bleu
    // L'input sera créé directement avec ui_text_input
    
    // === BOUTON CONFIRMER ===
    UINode* confirm_btn = ui_neon_button(data->ui_tree, "confirm-btn", "CONFIRMER", confirm_clicked, data);
    if (confirm_btn) {
        SET_SIZE(confirm_btn, 200, 45);
        ui_set_text_align(confirm_btn, "center");
        ALIGN_SELF_X(confirm_btn);
        ui_neon_button_set_glow_color(confirm_btn, 0, 255, 0); // Vert
        ui_animate_slide_in_left(confirm_btn, 1.0f, 200.0f);
    }
    
    // === CONSTRUIRE AVEC SPACING VERTICAL FIXE ===
    UINode* content_column = UI_DIV(data->ui_tree, "profile-content-column");
    if (content_column) {
        SET_SIZE(content_column, 500, 420);
        ui_set_display_flex(content_column);
        FLEX_COLUMN(content_column);
        ui_set_justify_content(content_column, "flex-start");
        ui_set_align_items(content_column, "center");
        ui_set_flex_gap(content_column, 15);
        
        // Ajouter titre + avatar + liste + input + bouton dans l'ordre
        APPEND(content_column, title);
        APPEND(content_column, data->main_avatar);
        APPEND(content_column, avatars_container);
        
        // === TEXT INPUT (nouveau composant) ===
        data->username_input = ui_text_input(data->ui_tree, "username-input", "Entrez votre nom...");
        if (data->username_input) {
            SET_SIZE(data->username_input, 300, 35); // 🔧 350x40 → 300x35
            ALIGN_SELF_X(data->username_input);
            ui_text_input_set_max_length(data->username_input, 20);
            ui_animate_fade_in(data->username_input, 1.5f);
            APPEND(content_column, data->username_input);
        }
        
        APPEND(content_column, confirm_btn);
        
        ui_container_add_content(modal_container, content_column);
    }
    
    // Construire la hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Calculer les z-index
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Profile créée avec :\n");
    printf("   📝 Titre 'CRÉATION DE PROFIL'\n");
    printf("   🎭 Avatar principal (120x120) avec bordure or\n");
    printf("   🎨 6 mini-avatars sélectionnables (60x60)\n");
    printf("   ✍️  Champ de texte pour le nom\n");
    printf("   ✅ Bouton neon 'CONFIRMER'\n");
    
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
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Profile terminé\n");
}

// Créer la scène profile
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
    
    printf("✅ Profile scene created with proper memory allocation\n");
    return scene;
}

// Connecter les événements
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
        printf("🔧 Création d'un EventManager dédié pour la scène profile\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène profile\n");
            return;
        }
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        printf("🔗 EventManager dédié connecté à la scène profile\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    printf("✅ Scène profile prête avec son propre système d'événements\n");
}
