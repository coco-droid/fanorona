#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"      
#include "../utils/asset_manager.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === SIDEBAR COMPONENT ===

UINode* ui_sidebar(UITree* tree, const char* id) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    // Créer le container principal de la sidebar
    UINode* sidebar = ui_div(tree, id);
    if (!sidebar) {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create sidebar");
        return NULL;
    }
    
    // Style de la sidebar : fond avec image bg-bg.png
    SDL_Texture* bg_texture = NULL;
    GameWindow* window = use_main_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            bg_texture = asset_load_texture(renderer, "bg-bg.png");
        }
    }
    
    if (bg_texture) {
        atomic_set_background_image(sidebar->element, bg_texture);
    } else {
        atomic_set_background_color(sidebar->element, 101, 67, 33, 255); // Fallback bois foncé
    }
    atomic_set_padding(sidebar->element, 15, 15, 15, 15);
    
    // Configuration flexbox (colonne)
    ui_set_display_flex(sidebar);
    FLEX_COLUMN(sidebar);
    ui_set_justify_content(sidebar, "flex-start");
    ui_set_align_items(sidebar, "center");
    ui_set_flex_gap(sidebar, 20);
    
    // === 1. TITRE DU JEU ===
    UINode* title = ui_text(tree, "sidebar-title", "FANORONA");
    if (title) {
        ui_set_text_color(title, "rgb(255, 255, 255)");
        ui_set_text_size(title, 24);
        ui_set_text_align(title, "center");
        ui_set_text_style(title, true, false); // Gras
        APPEND(sidebar, title);
        
        ui_log_event("UIComponent", "SidebarTitle", id, "Game title added");
    }
    
    // === 2. CONTENEURS JOUEURS ===
    ui_sidebar_add_player_containers(sidebar);
    
    // === 3. CONTENEUR BOUTONS DE CONTRÔLE ===
    ui_sidebar_add_control_buttons(sidebar);
    
    ui_log_event("UIComponent", "Create", id, "Sidebar created with all sections");
    printf("✅ Sidebar '%s' créée avec :\n", id);
    printf("   📝 Titre FANORONA en blanc gras\n");
    printf("   👥 2 conteneurs joueurs avec infos complètes\n");
    printf("   🎮 4 boutons de contrôle en grille 2x2\n");
    
    return sidebar;
}

// === FONCTIONS HELPER POUR LES SECTIONS ===

void ui_sidebar_add_player_containers(UINode* sidebar) {
    if (!sidebar) return;
    
    // 🔧 FIX: Container à 95% largeur, hauteur réduite
    UINode* players_container = ui_div(sidebar->tree, "players-container");
    if (players_container) {
        SET_SIZE(players_container, 218, 180); // 🔧 FIX: Hauteur réduite (230->180)
        ui_set_display_flex(players_container);
        FLEX_COLUMN(players_container);
        ui_set_flex_gap(players_container, 10);
        
        // 🔧 FIX: AUCUN padding pour éviter réduction interne
        atomic_set_padding(players_container->element, 0, 0, 0, 0);
        
        // Joueur 1
        UINode* player1 = ui_sidebar_create_player_info(sidebar->tree, "player1", "Joueur 1", 0, "08:00");
        if (player1) APPEND(players_container, player1);
        
        // Joueur 2
        UINode* player2 = ui_sidebar_create_player_info(sidebar->tree, "player2", "Joueur 2", 0, "08:00");
        if (player2) APPEND(players_container, player2);
        
        APPEND(sidebar, players_container);
        
        ui_log_event("UIComponent", "SidebarPlayers", sidebar->id, "Player containers at 95% width with no padding");
    }
}

UINode* ui_sidebar_create_player_info(UITree* tree, const char* id, const char* name, int captures, const char* time) {
    UINode* player_container = ui_div(tree, id);
    if (!player_container) return NULL;
    
    // 🔧 FIX: 100% largeur parent, hauteur réduite
    SET_SIZE(player_container, 218, 80); // 🔧 FIX: Hauteur réduite (110->80)
    
    SDL_Texture* card_texture = NULL;
    GameWindow* window = use_main_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            card_texture = asset_load_texture(renderer, "profile-card.svg");
        }
    }
    
    if (card_texture) {
        atomic_set_background_image(player_container->element, card_texture);
    } else {
        atomic_set_background_color(player_container->element, 245, 245, 220, 255);
    }
    atomic_set_border_radius(player_container->element, 8);
    atomic_set_padding(player_container->element, 5, 5, 5, 5); // 🔧 FIX: Padding réduit
    
    // Configuration flexbox horizontale
    ui_set_display_flex(player_container);
    ui_set_flex_direction(player_container, "row");
    ui_set_align_items(player_container, "center");
    ui_set_justify_content(player_container, "space-between");
    
    // === AVATAR (cercle) ===
    UINode* avatar = ui_div(tree, "avatar");
    if (avatar) {
        SET_SIZE(avatar, 35, 35); // 🔧 FIX: Taille réduite
        atomic_set_background_color(avatar->element, 128, 128, 128, 255);
        atomic_set_border_radius(avatar->element, 18);
    }
    
    // === INFOS JOUEUR (nom + captures) ===
    UINode* info_container = ui_div(tree, "info-container");
    if (info_container) {
        SET_SIZE(info_container, 75, 40); // 🔧 FIX: Hauteur réduite
        ui_set_display_flex(info_container);
        FLEX_COLUMN(info_container);
        ui_set_justify_content(info_container, "center");
        
        // Nom du joueur
        UINode* player_name = ui_text(tree, "player-name", name);
        if (player_name) {
            ui_set_text_color(player_name, "rgb(0, 0, 0)");
            ui_set_text_size(player_name, 10); // 🔧 FIX: Taille réduite
            ui_set_text_style(player_name, true, false);
            APPEND(info_container, player_name);
        }
        
        // Captures
        char captures_text[32];
        snprintf(captures_text, sizeof(captures_text), "Cap:%d", captures); // 🔧 FIX: Texte court
        UINode* captures_label = ui_text(tree, "captures", captures_text);
        if (captures_label) {
            ui_set_text_color(captures_label, "rgb(0, 0, 0)");
            ui_set_text_size(captures_label, 8); // 🔧 FIX: Taille réduite
            APPEND(info_container, captures_label);
        }
    }
    
    // === TEMPS avec icône timer.svg ===
    UINode* time_container = ui_div(tree, "time-container");
    if (time_container) {
        SET_SIZE(time_container, 50, 40); // 🔧 FIX: Taille ajustée
        ui_set_display_flex(time_container);
        FLEX_COLUMN(time_container); // 🔧 FIX: Colonne pour icône au-dessus du texte
        ui_set_align_items(time_container, "center");
        ui_set_justify_content(time_container, "center");
        ui_set_flex_gap(time_container, 2); // 🔧 FIX: Petit gap
        
        // Charger timer.svg comme icône
        SDL_Texture* timer_icon = NULL;
        GameWindow* window = use_main_window();
        if (window) {
            SDL_Renderer* renderer = window_get_renderer(window);
            if (renderer) {
                timer_icon = asset_load_texture(renderer, "timer.svg");
            }
        }
        
        if (timer_icon) {
            UINode* timer_image = ui_image(tree, "timer-icon-img", timer_icon);
            if (timer_image) {
                SET_SIZE(timer_image, 14, 14); // 🔧 FIX: Taille réduite
                APPEND(time_container, timer_image);
            }
        }
        
        // Temps restant (en dessous de l'icône)
        UINode* time_text = ui_text(tree, "time-text", time);
        if (time_text) {
            ui_set_text_color(time_text, "rgb(0, 0, 0)");
            ui_set_text_size(time_text, 8); // 🔧 FIX: Taille réduite
            ui_set_text_style(time_text, true, false);
            APPEND(time_container, time_text);
        }
    }
    
    // Assembler le conteneur joueur
    if (avatar) APPEND(player_container, avatar);
    if (info_container) APPEND(player_container, info_container);
    if (time_container) APPEND(player_container, time_container);
    
    return player_container;
}

void ui_sidebar_add_control_buttons(UINode* sidebar) {
    if (!sidebar) return;
    
    // Container avec background Rectangle.svg
    UINode* controls_container = ui_div(sidebar->tree, "controls-container");
    if (!controls_container) return;
    
    SET_SIZE(controls_container, 218, 150); // 🔧 FIX: Hauteur réduite (170->150)
    
    SDL_Texture* container_bg = NULL;
    GameWindow* window = use_main_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            container_bg = asset_load_texture(renderer, "Rectangle.svg");
        }
    }
    
    if (container_bg) {
        atomic_set_background_image(controls_container->element, container_bg);
        atomic_set_background_size_str(controls_container->element, "cover");
    } else {
        atomic_set_background_color(controls_container->element, 64, 64, 64, 255);
    }
    
    ui_set_display_flex(controls_container);
    ui_set_flex_direction(controls_container, "column");
    ui_set_flex_gap(controls_container, 8); // 🔧 FIX: Gap réduit
    atomic_set_padding(controls_container->element, 8, 8, 8, 8); // 🔧 FIX: Padding réduit
    
    // === RANGÉE DU HAUT (Pause + Analyse) ===
    UINode* top_row = ui_div(sidebar->tree, "controls-top-row");
    if (top_row) {
        SET_SIZE(top_row, 202, 65); // 🔧 FIX: Hauteur réduite
        ui_set_display_flex(top_row);
        ui_set_flex_direction(top_row, "row");
        ui_set_justify_content(top_row, "space-between");
        
        UINode* pause_btn = ui_sidebar_create_control_button(sidebar->tree, "pause-btn", "pause.svg", "PAUSE", false);
        if (pause_btn) APPEND(top_row, pause_btn);
        
        UINode* analysis_btn = ui_sidebar_create_control_button(sidebar->tree, "analysis-btn", "sheet.svg", "ANALYSE", false);
        if (analysis_btn) APPEND(top_row, analysis_btn);
        
        APPEND(controls_container, top_row);
    }
    
    // === RANGÉE DU BAS (Paramètres + Quit) ===
    UINode* bottom_row = ui_div(sidebar->tree, "controls-bottom-row");
    if (bottom_row) {
        SET_SIZE(bottom_row, 202, 65); // 🔧 FIX: Hauteur réduite
        ui_set_display_flex(bottom_row);
        ui_set_flex_direction(bottom_row, "row");
        ui_set_justify_content(bottom_row, "space-between");
        
        UINode* settings_btn = ui_sidebar_create_control_button(sidebar->tree, "settings-btn", "setting.svg", "PARAM", false);
        if (settings_btn) APPEND(bottom_row, settings_btn);
        
        UINode* quit_btn = ui_sidebar_create_control_button(sidebar->tree, "quit-btn", "leave.svg", "QUIT", true);
        if (quit_btn) APPEND(bottom_row, quit_btn);
        
        APPEND(controls_container, bottom_row);
    }
    
    APPEND(sidebar, controls_container);
    ui_log_event("UIComponent", "SidebarControls", sidebar->id, "Controls with proper sizing");
}

UINode* ui_sidebar_create_control_button(UITree* tree, const char* id, const char* icon_svg, const char* text, bool is_prominent) {
    UINode* button = ui_div(tree, id);
    if (!button) return NULL;
    
    // 🔧 FIX: Boutons bien dimensionnés
    SET_SIZE(button, 95, 55); // 🔧 FIX: Hauteur réduite (60->55)
    
    SDL_Texture* btn_bg = NULL;
    GameWindow* window = use_main_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            const char* bg_file = is_prominent ? "btn-brun.svg" : "btn.svg";
            btn_bg = asset_load_texture(renderer, bg_file);
        }
    }
    
    if (btn_bg) {
        atomic_set_background_image(button->element, btn_bg);
        atomic_set_background_size_str(button->element, "cover");
    } else {
        if (is_prominent) {
            atomic_set_background_color(button->element, 218, 165, 32, 255);
        } else {
            atomic_set_background_color(button->element, 192, 192, 192, 255);
        }
    }
    
    atomic_set_border_radius(button->element, 4);
    
    // Flexbox column
    ui_set_display_flex(button);
    FLEX_COLUMN(button);
    ui_set_justify_content(button, "center");
    ui_set_align_items(button, "center");
    ui_set_flex_gap(button, 3); // 🔧 FIX: Gap réduit
    
    // Icône SVG
    SDL_Texture* icon_texture = NULL;
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            icon_texture = asset_load_texture(renderer, icon_svg);
        }
    }
    
    if (icon_texture) {
        UINode* icon_img = ui_image(tree, "btn-icon", icon_texture);
        if (icon_img) {
            SET_SIZE(icon_img, 24, 24); // 🔧 FIX: Icône plus grande
            APPEND(button, icon_img);
        }
    }
    
    // Texte du bouton
    UINode* text_node = ui_text(tree, "btn-text", text);
    if (text_node) {
        ui_set_text_size(text_node, 7); // 🔧 FIX: Taille réduite (8->7)
        ui_set_text_style(text_node, true, false);
        ui_set_text_align(text_node, "center");
        ui_set_text_color(text_node, "rgb(255, 255, 255)");
        APPEND(button, text_node);
    }
    
    return button;
}
