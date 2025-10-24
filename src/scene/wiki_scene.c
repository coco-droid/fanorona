#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Données pour la scène Wiki
typedef struct WikiSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* back_link;
} WikiSceneData;

// Initialisation de la scène Wiki
static void wiki_scene_init(Scene* scene) {
    printf("📚 Initialisation de la scène Wiki\n");
    
    ui_set_hitbox_visualization(false);
    
    WikiSceneData* data = (WikiSceneData*)malloc(sizeof(WikiSceneData));
    if (!data) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour WikiSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->core = NULL;
    data->back_link = NULL;
    
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
    UINode* app = UI_DIV(data->ui_tree, "wiki-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    
    if (background_texture) {
        atomic_set_background_image(app->element, background_texture);
    } else {
        SET_BG(app, "rgb(135, 206, 250)");
    }
    
    // Container modal (taille identique à menu_scene)
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "wiki-modal", 500, 450);
    
    // 🆕 CONTENEUR PARENT POUR TOUT LE CONTENU (remplace l'ajout direct au modal)
    UINode* content_parent = UI_DIV(data->ui_tree, "wiki-content-parent");
    SET_SIZE(content_parent, 450, 350); // 🔧 AUGMENTÉ: 340 -> 350 pour compenser la marge de 24px
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 8); // 🔧 RÉDUIT: 12 -> 8
    
    // Ajouter le header personnalisé "WIKI DU JEU" AU CONTENEUR PARENT
    UINode* wiki_header = UI_TEXT(data->ui_tree, "wiki-header", "WIKI DU JEU");
    ui_set_text_color(wiki_header, "rgb(255, 165, 0)");
    ui_set_text_size(wiki_header, 20);
    ui_set_text_align(wiki_header, "center");
    ui_set_text_style(wiki_header, true, false);
    atomic_set_margin(wiki_header->element, 24, 0, 0, 0); // 🔧 AJOUT: 24px margin-top pour séparer du sous-titre
    
    // 🔧 SUPPRESSION: Sous-titre "GUIDE COMPLET" enlevé complètement
    
    // Container pour les sections (RÉDUIT pour compenser la marge)
    UINode* sections_container = UI_DIV(data->ui_tree, "sections-container");
    SET_SIZE(sections_container, 440, 180); // 🔧 RÉDUIT: 210 -> 180 (pour compenser +24px marge header)
    ui_set_display_flex(sections_container);
    FLEX_COLUMN(sections_container);
    ui_set_justify_content(sections_container, "space-between");
    ui_set_align_items(sections_container, "stretch");
    ui_set_flex_gap(sections_container, 0); // 🔧 RÉDUIT: 10 -> 0
    
    // === SECTION 1: RÈGLES DE BASE (UNIQUEMENT TITRE) ===
    UINode* section1 = UI_DIV(data->ui_tree, "section-rules");
    SET_SIZE(section1, 440, 55); // 🔧 RÉDUIT: 60 -> 55
    ui_set_display_flex(section1);
    ui_set_flex_direction(section1, "row");
    ui_set_justify_content(section1, "space-between");
    ui_set_align_items(section1, "center"); // 🔧 CHANGÉ: flex-start -> center
    atomic_set_background_color(section1->element, 0, 0, 0, 50);
    atomic_set_padding(section1->element, 8, 10, 8, 10); // 🔧 RÉDUIT padding vertical
    
    // Conteneur texte section 1 (SIMPLIFIÉ - titre uniquement)
    UINode* section1_text = UI_DIV(data->ui_tree, "section1-text");
    SET_SIZE(section1_text, 370, 40); // 🔧 RÉDUIT: 60 -> 40
    ui_set_display_flex(section1_text);
    FLEX_COLUMN(section1_text);
    ui_set_justify_content(section1_text, "center"); // 🔧 Centrage vertical
    
    UINode* title1 = UI_TEXT(data->ui_tree, "title-rules", "RÈGLES DE BASE");
    ui_set_text_color(title1, "rgb(233, 215, 161)");
    ui_set_text_size(title1, 14);
    ui_set_text_style(title1, true, false);
    
    // 🔧 SUPPRESSION: Description "Plateau 5x9..." enlevée
    
    APPEND(section1_text, title1);
    // 🔧 SUPPRESSION: desc1 n'est plus ajouté
    
    // Icône section 1
    UINode* icon1 = UI_DIV(data->ui_tree, "icon-rules");
    SET_SIZE(icon1, 40, 40);
    atomic_set_background_color(icon1->element, 233, 215, 161, 100);
    atomic_set_border(icon1->element, 2, 233, 215, 161, 255);
    
    APPEND(section1, section1_text);
    APPEND(section1, icon1);
    
    // === SECTION 2: STRATÉGIES (UNIQUEMENT TITRE) ===
    UINode* section2 = UI_DIV(data->ui_tree, "section-strategy");
    SET_SIZE(section2, 440, 55); // 🔧 RÉDUIT: 60 -> 55
    ui_set_display_flex(section2);
    ui_set_flex_direction(section2, "row");
    ui_set_justify_content(section2, "space-between");
    ui_set_align_items(section2, "center");
    atomic_set_background_color(section2->element, 0, 0, 0, 50);
    atomic_set_padding(section2->element, 8, 10, 8, 10);
    
    UINode* section2_text = UI_DIV(data->ui_tree, "section2-text");
    SET_SIZE(section2_text, 370, 40);
    ui_set_display_flex(section2_text);
    FLEX_COLUMN(section2_text);
    ui_set_justify_content(section2_text, "center");
    
    UINode* title2 = UI_TEXT(data->ui_tree, "title-strategy", "STRATÉGIES");
    ui_set_text_color(title2, "rgb(233, 215, 161)");
    ui_set_text_size(title2, 14);
    ui_set_text_style(title2, true, false);
    
    // 🔧 SUPPRESSION: Description "Sacrifices tactiques..." enlevée
    
    APPEND(section2_text, title2);
    
    UINode* icon2 = UI_DIV(data->ui_tree, "icon-strategy");
    SET_SIZE(icon2, 40, 40);
    atomic_set_background_color(icon2->element, 233, 215, 161, 100);
    atomic_set_border(icon2->element, 2, 233, 215, 161, 255);
    
    APPEND(section2, section2_text);
    APPEND(section2, icon2);
    
    // === SECTION 3: HISTOIRE (UNIQUEMENT TITRE) ===
    UINode* section3 = UI_DIV(data->ui_tree, "section-history");
    SET_SIZE(section3, 440, 55); // 🔧 RÉDUIT: 60 -> 55
    ui_set_display_flex(section3);
    ui_set_flex_direction(section3, "row");
    ui_set_justify_content(section3, "space-between");
    ui_set_align_items(section3, "center");
    atomic_set_background_color(section3->element, 0, 0, 0, 50);
    atomic_set_padding(section3->element, 8, 10, 8, 10);
    
    UINode* section3_text = UI_DIV(data->ui_tree, "section3-text");
    SET_SIZE(section3_text, 370, 40);
    ui_set_display_flex(section3_text);
    FLEX_COLUMN(section3_text);
    ui_set_justify_content(section3_text, "center");
    
    UINode* title3 = UI_TEXT(data->ui_tree, "title-history", "HISTOIRE");
    ui_set_text_color(title3, "rgb(233, 215, 161)");
    ui_set_text_size(title3, 14);
    ui_set_text_style(title3, true, false);
    
    // 🔧 SUPPRESSION: Description "Jeu ancestral..." enlevée
    
    APPEND(section3_text, title3);
    
    UINode* icon3 = UI_DIV(data->ui_tree, "icon-history");
    SET_SIZE(icon3, 40, 40);
    atomic_set_background_color(icon3->element, 233, 215, 161, 100);
    atomic_set_border(icon3->element, 2, 233, 215, 161, 255);
    
    APPEND(section3, section3_text);
    APPEND(section3, icon3);
    
    // Ajouter les sections au container de sections
    APPEND(sections_container, section1);
    APPEND(sections_container, section2);
    APPEND(sections_container, section3);
    
    // Bouton retour
    data->back_link = ui_create_link(data->ui_tree, "back-link", "RETOUR", "menu", SCENE_TRANSITION_REPLACE);
    if (data->back_link) {
        SET_SIZE(data->back_link, 150, 35);
        ui_set_text_align(data->back_link, "center");
        atomic_set_background_color(data->back_link->element, 64, 64, 64, 200);
        atomic_set_border(data->back_link->element, 2, 128, 128, 128, 255);
        atomic_set_text_color_rgba(data->back_link->element, 255, 255, 255, 255);
        atomic_set_padding(data->back_link->element, 6, 10, 6, 10);
        atomic_set_margin(data->back_link->element, 8, 0, 0, 0); // 🔧 RÉDUIT: 10 -> 8
        ui_link_set_target_window(data->back_link, WINDOW_TYPE_MINI);
    }
    
    // 🆕 ASSEMBLER TOUT DANS LE CONTENEUR PARENT
    APPEND(content_parent, wiki_header);
    // 🔧 SUPPRESSION: subtitle n'est plus ajouté
    APPEND(content_parent, sections_container);
    APPEND(content_parent, data->back_link);
    
    // 🆕 AJOUTER LE CONTENEUR PARENT AU MODAL (via ui_container_add_content)
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent); // Centrage vertical
    
    // Hiérarchie
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    // Animations d'entrée
    ui_animate_fade_in(modal_container, 0.8f);
    ui_animate_slide_in_left(section1, 1.0f, 200.0f);
    ui_animate_slide_in_right(section2, 1.2f, 200.0f);
    ui_animate_slide_in_left(section3, 1.4f, 200.0f);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    
    printf("✅ Interface Wiki créée - version compacte sans descriptions\n");
    
    scene->data = data;
    scene->ui_tree = data->ui_tree;
}

// Mise à jour de la scène Wiki
static void wiki_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    WikiSceneData* data = (WikiSceneData*)scene->data;
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        
        if (data->back_link) {
            ui_link_update(data->back_link, delta_time);
        }
    }
}

// Rendu de la scène Wiki
static void wiki_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    WikiSceneData* data = (WikiSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_render(data->ui_tree, renderer);
    }
}

// Nettoyage de la scène Wiki
static void wiki_scene_cleanup(Scene* scene) {
    printf("🧹 Nettoyage de la scène Wiki\n");
    if (!scene || !scene->data) return;
    
    WikiSceneData* data = (WikiSceneData*)scene->data;
    
    if (data->ui_tree) {
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
    }
    
    free(data);
    scene->data = NULL;
    
    printf("✅ Nettoyage de la scène Wiki terminé\n");
}

// Créer la scène Wiki
Scene* create_wiki_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour la scène Wiki\n");
        return NULL;
    }
    
    scene->id = strdup("wiki");
    scene->name = strdup("Wiki du Jeu");
    
    if (!scene->id || !scene->name) {
        printf("❌ Erreur: Impossible d'allouer la mémoire pour les chaînes de la scène Wiki\n");
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
    
    scene->init = wiki_scene_init;
    scene->update = wiki_scene_update;
    scene->render = wiki_scene_render;
    scene->cleanup = wiki_scene_cleanup;
    scene->data = NULL;
    
    printf("📚 Wiki scene created\n");
    return scene;
}

// Connexion des événements
void wiki_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) {
        printf("❌ Scene ou Core NULL dans wiki_scene_connect_events\n");
        return;
    }
    
    WikiSceneData* data = (WikiSceneData*)scene->data;
    if (!data) {
        printf("❌ Données de scène NULL\n");
        return;
    }
    
    if (!scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène Wiki\n");
        scene->event_manager = event_manager_create();
        if (!scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène Wiki\n");
            return;
        }
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
        printf("🔗 EventManager dédié connecté à la scène Wiki\n");
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    // Connecter le lien retour
    if (data->back_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        
        if (scene_manager) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
            ui_link_set_activation_delay(data->back_link, 0.5f);
            printf("🔗 Lien 'Retour' connecté au SceneManager\n");
        }
    }
    
    printf("✅ Scène Wiki prête avec système d'événements complet\n");
}
