#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"  // 🆕 AJOUT: Import pour ui_create_link
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // 🆕 AJOUT: Pour strdup

// Données pour la scène menu
typedef struct MenuSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* ai_link;  // 🆕 AJOUT: Référence au lien IA pour la connexion
} MenuSceneData;

// Callbacks pour les neon buttons
static void multiplayer_clicked(UINode* node, void* user_data) {
    printf("🌐 Multijoueur sélectionné\n");
    (void)node; (void)user_data;
}

static void wiki_clicked(UINode* node, void* user_data) {
    printf("📚 Wiki ouvert\n");
    (void)node; (void)user_data;
}

// 🆕 Fonction pour styliser le lien comme un neon button
static void style_link_as_neon_button(UINode* link, int r, int g, int b) {
    if (!link || !link->element) return;
    
    // Appliquer le style de base d'un neon button
    SET_SIZE(link, 280, 45);
    ui_set_text_align(link, "center");
    
    // Style neon simulé avec couleur de base
    atomic_set_background_color(link->element, r/4, g/4, b/4, 200); // Couleur de base sombre
    atomic_set_border(link->element, 2, r, g, b, 255); // Bordure colorée pour effet neon
    atomic_set_text_color_rgba(link->element, 255, 255, 255, 255); // Texte blanc
    atomic_set_padding(link->element, 10, 15, 10, 15);
    
    printf("✨ UI Link stylisé comme neon button avec couleur (%d,%d,%d)\n", r, g, b);
}

// 🆕 Callback hover pour effet neon avec scale sur le lien
static void ai_link_hovered(void* element, SDL_Event* event) {
    (void)element;
    (void)event;
}

// 🆕 Callback unhover pour retour normal avec scale
static void ai_link_unhovered(void* element, SDL_Event* event) {
    (void)element;
    (void)event;
}

// Initialisation de la scène menu
static void menu_scene_init(Scene* scene) {
    printf("📋 Initialisation de la scène Menu avec UI Link vers game_scene\n");
    
    // 🔧 DÉSACTIVER la visualisation des hitboxes pour la scène menu
    ui_set_hitbox_visualization(false);
    printf("🚫 Visualisation des hitboxes DÉSACTIVÉE pour la scène menu\n");
    
    MenuSceneData* data = (MenuSceneData*)malloc(sizeof(MenuSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour MenuSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->ai_link = NULL;  // 🆕 Initialiser la référence
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // === CHARGER LE BACKGROUND SEULEMENT ===
    SDL_Texture* background_texture = NULL;
    
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            background_texture = asset_load_texture(renderer, "fix_bg.png");
            printf("🔍 Background menu chargé : %s\n", background_texture ? "✅ OK" : "❌ ÉCHEC");
        }
    }
    
    // Container principal (plein écran) avec background
    UINode* app = UI_DIV(data->ui_tree, "menu-app");
    if (!app) {
        printf("❌ Erreur: Impossible de créer le container principal\n");
        free(data);
        return;
    }
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    // Utiliser le même background que home
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
        printf("🖼️ Background identique à home appliqué\n");
    } else {
        SET_BG(app, "rgb(135, 206, 250)"); // Bleu ciel par défaut
    }
    
    // === CONTAINER MODAL AVEC LOGO ET TEXTE AUTOMATIQUES ===
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "modal-container", 500, 450);
    if (!modal_container) {
        printf("❌ Erreur: Impossible de créer le container modal\n");
        free(data);
        return;
    }
    // 🎉 Le container a maintenant automatiquement :
    // - Logo à 10px du haut depuis l'intérieur, centré horizontalement avec align-self
    // - Texte "Stratégie et Tradition" à 98px depuis l'intérieur (logo + 8px), centré avec align-self
    
    // === CONTAINER POUR LES BOUTONS (SIMPLE) ===
    UINode* buttons_container = UI_DIV(data->ui_tree, "buttons-container");
    if (buttons_container) {
        SET_SIZE(buttons_container, 300, 200); // Taille définie
        
        // Configuration flexbox UNIQUEMENT pour les boutons à l'intérieur
        ui_set_display_flex(buttons_container);
        FLEX_COLUMN(buttons_container);
        ui_set_justify_content(buttons_container, "center");
        ui_set_align_items(buttons_container, "center");
        ui_set_flex_gap(buttons_container, 15);
        
        // === VRAIS NEON BUTTONS (maintenant que neon_btn.c est compilé) ===
        
        // 1. Bouton Multijoueur avec neon - DEBUG DES DIMENSIONS
        UINode* multiplayer_btn = ui_neon_button(data->ui_tree, "multiplayer-btn", "JOUER EN MULTIJOUEUR", multiplayer_clicked, NULL);
        if (multiplayer_btn) {
            SET_SIZE(multiplayer_btn, 280, 45);
            ui_set_text_align(multiplayer_btn, "center");
            
            // 🆕 DEBUG DÉTAILLÉ: Vérifier les dimensions du bouton "play"
            printf("🔍 [BUTTON_DEBUG] Bouton Multijoueur créé:\n");
            printf("   📐 Taille demandée: 280x45\n");
            printf("   📐 Taille réelle dans style: %dx%d\n", 
                   multiplayer_btn->element->style.width, 
                   multiplayer_btn->element->style.height);
            printf("   📍 Position: (%d, %d)\n",
                   multiplayer_btn->element->style.x,
                   multiplayer_btn->element->style.y);
            
            // Vérifier le rectangle de rendu final
            SDL_Rect render_rect = atomic_get_final_render_rect(multiplayer_btn->element);
            printf("   🎯 Rectangle de rendu final: (%d,%d,%dx%d)\n",
                   render_rect.x, render_rect.y, render_rect.w, render_rect.h);
            
            // Configuration spécifique neon
            ui_neon_button_set_glow_color(multiplayer_btn, 0, 255, 127); // Vert neon
            ui_neon_button_set_animation_speed(multiplayer_btn, 1.5f);
            
            APPEND(buttons_container, multiplayer_btn);
            printf("✨ Neon Button 'Multijoueur' créé avec lueur verte\n");
        }
        
        // 🆕 2. UI LINK pour IA avec transition de fenêtre (MINI → MAIN)
        data->ai_link = ui_create_link(data->ui_tree, "ai-link", "JOUER CONTRE L'IA", "game", SCENE_TRANSITION_CLOSE_AND_OPEN);
        if (data->ai_link) {
            // Styliser comme un neon button violet
            style_link_as_neon_button(data->ai_link, 255, 0, 255); // Violet neon
            
            // Ajouter les effets hover/unhover pour l'effet neon
            atomic_set_hover_handler(data->ai_link->element, ai_link_hovered);
            atomic_set_unhover_handler(data->ai_link->element, ai_link_unhovered);
            
            // 🆕 CONFIGURER LA FENÊTRE CIBLE (MAIN WINDOW)
            ui_link_set_target_window(data->ai_link, WINDOW_TYPE_MAIN);
            
            APPEND(buttons_container, data->ai_link);
            printf("🔗✨ UI Link 'IA' créé avec transition MINI → MAIN WINDOW\n");
            printf("   🎯 Cible: game_scene dans MAIN WINDOW (800x600)\n");
            printf("   🔄 Transition: SCENE_TRANSITION_CLOSE_AND_OPEN\n");
            printf("   🎨 Style: Neon button violet avec effets hover\n");
        }
        
        // 3. Bouton Wiki avec neon (inchangé)
        UINode* wiki_btn = ui_neon_button(data->ui_tree, "wiki-btn", "WIKI", wiki_clicked, NULL);
        if (wiki_btn) {
            SET_SIZE(wiki_btn, 280, 45);
            ui_set_text_align(wiki_btn, "center");
            
            // Configuration spécifique neon
            ui_neon_button_set_glow_color(wiki_btn, 0, 191, 255); // Bleu ciel neon
            ui_neon_button_set_animation_speed(wiki_btn, 1.0f);
            
            APPEND(buttons_container, wiki_btn);
            printf("✨ Neon Button 'Wiki' créé avec lueur bleu ciel\n");
        }
        
        // 🎯 AJOUTER LE CONTAINER DE BOUTONS AU MODAL AVEC CENTRAGE VERTICAL
        // Il sera automatiquement positionné à 126px du haut (sous-titre + 8px) et centré horizontalement
        ui_container_add_content(modal_container, buttons_container);
        
        // 🆕 CENTRAGE VERTICAL: Utiliser align-self pour centrer le container de boutons
        ALIGN_SELF_Y(buttons_container);  // Centrage vertical automatique
        
        printf("📦 Container de boutons ajouté avec positionnement automatique et centrage Y\n");
        printf("   📍 Position: centré horizontalement ET verticalement dans le modal\n");
    }
    
    // Construire la hiérarchie simplifiée
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Calculer les z-index implicites
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Menu créée avec :\n");
    printf("   🖼️  Background identique à home\n");
    printf("   📦  Container modal avec logo et sous-titre AUTOMATIQUES\n");
    printf("   🎮  Bouton Multijoueur : Neon button classique\n");
    printf("   🔗  Bouton IA : UI LINK avec transition MINI→MAIN vers game_scene\n");
    printf("   📚  Bouton Wiki : Neon button classique\n");
    printf("   🌟  NOUVELLE FONCTIONNALITÉ : Clic sur IA = Ouverture game en main window !\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène menu avec animations neon
static void menu_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // Mettre à jour l'arbre UI
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        // Mettre à jour spécifiquement les animations neon
        ui_neon_button_update_all(data->ui_tree, delta_time);
        
        // 🆕 Mettre à jour le lien IA
        if (data->ai_link) {
            ui_link_update(data->ai_link, delta_time);
        }
    }
}

// Rendu de la scène menu
static void menu_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // Rendre l'arbre UI
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène menu
static void menu_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Menu\n");
    if (!scene || !scene->data) {
        return;
    }
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    
    // Nettoyer l'arbre UI
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Menu terminé\n");
}

// Créer la scène menu - Assurons-nous que cette fonction est bien définie et exportée
Scene* create_menu_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Menu\n");
        return NULL;
    }
    
    // 🔧 FIX: Use strdup() instead of string literals
    scene->id = strdup("menu");
    scene->name = strdup("Menu Principal");
    
    // 🔧 FIX: Check if strdup() succeeded
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène Menu\n");
        if (scene->id) free(scene->id);
        if (scene->name) free(scene->name);
        free(scene);
        return NULL;
    }
    
    scene->target_window = WINDOW_TYPE_MINI; // 🔧 FIX: Change from MAIN to MINI to match the current window
    scene->event_manager = NULL;
    scene->ui_tree = NULL;
    scene->initialized = false;
    scene->active = false;
    
    scene->init = menu_scene_init;
    scene->update = menu_scene_update;
    scene->render = menu_scene_render;
    scene->cleanup = menu_scene_cleanup;
    scene->data = NULL;
    
    printf("✅ Menu scene created with proper memory allocation\n");
    return scene;
}

// Assurons-nous que cette fonction est bien définie et exportée  
void menu_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans menu_scene_connect_events\n");
        return;
    }
    
    MenuSceneData* data = (MenuSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    // Créer un EventManager dédié à la scène au lieu d'utiliser celui du Core
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène menu\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène menu\n");
            return;
        }
    }
    
    // Connecter l'EventManager dédié à l'UITree
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        
        // Enregistrer tous les éléments UI
        ui_tree_register_all_events(data->ui_tree);
        
        // Stocker l'UITree dans la scène
        scene->ui_tree = data->ui_tree;
        
        printf("🔗 EventManager dédié connecté à la scène menu\n");
    }
    
    // Stocker la référence du core
    data->core = core;
    
    // Marquer comme initialisé et actif
    scene->initialized = true;
    scene->active = true;
    
    printf("✅ Scène menu prête avec son propre système d'événements\n");
    
    // 🆕 CONNECTER SPÉCIFIQUEMENT LE LIEN IA AU SCENEMANAGER
    if (data->ai_link) {
        // Obtenir le SceneManager du Core
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            // Connecter le lien UI au SceneManager pour les vraies transitions
            ui_link_connect_to_manager(data->ai_link, scene_manager);
            
            // 🆕 Configurer un délai de sécurité (0.5 seconde)
            ui_link_set_activation_delay(data->ai_link, 0.5f);
            
            printf("🔗 UI Link 'IA' connecté au SceneManager pour transition MINI→MAIN\n");
            printf("   🎯 Lors du clic : mini_window se fermera, main_window s'ouvrira avec game_scene\n");
            printf("   📏 Dimensions : 700x500 → 800x600\n");
            printf("   🎮 Layout : menu simple → sidebar + zone de jeu\n");
            printf("   ⏱️ Délai de sécurité de 0.5s configuré pour le lien 'IA'\n");
        } else {
            printf("❌ SceneManager non disponible pour le lien IA\n");
        }
    }
}
