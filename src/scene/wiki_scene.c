#define _POSIX_C_SOURCE 200809L
#include "scene.h"
#include "../ui/ui_components.h"
#include "../ui/components/ui_link.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../sound/sound.h" // Pour le son des clics
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enum pour les vues
typedef enum WikiView {
    WIKI_VIEW_MENU,
    WIKI_VIEW_RULES,
    WIKI_VIEW_STRATEGY,
    WIKI_VIEW_HISTORY
} WikiView;

// Données pour la scène Wiki
typedef struct WikiSceneData {
    bool initialized;
    UITree* ui_tree;
    GameCore* core;
    UINode* back_link;
    SDL_Renderer* last_renderer;
    
    // 🆕 État de la vue
    WikiView current_view;
    int current_page;
    bool dirty; // Pour forcer la reconstruction
} WikiSceneData;

// === CONTENU DU WIKI (PAGES) ===

static const char* RULES_PAGES[] = {
    "LE BUT DU JEU\n\nCapturer tous les pions adverses ou\nles bloquer pour qu'ils ne puissent\nplus bouger.",
    "DEPLACEMENT\n\nLes pions se deplacent d'une intersection\nvers une intersection adjacente vide\nen suivant les lignes.",
    "CAPTURE PAR APPROCHE\n\nSi vous bougez vers un pion adverse\net que vous arrivez a cote de lui,\nvous le capturez (et ceux derriere).",
    "CAPTURE PAR ELOIGNEMENT\n\nSi vous bougez en vous eloignant\nd'un pion adverse qui etait a cote,\nvous le capturez."
};
static int RULES_PAGE_COUNT = 4;

static const char* STRATEGY_PAGES[] = {
    "CONTROLE DU CENTRE\n\nLes intersections centrales offrent\nplus de possibilites de mouvement.\nOccupez-les tot.",
    "SACRIFICES\n\nParfois, perdre un pion permet\nd'en capturer trois au tour suivant.\nSoyez prevoyant.",
    "FIN DE PARTIE\n\nQuand il reste peu de pions,\nevitez les echanges egaux si vous\netes en tete."
};
static int STRATEGY_PAGE_COUNT = 3;

static const char* HISTORY_PAGES[] = {
    "ORIGINES\n\nLe Fanorona est le jeu national\nde Madagascar. Il existe depuis\ndes siecles.",
    "RITUELS\n\nAutrefois, les devins l'utilisaient\npour predire l'issue des batailles\nselon le deroulement de la partie."
};
static int HISTORY_PAGE_COUNT = 2;

// Forward declaration
static void wiki_scene_build_ui(Scene* scene, SDL_Renderer* renderer);

// === CALLBACKS ===

static void local_click_wrapper(void* element, SDL_Event* event) {
    (void)event;
    sound_play_button_click();
    AtomicElement* atomic = (AtomicElement*)element;
    void (*cb)(UINode*, void*) = (void (*)(UINode*, void*))atomic_get_custom_data(atomic, "cb");
    void* ud = atomic_get_custom_data(atomic, "ud");
    
    // Récupérer le UINode associé (stocké dans user_data de l'élément)
    UINode* node = (UINode*)atomic->user_data;
    
    if (cb && node) cb(node, ud);
}

static void on_section_click(UINode* node, void* user_data) {
    Scene* scene = (Scene*)user_data;
    WikiSceneData* data = (WikiSceneData*)scene->data;
    
    if (strstr(node->id, "rules")) data->current_view = WIKI_VIEW_RULES;
    else if (strstr(node->id, "strategy")) data->current_view = WIKI_VIEW_STRATEGY;
    else if (strstr(node->id, "history")) data->current_view = WIKI_VIEW_HISTORY;
    
    data->current_page = 0;
    data->dirty = true;
}

static void on_back_to_menu(UINode* node, void* user_data) {
    (void)node;
    Scene* scene = (Scene*)user_data;
    WikiSceneData* data = (WikiSceneData*)scene->data;
    data->current_view = WIKI_VIEW_MENU;
    data->dirty = true;
}

static void on_prev_page(UINode* node, void* user_data) {
    (void)node;
    Scene* scene = (Scene*)user_data;
    WikiSceneData* data = (WikiSceneData*)scene->data;
    if (data->current_page > 0) {
        data->current_page--;
        data->dirty = true;
    }
}

static void on_next_page(UINode* node, void* user_data) {
    (void)node;
    Scene* scene = (Scene*)user_data;
    WikiSceneData* data = (WikiSceneData*)scene->data;
    
    int max_pages = 0;
    if (data->current_view == WIKI_VIEW_RULES) max_pages = RULES_PAGE_COUNT;
    else if (data->current_view == WIKI_VIEW_STRATEGY) max_pages = STRATEGY_PAGE_COUNT;
    else if (data->current_view == WIKI_VIEW_HISTORY) max_pages = HISTORY_PAGE_COUNT;
    
    if (data->current_page < max_pages - 1) {
        data->current_page++;
        data->dirty = true;
    }
}

// Helper pour rendre un élément interactif
static void make_interactive(UINode* node, void (*callback)(UINode*, void*), void* user_data) {
    if (!node) return;
    atomic_set_custom_data(node->element, "cb", (void*)callback);
    atomic_set_custom_data(node->element, "ud", user_data);
    atomic_set_click_handler(node->element, local_click_wrapper);
    
    // Effet hover simple
    atomic_set_custom_data(node->element, "orig_bg", (void*)(intptr_t)0); // Marker
    // Note: Pour un effet hover complet, il faudrait des handlers hover/unhover dédiés
}

// 🆕 Helper pour le style doré uniforme
static void apply_gold_button_style(UINode* node) {
    if (!node) return;
    atomic_set_background_color(node->element, 20, 20, 20, 220); // 🆕 FIX: Darker bg
    atomic_set_border(node->element, 2, 255, 215, 0, 255); // Gold
    atomic_set_text_color_rgba(node->element, 255, 255, 255, 255);
    atomic_set_padding(node->element, 5, 10, 5, 10);
}

// Initialisation de la scène Wiki
static void wiki_scene_init(Scene* scene) {
    printf("📚 Initialisation de la scène Wiki\n");
    
    ui_set_hitbox_visualization(false);
    
    WikiSceneData* data = (WikiSceneData*)malloc(sizeof(WikiSceneData));
    if (!data) return;
    
    data->initialized = true;
    data->core = NULL;
    data->back_link = NULL;
    data->ui_tree = NULL;
    data->last_renderer = NULL;
    data->current_view = WIKI_VIEW_MENU;
    data->current_page = 0;
    data->dirty = false;
    
    scene->data = data;
    
    GameWindow* window = use_mini_window();
    if (window && window->renderer) {
        wiki_scene_build_ui(scene, window->renderer);
        data->last_renderer = window->renderer;
    }
}

static void wiki_scene_build_ui(Scene* scene, SDL_Renderer* renderer) {
    WikiSceneData* data = (WikiSceneData*)scene->data;
    if (!data || !renderer) return;

    // Nettoyage
    if (scene->event_manager) event_manager_clear_all(scene->event_manager);
    if (data->ui_tree) {
        ui_tree_stop_all_animations(data->ui_tree);
        data->ui_tree->event_manager = NULL;
        ui_tree_destroy(data->ui_tree);
        data->ui_tree = NULL;
        data->back_link = NULL;
    }
    
    data->ui_tree = ui_tree_create();
    ui_set_global_tree(data->ui_tree);
    if (scene->event_manager) data->ui_tree->event_manager = scene->event_manager;
    
    // Background
    SDL_Texture* background_texture = asset_load_texture(renderer, "fix_bg.png");
    UINode* app = UI_DIV(data->ui_tree, "wiki-app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 700, 500);
    if (background_texture) atomic_set_background_image(app->element, background_texture);
    else SET_BG(app, "rgb(135, 206, 250)");
    
    UINode* modal_container = UI_CONTAINER_CENTERED(data->ui_tree, "wiki-modal", 500, 450);
    
    UINode* content_parent = UI_DIV(data->ui_tree, "wiki-content-parent");
    SET_SIZE(content_parent, 450, 350);
    ui_set_display_flex(content_parent);
    FLEX_COLUMN(content_parent);
    ui_set_justify_content(content_parent, "flex-start");
    ui_set_align_items(content_parent, "center");
    ui_set_flex_gap(content_parent, 8);
    
    // Header
    const char* header_text = "WIKI DU JEU";
    if (data->current_view == WIKI_VIEW_RULES) header_text = "REGLES";
    else if (data->current_view == WIKI_VIEW_STRATEGY) header_text = "STRATEGIES";
    else if (data->current_view == WIKI_VIEW_HISTORY) header_text = "HISTOIRE";
    
    UINode* wiki_header = UI_TEXT(data->ui_tree, "wiki-header", header_text);
    ui_set_text_color(wiki_header, "rgb(255, 165, 0)");
    ui_set_text_size(wiki_header, 20);
    ui_set_text_align(wiki_header, "center");
    ui_set_text_style(wiki_header, true, false);
    atomic_set_margin(wiki_header->element, 24, 0, 0, 0);
    
    APPEND(content_parent, wiki_header);
    
    if (data->current_view == WIKI_VIEW_MENU) {
        // === MENU PRINCIPAL ===
        UINode* sections_container = UI_DIV(data->ui_tree, "sections-container");
        SET_SIZE(sections_container, 440, 180);
        ui_set_display_flex(sections_container);
        FLEX_COLUMN(sections_container);
        ui_set_justify_content(sections_container, "space-between");
        ui_set_align_items(sections_container, "stretch");
        ui_set_flex_gap(sections_container, 0);
        
        // Helper macro for sections
        #define ADD_SECTION(id, title) do { \
            UINode* section = UI_DIV(data->ui_tree, id); \
            SET_SIZE(section, 440, 55); \
            ui_set_display_flex(section); \
            ui_set_flex_direction(section, "row"); \
            ui_set_justify_content(section, "space-between"); \
            ui_set_align_items(section, "center"); \
            atomic_set_background_color(section->element, 0, 0, 0, 50); \
            atomic_set_padding(section->element, 8, 10, 8, 10); \
            make_interactive(section, on_section_click, scene); \
            \
            UINode* text_div = UI_DIV(data->ui_tree, id "-text"); \
            SET_SIZE(text_div, 370, 40); \
            ui_set_display_flex(text_div); \
            FLEX_COLUMN(text_div); \
            ui_set_justify_content(text_div, "center"); \
            \
            UINode* title_node = UI_TEXT(data->ui_tree, id "-title", title); \
            ui_set_text_color(title_node, "rgb(233, 215, 161)"); \
            ui_set_text_size(title_node, 14); \
            ui_set_text_style(title_node, true, false); \
            APPEND(text_div, title_node); \
            \
            UINode* icon = UI_DIV(data->ui_tree, id "-icon"); \
            SET_SIZE(icon, 40, 40); \
            atomic_set_background_color(icon->element, 233, 215, 161, 100); \
            atomic_set_border(icon->element, 2, 233, 215, 161, 255); \
            \
            APPEND(section, text_div); \
            APPEND(section, icon); \
            APPEND(sections_container, section); \
            if (strstr(id, "rules")) ui_animate_slide_in_left(section, 0.8f, 200.0f); \
            else if (strstr(id, "strategy")) ui_animate_slide_in_right(section, 1.0f, 200.0f); \
            else ui_animate_slide_in_left(section, 1.2f, 200.0f); \
        } while(0)

        ADD_SECTION("section-rules", "REGLES DE BASE");
        ADD_SECTION("section-strategy", "STRATEGIES");
        ADD_SECTION("section-history", "HISTOIRE");
        
        APPEND(content_parent, sections_container);
        
        // Bouton retour menu principal
        data->back_link = ui_create_link(data->ui_tree, "back-link", "RETOUR", "menu", SCENE_TRANSITION_REPLACE);
        if (data->back_link) {
            SET_SIZE(data->back_link, 150, 35);
            ui_set_text_align(data->back_link, "center");
            apply_gold_button_style(data->back_link); // 🆕 FIX: Style doré
            atomic_set_margin(data->back_link->element, 8, 0, 0, 0);
            ui_link_set_target_window(data->back_link, WINDOW_TYPE_MINI);
            
            // 🆕 FIX: Reconnecter le lien au manager si le core est disponible (car l'arbre est recréé)
            if (data->core) {
                extern SceneManager* game_core_get_scene_manager(GameCore* core);
                SceneManager* sm = game_core_get_scene_manager(data->core);
                if (sm) ui_link_connect_to_manager(data->back_link, sm);
            }
            
            APPEND(content_parent, data->back_link);
        }
        
    } else {
        // === VUE CONTENU ===
        const char* content_text = "";
        int max_pages = 1;
        
        if (data->current_view == WIKI_VIEW_RULES) {
            content_text = RULES_PAGES[data->current_page];
            max_pages = RULES_PAGE_COUNT;
        } else if (data->current_view == WIKI_VIEW_STRATEGY) {
            content_text = STRATEGY_PAGES[data->current_page];
            max_pages = STRATEGY_PAGE_COUNT;
        } else if (data->current_view == WIKI_VIEW_HISTORY) {
            content_text = HISTORY_PAGES[data->current_page];
            max_pages = HISTORY_PAGE_COUNT;
        }
        
        // Zone de texte
        UINode* text_box = UI_DIV(data->ui_tree, "content-box");
        SET_SIZE(text_box, 400, 180);
        
        // 🆕 FIX: Activer Flexbox pour centrer le texte dans la boîte
        ui_set_display_flex(text_box);
        FLEX_COLUMN(text_box); // 🆕 FIX: Colonne pour gérer les lignes multiples
        ui_set_justify_content(text_box, "center");
        ui_set_align_items(text_box, "center");
        
        atomic_set_background_color(text_box->element, 0, 0, 0, 80);
        atomic_set_border(text_box->element, 1, 233, 215, 161, 100);
        atomic_set_padding(text_box->element, 15, 15, 15, 15);
        
        // 🆕 FIX: Découper le texte en lignes pour éviter les problèmes de rendu et d'encodage
        char buffer[1024];
        strncpy(buffer, content_text, sizeof(buffer));
        buffer[sizeof(buffer)-1] = '\0';
        
        char* current = buffer;
        char* next;
        int line_count = 0;
        
        while (*current) {
            next = strchr(current, '\n');
            if (next) *next = '\0';
            
            // Nettoyer \r éventuel (cause souvent des caractères bizarres)
            size_t len = strlen(current);
            if (len > 0 && current[len-1] == '\r') current[len-1] = '\0';
            
            if (strlen(current) > 0) {
                char line_id[32];
                snprintf(line_id, sizeof(line_id), "line-%d", line_count++);
                UINode* line_node = UI_TEXT(data->ui_tree, line_id, current);
                
                // Style différent pour le titre (première ligne)
                if (line_count == 1) {
                    ui_set_text_color(line_node, "rgb(255, 165, 0)");
                    ui_set_text_size(line_node, 18);
                    ui_set_text_style(line_node, true, false);
                    atomic_set_margin(line_node->element, 0, 0, 10, 0);
                } else {
                    ui_set_text_color(line_node, "rgb(255, 255, 255)");
                    ui_set_text_size(line_node, 16);
                }
                
                ui_set_text_align(line_node, "center");
                APPEND(text_box, line_node);
            } else {
                // Ligne vide = espacement
                char spacer_id[32];
                snprintf(spacer_id, sizeof(spacer_id), "spacer-%d", line_count++);
                UINode* spacer = UI_DIV(data->ui_tree, spacer_id);
                SET_SIZE(spacer, 10, 10);
                APPEND(text_box, spacer);
            }
            
            if (!next) break;
            current = next + 1;
        }
        
        APPEND(content_parent, text_box);
        ui_animate_fade_in(text_box, 0.5f);
        
        // Pagination
        UINode* pagination = UI_DIV(data->ui_tree, "pagination");
        SET_SIZE(pagination, 400, 40);
        ui_set_display_flex(pagination);
        FLEX_ROW(pagination);
        ui_set_justify_content(pagination, "center");
        ui_set_align_items(pagination, "center");
        ui_set_flex_gap(pagination, 20);
        
        // Prev button
        if (data->current_page > 0) {
            UINode* prev_btn = UI_DIV(data->ui_tree, "prev-btn");
            SET_SIZE(prev_btn, 80, 30);
            ui_set_display_flex(prev_btn);
            ui_set_justify_content(prev_btn, "center");
            ui_set_align_items(prev_btn, "center");
            apply_gold_button_style(prev_btn);
            atomic_set_padding(prev_btn->element, 0, 0, 0, 0); // 🆕 FIX: Remove padding for centering
            make_interactive(prev_btn, on_prev_page, scene);
            
            UINode* prev_txt = UI_TEXT(data->ui_tree, "prev-txt", "< PREV");
            SET_SIZE(prev_txt, 80, 30); // 🆕 FIX: Match parent size
            ui_set_text_align(prev_txt, "center");
            ui_set_text_color(prev_txt, "white");
            ui_set_text_size(prev_txt, 12);
            APPEND(prev_btn, prev_txt);
            APPEND(pagination, prev_btn);
        }
        
        // Page indicator
        char page_str[32];
        snprintf(page_str, sizeof(page_str), "%d / %d", data->current_page + 1, max_pages);
        UINode* page_ind = UI_TEXT(data->ui_tree, "page-ind", page_str);
        ui_set_text_color(page_ind, "rgb(233, 215, 161)");
        APPEND(pagination, page_ind);
        
        // Next button
        if (data->current_page < max_pages - 1) {
            UINode* next_btn = UI_DIV(data->ui_tree, "next-btn");
            SET_SIZE(next_btn, 80, 30);
            ui_set_display_flex(next_btn);
            ui_set_justify_content(next_btn, "center");
            ui_set_align_items(next_btn, "center");
            apply_gold_button_style(next_btn);
            atomic_set_padding(next_btn->element, 0, 0, 0, 0); // 🆕 FIX: Remove padding
            make_interactive(next_btn, on_next_page, scene);
            
            UINode* next_txt = UI_TEXT(data->ui_tree, "next-txt", "NEXT >");
            SET_SIZE(next_txt, 80, 30); // 🆕 FIX: Match parent size
            ui_set_text_align(next_txt, "center");
            ui_set_text_color(next_txt, "white");
            ui_set_text_size(next_txt, 12);
            APPEND(next_btn, next_txt);
            APPEND(pagination, next_btn);
        }
        
        APPEND(content_parent, pagination);
        
        // Bouton retour liste
        UINode* back_list = UI_DIV(data->ui_tree, "back-list-btn");
        SET_SIZE(back_list, 150, 35);
        ui_set_display_flex(back_list);
        ui_set_justify_content(back_list, "center");
        ui_set_align_items(back_list, "center");
        apply_gold_button_style(back_list);
        atomic_set_padding(back_list->element, 0, 0, 0, 0); // 🆕 FIX: Remove padding
        make_interactive(back_list, on_back_to_menu, scene);
        
        UINode* back_txt = UI_TEXT(data->ui_tree, "back-list-txt", "RETOUR LISTE");
        SET_SIZE(back_txt, 150, 35); // 🆕 FIX: Match parent size
        ui_set_text_align(back_txt, "center");
        ui_set_text_color(back_txt, "white");
        ui_set_text_size(back_txt, 14);
        APPEND(back_list, back_txt);
        
        APPEND(content_parent, back_list);
    }
    
    ui_container_add_content(modal_container, content_parent);
    ALIGN_SELF_Y(content_parent);
    
    APPEND(data->ui_tree->root, app);
    APPEND(app, modal_container);
    
    ui_calculate_implicit_z_index(data->ui_tree);
    if (scene->event_manager) ui_tree_register_all_events(data->ui_tree);
    
    data->dirty = false;
}

// Mise à jour de la scène Wiki
static void wiki_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    WikiSceneData* data = (WikiSceneData*)scene->data;
    
    // Reconstruire si nécessaire
    if (data->dirty && data->last_renderer) {
        wiki_scene_build_ui(scene, data->last_renderer);
    }
    
    ui_update_animations(delta_time);
    
    if (data->ui_tree) {
        ui_tree_update(data->ui_tree, delta_time);
        if (data->back_link) ui_link_update(data->back_link, delta_time);
    }
}

// Rendu de la scène Wiki
static void wiki_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    WikiSceneData* data = (WikiSceneData*)scene->data;
    
    if (renderer != data->last_renderer) {
        data->last_renderer = renderer;
        data->dirty = true; // Force rebuild on renderer change
    }
    
    if (data->dirty) {
        wiki_scene_build_ui(scene, renderer);
    }
    
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
    if (!scene) return NULL;
    
    scene->id = strdup("wiki");
    scene->name = strdup("Wiki du Jeu");
    
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
    
    scene->init = wiki_scene_init;
    scene->update = wiki_scene_update;
    scene->render = wiki_scene_render;
    scene->cleanup = wiki_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}

// Connexion des événements
void wiki_scene_connect_events(Scene* scene, GameCore* core) {
    if (!scene || !core) return;
    
    WikiSceneData* data = (WikiSceneData*)scene->data;
    if (!data) return;
    
    if (!scene->event_manager) {
        scene->event_manager = event_manager_create();
    }
    
    if (data->ui_tree) {
        data->ui_tree->event_manager = scene->event_manager;
        ui_tree_register_all_events(data->ui_tree);
        scene->ui_tree = data->ui_tree;
    }
    
    data->core = core;
    scene->initialized = true;
    scene->active = true;
    
    // Connecter le lien retour (seulement si présent dans la vue actuelle)
    if (data->back_link) {
        extern SceneManager* game_core_get_scene_manager(GameCore* core);
        SceneManager* scene_manager = game_core_get_scene_manager(core);
        if (scene_manager) {
            ui_link_connect_to_manager(data->back_link, scene_manager);
        }
    }
}
