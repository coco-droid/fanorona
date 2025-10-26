#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"      
#include "../utils/asset_manager.h" 
#include "../pions/pions.h"  // 🆕 AJOUT: Import pour GamePlayer
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
    
    // === 2. CONTENEURS JOUEURS (avec valeurs par défaut) ===
    UINode* players_container = ui_div(tree, "players-container");
    if (players_container) {
        SET_SIZE(players_container, 218, 180);
        ui_set_display_flex(players_container);
        FLEX_COLUMN(players_container);
        ui_set_flex_gap(players_container, 10);
        atomic_set_padding(players_container->element, 0, 0, 0, 0);
        
        // 🔧 FIX: Créer avec valeurs par défaut (Joueur générique)
        GamePlayer default_p1 = {.name = "Joueur 1", .avatar = AVATAR_WARRIOR, .captures_made = 0, .thinking_time = 0.0f};
        GamePlayer default_p2 = {.name = "Joueur 2", .avatar = AVATAR_WARRIOR, .captures_made = 0, .thinking_time = 0.0f};
        
        UINode* player1_node = ui_sidebar_create_player_info(tree, "player1", &default_p1);
        UINode* player2_node = ui_sidebar_create_player_info(tree, "player2", &default_p2);
        
        if (player1_node) APPEND(players_container, player1_node);
        if (player2_node) APPEND(players_container, player2_node);
        
        APPEND(sidebar, players_container);
        printf("📦 Players container créé avec valeurs par défaut (sera mis à jour)\n");
    }
    
    // === 3. CONTENEUR BOUTONS DE CONTRÔLE ===
    ui_sidebar_add_control_buttons(sidebar);
    
    ui_log_event("UIComponent", "Create", id, "Sidebar created - players will be added later");
    printf("✅ Sidebar '%s' créée (joueurs à ajouter par game_scene)\n", id);
    
    return sidebar;
}

// 🔧 FIX: Nouvelle signature avec joueurs réels
void ui_sidebar_add_player_containers(UINode* sidebar, GamePlayer* player1, GamePlayer* player2) {
    if (!sidebar) return;
    
    // 🔧 FIX: Ignore passed parameters, fetch from config directly
    (void)player1; (void)player2;
    
    // 🆕 RÉCUPÉRER DEPUIS LA CONFIG GLOBALE (source unique de vérité)
    extern GameConfig* config_get_current(void);
    GameConfig* cfg = config_get_current();
    
    // 🆕 Recréer les players depuis config pour avoir les données fraîches
    GamePlayer p1 = {
        .name = "",
        .avatar = cfg->player1_avatar,
        .captures_made = 0,
        .thinking_time = 0.0f
    };
    strncpy(p1.name, cfg->player1_name, sizeof(p1.name) - 1);
    
    GamePlayer p2 = {
        .name = "",
        .avatar = cfg->player2_avatar,
        .captures_made = 0,
        .thinking_time = 0.0f
    };
    strncpy(p2.name, cfg->player2_name, sizeof(p2.name) - 1);
    
    printf("📋 Ajout des containers joueurs avec données de CONFIG:\n");
    printf("   👤 J1: %s (Avatar: %s)\n", p1.name, avatar_id_to_filename(p1.avatar));
    printf("   👤 J2: %s (Avatar: %s)\n", p2.name, avatar_id_to_filename(p2.avatar));
    
    // 🔧 FIX: Find nodes ONCE at the beginning
    UINode* player1_node = ui_tree_find_node(sidebar->tree, "player1");
    UINode* player2_node = ui_tree_find_node(sidebar->tree, "player2");
    
    if (!player1_node || !player2_node) {
        printf("❌ Player containers not found in sidebar\n");
        return;
    }
    
    // Update player 1 name AND avatar
    if (player1_node && player1_node->children_count >= 2) {
        // Update avatar (children[0] = avatar_container)
        UINode* p1_avatar_cont = player1_node->children[0];
        if (p1_avatar_cont && p1_avatar_cont->children_count > 0) {
            UINode* avatar_circle = p1_avatar_cont->children[0];
            const char* avatar_path = avatar_id_to_filename(p1.avatar);
            GameWindow* window = use_main_window();
            if (window && avatar_circle) {
                SDL_Renderer* renderer = window_get_renderer(window);
                SDL_Texture* tex = asset_load_texture(renderer, avatar_path);
                if (tex) atomic_set_background_image(avatar_circle->element, tex);
            }
        }
        
        // Update name (children[1] = info_container -> children[0] = name)
        UINode* p1_info = player1_node->children[1];
        if (p1_info && p1_info->children_count > 0) {
            atomic_set_text(p1_info->children[0]->element, p1.name);
        }
    }
    
    // 🔧 FIX: Update player 2 (remove duplicate declaration)
    if (player2_node && player2_node->children_count >= 2) {
        // Update avatar
        UINode* p2_avatar_cont = player2_node->children[0];
        if (p2_avatar_cont && p2_avatar_cont->children_count > 0) {
            UINode* avatar_circle = p2_avatar_cont->children[0];
            const char* avatar_path = avatar_id_to_filename(p2.avatar);
            GameWindow* window = use_main_window();
            if (window && avatar_circle) {
                SDL_Renderer* renderer = window_get_renderer(window);
                SDL_Texture* tex = asset_load_texture(renderer, avatar_path);
                if (tex) atomic_set_background_image(avatar_circle->element, tex);
            }
        }
        
        // Update name
        UINode* p2_info = player2_node->children[1];
        if (p2_info && p2_info->children_count > 0) {
            atomic_set_text(p2_info->children[0]->element, p2.name);
        }
    }
    
    printf("✅ Player data updated in existing containers\n");
    
    // 🔧 FIX: Logs détaillés de l'insertion
    printf("🔍 [DEBUG] Player containers updated in place:\n");
    printf("   - sidebar->children_count: %d\n", sidebar->children_count);
    
    printf("   - sidebar->children_count APRÈS: %d\n", sidebar->children_count);
    printf("   - Players taille: %dx%d\n", 218, 180);
    printf("   - Players background: profile-card.svg (stretch)\n");
    
    // 🆕 LOGS détaillés des joueurs
    printf("📊 === INFOS JOUEURS AFFICHÉES ===\n");
    printf("👤 JOUEUR 1:\n");
    printf("   Nom: '%s'\n", p1.name);
    printf("   Avatar ID: %d\n", (int)p1.avatar);
    printf("   Avatar PNG: '%s'\n", avatar_id_to_filename(p1.avatar));
    printf("   Captures: %d\n", p1.captures_made);
    printf("👤 JOUEUR 2:\n");
    printf("   Nom: '%s'\n", p2.name);
    printf("   Avatar ID: %d\n", (int)p2.avatar);
    printf("   Avatar PNG: '%s'\n", avatar_id_to_filename(p2.avatar));
    printf("   Captures: %d\n", p2.captures_made);
    printf("==================================\n");
    
    ui_log_event("UIComponent", "SidebarPlayers", sidebar->id, "Players inserted at index 1 (before controls)");
}

UINode* ui_sidebar_create_player_info(UITree* tree, const char* id, GamePlayer* player) {
    UINode* player_container = ui_div(tree, id);
    if (!player_container) return NULL;
    
    SET_SIZE(player_container, 218, 80);
    
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
        atomic_set_background_size_str(player_container->element, "stretch");
    } else {
        atomic_set_background_color(player_container->element, 245, 245, 220, 255);
    }
    
    // 🆕 Indicateur de tour actif (bordure dorée)
    if (player && player->is_current_turn) {
        atomic_set_border(player_container->element, 3, 255, 215, 0, 255);
    } else {
        atomic_set_border(player_container->element, 1, 200, 200, 200, 128);
    }
    
    atomic_set_border_radius(player_container->element, 8);
    atomic_set_padding(player_container->element, 5, 5, 5, 5);
    
    // Configuration flexbox horizontale
    ui_set_display_flex(player_container);
    ui_set_flex_direction(player_container, "row");
    ui_set_align_items(player_container, "center");
    ui_set_justify_content(player_container, "space-between");
    ui_set_flex_gap(player_container, 5);
    
    // 🆕 EXTRACTION DES DONNÉES JOUEUR
    const char* player_name = "Joueur";
    AvatarID avatar_id = AVATAR_WARRIOR;
    int captures = 0;
    
    if (player) {
        player_name = player->name;
        avatar_id = player->avatar;
        captures = player->captures_made;
    }
    
    // === MINI-CONTAINER 1: AVATAR ===
    UINode* avatar_container = ui_div(tree, "avatar-mini-container");
    if (avatar_container) {
        SET_SIZE(avatar_container, 45, 70);
        ui_set_display_flex(avatar_container);
        FLEX_COLUMN(avatar_container);
        ui_set_justify_content(avatar_container, "center");
        ui_set_align_items(avatar_container, "center");
        atomic_set_background_color(avatar_container->element, 0, 0, 0, 0); // Transparent
        
        // Avatar avec texture réelle
        UINode* avatar = ui_div(tree, "avatar-circle");
        if (avatar) {
            SET_SIZE(avatar, 40, 40);
            atomic_set_border_radius(avatar->element, 20);
            
            // Charger la texture de l'avatar
            const char* avatar_path = avatar_id_to_filename(avatar_id);
            SDL_Texture* avatar_texture = NULL;
            if (window) {
                SDL_Renderer* renderer = window_get_renderer(window);
                if (renderer) {
                    avatar_texture = asset_load_texture(renderer, avatar_path);
                }
            }
            
            if (avatar_texture) {
                atomic_set_background_image(avatar->element, avatar_texture);
                atomic_set_background_size_str(avatar->element, "cover");
            } else {
                atomic_set_background_color(avatar->element, 128, 128, 128, 255);
            }
            
            atomic_set_border(avatar->element, 2, 255, 215, 0, 255); // Bordure dorée
            APPEND(avatar_container, avatar);
        }
    }
    
    // === MINI-CONTAINER 2: NOM + CAPTURES ===
    UINode* info_container = ui_div(tree, "info-mini-container");
    if (info_container) {
        SET_SIZE(info_container, 90, 70);
        ui_set_display_flex(info_container);
        FLEX_COLUMN(info_container);
        ui_set_justify_content(info_container, "center");
        ui_set_align_items(info_container, "flex-start");
        ui_set_flex_gap(info_container, 4);
        atomic_set_background_color(info_container->element, 0, 0, 0, 0); // Transparent
        atomic_set_padding(info_container->element, 0, 5, 0, 5);
        
        // Nom du joueur
        UINode* player_name_node = ui_text(tree, "player-name", player_name);
        if (player_name_node) {
            ui_set_text_color(player_name_node, "rgb(0, 0, 0)");
            ui_set_text_size(player_name_node, 11);
            ui_set_text_style(player_name_node, true, false);
            APPEND(info_container, player_name_node);
        }
        
        // Captures
        char captures_text[32];
        snprintf(captures_text, sizeof(captures_text), "Captures: %d", captures);
        UINode* captures_label = ui_text(tree, "captures", captures_text);
        if (captures_label) {
            ui_set_text_color(captures_label, "rgb(64, 64, 64)");
            ui_set_text_size(captures_label, 8);
            APPEND(info_container, captures_label);
        }
    }
    
    // === MINI-CONTAINER 3: TIMER + TEMPS ===
    UINode* time_container = ui_div(tree, "time-mini-container");
    if (time_container) {
        SET_SIZE(time_container, 60, 70);
        ui_set_display_flex(time_container);
        FLEX_COLUMN(time_container);
        ui_set_align_items(time_container, "center");
        ui_set_justify_content(time_container, "center");
        ui_set_flex_gap(time_container, 4);
        atomic_set_background_color(time_container->element, 0, 0, 0, 0); // Transparent
        
        // Icône timer.svg plus grande
        SDL_Texture* timer_icon = NULL;
        if (window) {
            SDL_Renderer* renderer = window_get_renderer(window);
            if (renderer) {
                timer_icon = asset_load_texture(renderer, "timer.svg");
            }
        }
        
        if (timer_icon) {
            UINode* timer_image = ui_image(tree, "timer-icon-img", timer_icon);
            if (timer_image) {
                SET_SIZE(timer_image, 20, 20); // 🆕 AUGMENTÉ: 14->20
                APPEND(time_container, timer_image);
            }
        }
        
        // Temps écoulé (calculé depuis thinking_time si disponible)
        char time_text[16];
        if (player) {
            int minutes = ((int)player->thinking_time) / 60;
            int seconds = ((int)player->thinking_time) % 60;
            snprintf(time_text, sizeof(time_text), "%02d:%02d", minutes, seconds);
        } else {
            strcpy(time_text, "00:00");
        }
        
        UINode* time_text_node = ui_text(tree, "time-text", time_text);
        if (time_text_node) {
            ui_set_text_color(time_text_node, "rgb(0, 0, 0)");
            ui_set_text_size(time_text_node, 9);
            ui_set_text_style(time_text_node, true, false);
            APPEND(time_container, time_text_node);
        }
    }
    
    // Assembler les 3 mini-containers
    if (avatar_container) APPEND(player_container, avatar_container);
    if (info_container) APPEND(player_container, info_container);
    if (time_container) APPEND(player_container, time_container);
    
    ui_log_event("UIComponent", "PlayerInfo", id, "Player info with 3 mini-containers and real avatar");
    
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
        atomic_set_background_size_str(controls_container->element, "stretch");  // 🔧 CHANGED: cover -> stretch
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
        atomic_set_background_size_str(button->element, "stretch");  // 🔧 FIX: stretch au lieu de cover
    } else {
        if (is_prominent) {
            atomic_set_background_color(button->element, 218, 165, 32, 255);
        } else {
            atomic_set_background_color(button->element, 192, 192, 192, 255);
        }
    }
    
    atomic_set_border_radius(button->element, 4);
    
    // 🔧 FIX: Flexbox ROW pour alignement horizontal icône + texte
    ui_set_display_flex(button);
    ui_set_flex_direction(button, "row");  // 🔧 CHANGED: row au lieu de column
    ui_set_justify_content(button, "center");
    ui_set_align_items(button, "center");
    ui_set_flex_gap(button, 4);  // 🔧 Gap horizontal entre icône et texte
    
    // Icône SVG plus petite
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
            SET_SIZE(icon_img, 18, 18);  // 🔧 REDUCED: 24->18
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

// 🆕 Fonction pour mettre à jour l'indicateur de tour
void ui_sidebar_update_current_turn(UINode* sidebar, GamePlayer* current_player) {
    if (!sidebar || !current_player) return;
    
    UINode* player1_node = ui_tree_find_node(sidebar->tree, "player1");
    UINode* player2_node = ui_tree_find_node(sidebar->tree, "player2");
    
    if (player1_node) {
        bool is_active = (current_player->player_number == 1);
        if (is_active) {
            atomic_set_border(player1_node->element, 3, 255, 215, 0, 255);
        } else {
            atomic_set_border(player1_node->element, 1, 200, 200, 200, 128);
        }
    }
    
    if (player2_node) {
        bool is_active = (current_player->player_number == 2);
        if (is_active) {
            atomic_set_border(player2_node->element, 3, 255, 215, 0, 255);
        } else {
            atomic_set_border(player2_node->element, 1, 200, 200, 200, 128);
        }
    }
}
