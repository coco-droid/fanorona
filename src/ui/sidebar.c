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
    
    // Style de la sidebar : fond bois foncé, occupe 1/3 de l'écran
    atomic_set_background_color(sidebar->element, 101, 67, 33, 255); // Bois foncé
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
    
    // Container pour les deux joueurs
    UINode* players_container = ui_div(sidebar->tree, "players-container");
    if (players_container) {
        SET_SIZE(players_container, 200, 140);
        ui_set_display_flex(players_container);
        FLEX_COLUMN(players_container);
        ui_set_flex_gap(players_container, 10);
        
        // Joueur 1
        UINode* player1 = ui_sidebar_create_player_info(sidebar->tree, "player1", "Joueur 1", 0, "08:00");
        if (player1) APPEND(players_container, player1);
        
        // Joueur 2
        UINode* player2 = ui_sidebar_create_player_info(sidebar->tree, "player2", "Joueur 2", 0, "08:00");
        if (player2) APPEND(players_container, player2);
        
        APPEND(sidebar, players_container);
        
        ui_log_event("UIComponent", "SidebarPlayers", sidebar->id, "Player containers added");
    }
}

UINode* ui_sidebar_create_player_info(UITree* tree, const char* id, const char* name, int captures, const char* time) {
    UINode* player_container = ui_div(tree, id);
    if (!player_container) return NULL;
    
    // Style conteneur joueur : crème avec coins arrondis
    SET_SIZE(player_container, 200, 60);
    atomic_set_background_color(player_container->element, 245, 245, 220, 255); // Couleur crème
    atomic_set_border_radius(player_container->element, 8);
    atomic_set_padding(player_container->element, 8, 8, 8, 8);
    
    // Configuration flexbox horizontale
    ui_set_display_flex(player_container);
    ui_set_flex_direction(player_container, "row");
    ui_set_align_items(player_container, "center");
    ui_set_justify_content(player_container, "space-between");
    
    // === AVATAR (cercle gris) ===
    UINode* avatar = ui_div(tree, "avatar");
    if (avatar) {
        SET_SIZE(avatar, 40, 40);
        atomic_set_background_color(avatar->element, 128, 128, 128, 255); // Gris
        atomic_set_border_radius(avatar->element, 20); // Cercle
    }
    
    // === INFOS JOUEUR (nom + captures) ===
    UINode* info_container = ui_div(tree, "info-container");
    if (info_container) {
        SET_SIZE(info_container, 80, 50);
        ui_set_display_flex(info_container);
        FLEX_COLUMN(info_container);
        ui_set_justify_content(info_container, "center");
        
        // Nom du joueur
        UINode* player_name = ui_text(tree, "player-name", name);
        if (player_name) {
            ui_set_text_color(player_name, "rgb(0, 0, 0)");
            ui_set_text_size(player_name, 12);
            ui_set_text_style(player_name, true, false);
            APPEND(info_container, player_name);
        }
        
        // Captures
        char captures_text[32];
        snprintf(captures_text, sizeof(captures_text), "Captures:%d", captures);
        UINode* captures_label = ui_text(tree, "captures", captures_text);
        if (captures_label) {
            ui_set_text_color(captures_label, "rgb(0, 0, 0)");
            ui_set_text_size(captures_label, 10);
            APPEND(info_container, captures_label);
        }
    }
    
    // === TEMPS (chronomètre) ===
    UINode* time_container = ui_div(tree, "time-container");
    if (time_container) {
        SET_SIZE(time_container, 60, 50);
        ui_set_display_flex(time_container);
        ui_set_flex_direction(time_container, "row");
        ui_set_align_items(time_container, "center");
        ui_set_justify_content(time_container, "center");
        
        // Icône chronomètre (⏱️ en texte pour simplicité)
        UINode* timer_icon = ui_text(tree, "timer-icon", "⏱️");
        if (timer_icon) {
            ui_set_text_size(timer_icon, 14);
            APPEND(time_container, timer_icon);
        }
        
        // Temps restant
        UINode* time_text = ui_text(tree, "time-text", time);
        if (time_text) {
            ui_set_text_color(time_text, "rgb(0, 0, 0)");
            ui_set_text_size(time_text, 10);
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
    
    // Container pour les boutons de contrôle
    UINode* controls_container = ui_div(sidebar->tree, "controls-container");
    if (!controls_container) return;
    
    SET_SIZE(controls_container, 200, 120);
    ui_set_display_flex(controls_container);
    ui_set_flex_direction(controls_container, "column");
    ui_set_flex_gap(controls_container, 10);
    
    // === RANGÉE DU HAUT (Pause + Analyse) ===
    UINode* top_row = ui_div(sidebar->tree, "controls-top-row");
    if (top_row) {
        SET_SIZE(top_row, 200, 50);
        ui_set_display_flex(top_row);
        ui_set_flex_direction(top_row, "row");
        ui_set_justify_content(top_row, "space-between");
        
        // Bouton Pause
        UINode* pause_btn = ui_sidebar_create_control_button(sidebar->tree, "pause-btn", "⏸️", "PAUSE", false);
        if (pause_btn) APPEND(top_row, pause_btn);
        
        // Bouton Analyse
        UINode* analysis_btn = ui_sidebar_create_control_button(sidebar->tree, "analysis-btn", "📊", "ANALYSE", false);
        if (analysis_btn) APPEND(top_row, analysis_btn);
        
        APPEND(controls_container, top_row);
    }
    
    // === RANGÉE DU BAS (Paramètres + Quit) ===
    UINode* bottom_row = ui_div(sidebar->tree, "controls-bottom-row");
    if (bottom_row) {
        SET_SIZE(bottom_row, 200, 50);
        ui_set_display_flex(bottom_row);
        ui_set_flex_direction(bottom_row, "row");
        ui_set_justify_content(bottom_row, "space-between");
        
        // Bouton Paramètres
        UINode* settings_btn = ui_sidebar_create_control_button(sidebar->tree, "settings-btn", "⚙️", "PARAM", false);
        if (settings_btn) APPEND(bottom_row, settings_btn);
        
        // Bouton Quit (proéminent)
        UINode* quit_btn = ui_sidebar_create_control_button(sidebar->tree, "quit-btn", "🚪", "QUIT", true);
        if (quit_btn) APPEND(bottom_row, quit_btn);
        
        APPEND(controls_container, bottom_row);
    }
    
    APPEND(sidebar, controls_container);
    ui_log_event("UIComponent", "SidebarControls", sidebar->id, "Control buttons grid added");
}

UINode* ui_sidebar_create_control_button(UITree* tree, const char* id, const char* icon, const char* text, bool is_prominent) {
    UINode* button = ui_button(tree, id, text, NULL, NULL);
    if (!button) return NULL;
    
    SET_SIZE(button, 90, 45);
    
    if (is_prominent) {
        // Style proéminent pour le bouton Quit (ocre/beige)
        atomic_set_background_color(button->element, 218, 165, 32, 255); // Couleur ocre
        ui_set_text_color(button, "rgb(0, 0, 0)");
    } else {
        // Style normal (gris clair)
        atomic_set_background_color(button->element, 192, 192, 192, 255); // Gris clair
        ui_set_text_color(button, "rgb(0, 0, 0)");
    }
    
    atomic_set_border_radius(button->element, 4);
    ui_set_text_size(button, 10);
    ui_set_text_style(button, true, false);
    ui_set_text_align(button, "center");
    
    // Ajouter l'icône au texte (pour simplicité)
    char button_content[64];
    snprintf(button_content, sizeof(button_content), "%s %s", icon, text);
    atomic_set_text(button->element, button_content);
    
    return button;
}
