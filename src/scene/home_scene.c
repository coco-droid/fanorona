#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../utils/asset_manager.h"
#include "../utils/log_console.h"
#include "../ui/components/ui_link.h"  // 🔧 AJOUTÉ: Import pour ui_create_link
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declaration pour éviter l'include circulaire
typedef struct GameCore GameCore;
extern EventManager* game_core_get_event_manager(GameCore* core);
// Nouveau: accès à la fonction pour changer l'état running
extern void game_core_set_running(GameCore* core, bool running);
// Petit stockage global pour permettre au callback du bouton d'accéder au core
static GameCore* s_home_scene_core = NULL;

// Données pour la scène home
typedef struct HomeSceneData {
    bool initialized;
    UITree* ui_tree;
    SDL_Texture* background_texture;
    SDL_Texture* logo_texture;
    GameCore* core;
    UINode* play_button;
    UINode* quit_button;
} HomeSceneData;

// 🆕 Callbacks simplifiés sans dépendances
static void quit_button_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🎯 FEEDBACK VISUEL SIMPLIFIÉ
    atomic_set_background_color(atomic_element, 220, 100, 100, 255);
    atomic_set_text_color_rgba(atomic_element, 255, 255, 255, 255);
    
    // 🔧 FIX: Utiliser les valeurs directes du style
    int current_width = atomic_element->style.width;
    int current_height = atomic_element->style.height;
    atomic_set_size(atomic_element, current_width - 4, current_height - 2);
    
    // 🔧 LOG SIMPLE
    printf("🚪 Quit button clicked with visual feedback\n");
    
    // 🆕 Demander l'arrêt propre du core si disponible
    if (s_home_scene_core) {
        game_core_set_running(s_home_scene_core, false);
        printf("🔌 game_core_set_running(..., false) appelé pour quitter proprement\n");
    } else {
        // Fallback: exit si le core n'est pas accessible
        printf("⚠️ Core non disponible, exit(0) en fallback\n");
        exit(0);
    }
    
    (void)event;
}

// 🔧 FIX: Callbacks avec gestion sécurisée de la taille et effet de scale
static void button_hovered(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🆕 APPLIQUER L'EFFET DE SCALE HOVER via l'API UI
    // Créer un UINode temporaire pour utiliser l'API
    UINode temp_node = {0};
    temp_node.element = atomic_element;
    temp_node.id = atomic_element->id;
    
    // Appliquer l'effet de scale hover (105%)
    ui_button_scale_hover(&temp_node);
    
    // Ajouter l'overlay lumineux
    atomic_set_background_color(atomic_element, 255, 255, 255, 30);
    
    //printf("🔍 [HOVER_SCALE] Button hovered with 105%% scale effect\n");
    
    (void)event;
}

static void button_unhovered(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🆕 RETOUR À LA TAILLE NORMALE via l'API UI
    UINode temp_node = {0};
    temp_node.element = atomic_element;
    temp_node.id = atomic_element->id;
    
    // Retour au scale normal (100%)
    ui_button_scale_normal(&temp_node);
    
    // Supprimer l'overlay
    atomic_set_background_color(atomic_element, 0, 0, 0, 0);
    
    //printf("🔍 [UNHOVER_SCALE] Button unhovered with 100%% scale restored\n");
    
    (void)event;
}

// Fonction pour appliquer le style de bouton à un lien
static void style_link_like_button(UINode* link) {
    if (!link || !link->element) return;
    
    // 🔧 FIX: Forcer la taille AVANT et APRÈS
    atomic_set_size(link->element, 150, 40);
    
    // 🔧 FIX: Désactiver le shrink pour ce bouton spécifiquement
    atomic_set_flex_shrink(link->element, 0);
    
    // Configuration visuelle identique au bouton
    ui_button_set_background_image(link, "home_bg_btn.png");
    SET_BG_SIZE(link, "cover");
    SET_BG_REPEAT(link, "no-repeat");
    ui_set_text_color(link, "rgb(255, 255, 255)");
    atomic_set_padding(link->element, 10, 15, 10, 15);
    atomic_set_text_align(link->element, TEXT_ALIGN_CENTER);
    
    // 🔧 FIX: Re-forcer la taille après configuration
    atomic_set_size(link->element, 150, 40);
    
    printf("🔧 [STYLE_LINK] Taille forcée à 150x40, shrink=0\n");
}

// 🆕 NOUVEAU: Callback de debug pour vérifier les tailles AVEC PLUS DE DÉTAILS
static void debug_element_size(AtomicElement* element, const char* context) {
    if (!element) return;
    
    printf("🔍 [%s] Element '%s':\n", 
           context,
           element->id ? element->id : "NoID");
    printf("   📐 Style size: %dx%d\n", element->style.width, element->style.height);
    printf("   📍 Style pos: (%d,%d)\n", element->style.x, element->style.y);
    
    // Obtenir les différents rectangles
    SDL_Rect render_rect = atomic_get_render_rect(element);
    SDL_Rect final_rect = atomic_get_final_render_rect(element);
    SDL_Rect content_rect = atomic_get_content_rect(element);
    
    printf("   🎨 Render rect: (%d,%d,%dx%d)\n", 
           render_rect.x, render_rect.y, render_rect.w, render_rect.h);
    printf("   🎯 Final rect: (%d,%d,%dx%d)\n", 
           final_rect.x, final_rect.y, final_rect.w, final_rect.h);
    printf("   📦 Content rect: (%d,%d,%dx%d)\n", 
           content_rect.x, content_rect.y, content_rect.w, content_rect.h);
           
    // Vérifier si les tailles sont valides
    if (element->style.width <= 0 || element->style.height <= 0) {
        printf("❌ TAILLE INVALIDE DÉTECTÉE!\n");
    }
    
    // Vérifier si les rectangles diffèrent
    if (render_rect.w != final_rect.w || render_rect.h != final_rect.h) {
        printf("⚠️ DIFFÉRENCE entre render_rect et final_rect détectée!\n");
    }
    
    printf("\n");
}

// Initialisation de la scène home
static void home_scene_init(Scene* scene) {
    printf("🏠 Initialisation de la scène Home avec UI DOM-like\n");
    
    // 🔧 SUPPRESSION: Plus d'activation automatique de la console d'événements
    
    // Activer les logs d'événements pour debugging
    ui_set_event_logging(true);
    
    // Activer la visualisation des hitboxes (seulement pour home)
    ui_set_hitbox_visualization(true);
    printf("🎯 Visualisation des hitboxes activée pour la scène HOME\n");
    printf("   📱 Rectangles rouges transparents avec bordure bleue 4px\n");
    printf("   📊 Logs détaillés des dimensions dans la console d'événements\n");
    
    HomeSceneData* data = (HomeSceneData*)malloc(sizeof(HomeSceneData));
    if (!data) {
        printf("Erreur: Impossible d'allouer la mémoire pour HomeSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->background_texture = NULL;
    data->logo_texture = NULL;
    data->core = NULL; // 🆕 Sera défini plus tard
    data->play_button = NULL; // 🆕 Initialiser à NULL
    data->quit_button = NULL; // 🆕 Initialiser à NULL
    
    // Créer l'arbre UI
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    
    // Charger les assets
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            // Charger les textures avec des chemins complets
            data->background_texture = asset_load_texture(renderer, "fix_bg.png");
            data->logo_texture = asset_load_texture(renderer, "fanorona_text.png");
            
            printf("🔍 Chargement des assets :\n");
            printf("   Background: %s\n", data->background_texture ? "✅ OK" : "❌ ÉCHEC");
            printf("   Logo: %s\n", data->logo_texture ? "✅ OK" : "❌ ÉCHEC");
        }
    }
    
    // === CRÉATION DE L'INTERFACE ===
    
    // Container principal (plein écran)
    UINode* app = UI_DIV(data->ui_tree, "home-app");
    if (!app) {
        printf("❌ Erreur: Impossible de créer le container principal\n");
        return;
    }
    
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    // Définir l'image de fond du container principal
    if (data->background_texture) {
        atomic_set_background_image(app->element, data->background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)"); // Bleu ciel par défaut
    }
    
    // Container principal en flexbox column pour organiser verticalement
    ui_set_display_flex(app);
    FLEX_COLUMN(app);
    ui_set_justify_content(app, "center");
    ui_set_align_items(app, "center");
    ui_set_flex_gap(app, 30); // Réduire le gap pour que les boutons restent à l'écran
    
    // Logo Fanorona au centre (réduire la taille pour laisser place aux boutons)
    UINode* logo = UI_IMAGE(data->ui_tree, "fanorona-logo", data->logo_texture);
    if (!logo) {
        printf("❌ Erreur: Impossible de créer le logo\n");
        return;
    }
    
    SET_SIZE(logo, 400, 150); // Réduire la taille du logo
    
    if (data->logo_texture) {
        printf("🖼️ Logo PNG chargé avec composant image (fond transparent)\n");
        // S'assurer que le logo n'a pas de background par défaut (déjà transparent)
        atomic_set_background_color(logo->element, 0, 0, 0, 0); // Explicitement transparent
    } else {
        // Fallback: créer un texte à la place
        logo = UI_DIV(data->ui_tree, "fanorona-logo-fallback");
        if (logo) {
            SET_SIZE(logo, 400, 100); // Plus petit aussi
            SET_BG(logo, "rgb(255,255,255)");
            UINode* logo_text = UI_TEXT(data->ui_tree, "logo-text", "FANORONA");
            if (logo_text) {
                ui_set_text_align(logo_text, "center");
                ui_set_text_color(logo_text, "rgb(0,0,0)");
                CENTER(logo_text);
                APPEND(logo, logo_text);
            }
            printf("📝 Logo en texte de secours utilisé\n");
        }
    }
    
    // Container pour les boutons - 🔧 FIX: Augmenter la taille pour éviter le shrink
    UINode* button_container = UI_DIV(data->ui_tree, "button-container");
    if (!button_container) {
        printf("❌ Erreur: Impossible de créer le container de boutons\n");
        return;
    }
    
    // 🔧 FIX: Donner une taille suffisante au container pour éviter le flex shrink
    SET_SIZE(button_container, 300, 150); // Hauteur suffisante pour 2 boutons + gap
    
    ui_set_display_flex(button_container);
    FLEX_COLUMN(button_container);
    ui_set_justify_content(button_container, "center");
    ui_set_align_items(button_container, "center");
    ui_set_flex_gap(button_container, 20); // 🔧 FIX: Réduire le gap pour éviter l'overflow
    
    // 🆕 REMPLACER LE BOUTON PLAY PAR UN UI LINK AVEC DEBUG DÉTAILLÉ
    UINode* play_link = ui_create_link(data->ui_tree, "play-link", "JOUER", "menu", SCENE_TRANSITION_REPLACE);
    data->play_button = play_link; // On garde la référence dans play_button pour les callbacks
    
    if (play_link) {
        // 🆕 DEBUG AVANT STYLE: Dimensions initiales
        printf("🔍 [PLAY_LINK_DEBUG] AVANT style:\n");
        debug_element_size(play_link->element, "PLAY_LINK_INITIAL");
        
        // Appliquer le même style que le bouton précédent
        style_link_like_button(play_link);
        
        // 🆕 DEBUG APRÈS STYLE: Vérifier que les dimensions sont correctes
        printf("🔍 [PLAY_LINK_DEBUG] APRÈS style_link_like_button():\n");
        debug_element_size(play_link->element, "PLAY_LINK_STYLED");
        
        // Connecter les mêmes événements de hover/unhover pour l'effet visuel
        atomic_set_hover_handler(play_link->element, button_hovered);
        atomic_set_unhover_handler(play_link->element, button_unhovered);
        
        // 🆕 DEBUG FINAL: État final avant ajout à la hiérarchie
        printf("🔍 [PLAY_LINK_DEBUG] FINAL avant hiérarchie:\n");
        debug_element_size(play_link->element, "PLAY_LINK_FINAL");
        
        ui_log_event("UIComponent", "LinkSetup", play_link->id, "Hover and unhover handlers attached");
        printf("✅ Lien UI 'Play' créé avec apparence de bouton et événements visuels connectés\n");
    }
    
    // Bouton Quit - avec vérifications ET désactivation du shrink
    UINode* quit_button = ui_button(data->ui_tree, "quit-button", "QUITTER", NULL, NULL);
    data->quit_button = quit_button;
    if (quit_button) {
        // 🔧 FORCER la taille AVANT les autres configurations
        atomic_set_size(quit_button->element, 150, 40);
        
        // 🔧 FIX: Désactiver le shrink pour ce bouton
        atomic_set_flex_shrink(quit_button->element, 0);
        
        // 🆕 DEBUG: Vérifier la taille après création
        debug_element_size(quit_button->element, "AFTER_CREATION");
        
        // Configuration visuelle
        ui_button_set_background_image(quit_button, "home_bg_btn.png");
        SET_BG_SIZE(quit_button, "cover");
        SET_BG_REPEAT(quit_button, "no-repeat");
        ui_set_text_color(quit_button, "rgb(255, 255, 255)");
        ui_button_fix_text_rendering(quit_button);
        
        // 🔧 FIX: Re-forcer la taille après configuration
        atomic_set_size(quit_button->element, 150, 40);
        
        // 🆕 DEBUG: Vérifier la taille après configuration
        debug_element_size(quit_button->element, "AFTER_CONFIG");
        
        // Connecter les événements
        atomic_set_click_handler(quit_button->element, quit_button_clicked);
        atomic_set_hover_handler(quit_button->element, button_hovered);
        atomic_set_unhover_handler(quit_button->element, button_unhovered);
        
        // 🆕 DEBUG: Vérifier la taille après événements
        debug_element_size(quit_button->element, "AFTER_EVENTS");
        
        printf("✅ Bouton Quit créé avec shrink=0 et taille forcée\n");
    }
    
    // Construire la hiérarchie de manière sécurisée
    if (data->ui_tree && data->ui_tree->root) {
        APPEND(data->ui_tree->root, app);
        APPEND(app, logo);
        APPEND(app, button_container);
        
        // Ajouter le lien UI et le bouton au container
        if (play_link) APPEND(button_container, play_link);
        if (quit_button) APPEND(button_container, quit_button);
        
        // Calculer les z-index implicites après avoir construit la hiérarchie
        ui_calculate_implicit_z_index(data->ui_tree);
        
    } else {
        printf("❌ Erreur: Arbre UI ou racine non initialisé\n");
        return;
    }
    
    // 🆕 DEBUG: Vérifications finales APRÈS construction de la hiérarchie
    if (play_link) {
        printf("🔍 [FINAL_DEBUG] Lien Play APRÈS hiérarchie:\n");
        debug_element_size(play_link->element, "PLAY_LINK_IN_HIERARCHY");
        
        // Force un update pour voir si ça change quelque chose
        if (data->ui_tree) {
            ui_tree_update(data->ui_tree, 0.0f);
            printf("🔍 [FINAL_DEBUG] Lien Play APRÈS ui_tree_update:\n");
            debug_element_size(play_link->element, "PLAY_LINK_POST_UPDATE");
        }
    }
    
    printf("✅ Interface Home créée avec debug détaillé du bouton Play\n");
    
    scene->data = data;
}

// Mise à jour de la scène home
static void home_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    
    // Mettre à jour l'arbre UI
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
    }
    
    // 🆕 Mettre à jour spécifiquement le bouton play (qui est un lien)
    if (data->play_button) {
        ui_link_update(data->play_button, delta_time);
    }
    
    // NOTE: Les événements SDL sont maintenant gérés dans la boucle principale
    // Ne pas faire SDL_PollEvent ici pour éviter les conflits
}

// Rendu de la scène home
static void home_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    
    // 🔧 FIX: NE PLUS FAIRE Clear ici - le core s'en charge
    // SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255);
    // SDL_RenderClear(renderer);
    
    // Rendre SEULEMENT l'arbre UI
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
    
    // 🔧 FIX PRINCIPAL: NE PLUS FAIRE Present ici - le core s'en charge !
    // SDL_RenderPresent(renderer);
}

// Nettoyage de la scène home
static void home_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Home\n");
    if (!scene || !scene->data) {
        printf("⚠️ Scene ou scene->data est NULL, nettoyage ignoré\n");
        return;
    }
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    
    // Vérifier que les données sont valides
    if (!data->initialized) {
        printf("⚠️ HomeSceneData non initialisé, nettoyage partiel\n");
        free(scene->data);
        scene->data = NULL;
        return;
    }
    
    // Libérer les textures
    if (data->background_texture) {
        SDL_DestroyTexture(data->background_texture);
        data->background_texture = NULL;
        printf("✅ Background texture libérée\n");
    }
    if (data->logo_texture) {
        SDL_DestroyTexture(data->logo_texture);
        data->logo_texture = NULL;
        printf("✅ Logo texture libérée\n");
    }
    
    // Nettoyer l'arbre UI
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
        printf("✅ UI tree détruit\n");
    }
    
    // Nettoyer les polices TTF
    ui_cleanup_fonts();
    
    // Réinitialiser l'arbre global
    ui_set_global_tree(NULL);
    
    // Marquer comme non initialisé avant de libérer
    data->initialized = false;
    
    // Libérer les données de la scène
    free(scene->data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Home terminé\n");
}

// Créer la scène home
Scene* create_home_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("Erreur: Impossible d'allouer la mémoire pour la scène Home\n");
        return NULL;
    }
    
    // Nouveaux champs pour la structure Scene mise à jour
    scene->id = "home";                   // Identifiant unique
    scene->name = "Home";                 // Nom d'affichage
    scene->target_window = WINDOW_TYPE_MAIN; // Fenêtre cible
    scene->event_manager = NULL;          // Sera initialisé plus tard
    scene->ui_tree = NULL;               // Sera créé dans init
    scene->initialized = false;          // Pas encore initialisé
    scene->active = false;               // Pas encore actif
    
    // Fonctions de callback
    scene->init = home_scene_init;
    scene->update = home_scene_update;
    scene->render = home_scene_render;
    scene->cleanup = home_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

// Connexion des événements adaptée à la nouvelle architecture
void home_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans home_scene_connect_events\n");
        return;
    }
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    // 🆕 CONNECTER SPÉCIFIQUEMENT LE LIEN UI AU SCENEMANAGER
    if (data->play_button) {
        // Obtenir le SceneManager du Core
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            // Connecter le lien UI au SceneManager pour les vraies transitions
            ui_link_connect_to_manager(data->play_button, scene_manager);
            printf("🔗 UI Link 'Play' connecté au SceneManager pour les transitions réelles\n");
            
            // 🆕 Configurer un délai de sécurité (1 seconde)
            ui_link_set_activation_delay(data->play_button, 0.0f);
            printf("⏱️ Délai de sécurité de 1s configuré pour le lien 'Play'\n");
        } else {
            printf("❌ SceneManager non disponible pour le lien UI\n");
        }
    }
    
    // Créer un EventManager dédié à la scène si nécessaire
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène home\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène home\n");
            return;
        }
    }
    
    // Connecter l'EventManager à l'UITree
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        printf("🔗 EventManager dédié connecté à l'UITree\n");
        
        // Enregistrer tous les éléments UI avec des gestionnaires d'événements
        ui_tree_register_all_events(data->ui_tree);
        
        // Stocker l'UITree dans la scène
        scene->ui_tree = data->ui_tree;
        
        printf("✅ Tous les événements connectés via l'UITree avec EventManager dédié\n");
    } else {
        printf("❌ UITree est NULL\n");
        return;
    }
    
    // Stocker la référence du core
    data->core = core;
    s_home_scene_core = core; // 🆕 Stocker le core dans le global static
    
    printf("✅ Scène home prête avec son propre système d'événements\n");
}