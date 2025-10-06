#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "ui_tree.h"
#include <SDL2/SDL_ttf.h>

// Forward declarations
typedef struct GamePlayer GamePlayer;

// === FONCTIONS DE CRÉATION DE COMPOSANTS ===

// Syntaxe simplifiée pour créer des composants
UINode* ui_div(UITree* tree, const char* id);
UINode* ui_text(UITree* tree, const char* id, const char* content);
UINode* ui_image(UITree* tree, const char* id, SDL_Texture* texture);

// === FONCTIONS FLUIDES (CHAÎNABLES) ===

// Positionnement et taille
UINode* ui_set_position(UINode* node, int x, int y);
UINode* ui_set_size(UINode* node, int width, int height);
UINode* ui_set_z_index(UINode* node, int z_index);

// Style fluide
UINode* ui_set_background(UINode* node, const char* color);
UINode* ui_set_background_image(UINode* node, const char* image_path);
UINode* ui_set_background_size(UINode* node, const char* size);
UINode* ui_set_background_repeat(UINode* node, const char* repeat);
UINode* ui_set_color(UINode* node, const char* color);
UINode* ui_add_class(UINode* node, const char* class_name);

// Alignement et positionnement
UINode* ui_set_align(UINode* node, const char* horizontal, const char* vertical);
UINode* ui_center(UINode* node); // Centre automatiquement
UINode* ui_center_x(UINode* node); // Centre horizontalement
UINode* ui_center_y(UINode* node); // Centre verticalement

// 🆕 NOUVELLES FONCTIONS pour la gestion de l'overflow
UINode* ui_set_overflow(UINode* node, const char* overflow);
UINode* ui_set_overflow_visible(UINode* node);   // Les enfants peuvent déborder
UINode* ui_set_overflow_hidden(UINode* node);    // Les enfants sont contraints
UINode* ui_set_overflow_scroll(UINode* node);    // Avec scroll (futur)
UINode* ui_set_overflow_auto(UINode* node);      // Automatique

// Fonctions utilitaires pour l'overflow
bool ui_is_child_overflowing(UINode* parent, UINode* child);
void ui_constrain_all_children(UINode* parent);

// 🆕 NOUVELLES FONCTIONS pour align-self
UINode* ui_set_align_self(UINode* node, const char* align_self);
UINode* ui_set_align_self_center_x(UINode* node);
UINode* ui_set_align_self_center_y(UINode* node);
UINode* ui_set_align_self_center_both(UINode* node);

// Flexbox
UINode* ui_set_flex_direction(UINode* node, const char* direction);
UINode* ui_set_justify_content(UINode* node, const char* justify);
UINode* ui_set_align_items(UINode* node, const char* align);
UINode* ui_set_flex_gap(UINode* node, int gap);
UINode* ui_set_display_flex(UINode* node);

// Texte et polices
UINode* ui_set_font(UINode* node, const char* font_path, int size);
UINode* ui_set_text_color(UINode* node, const char* color);
UINode* ui_set_text_align(UINode* node, const char* align);
UINode* ui_set_text_style(UINode* node, bool bold, bool italic);
UINode* ui_set_font_size(UINode* node, int size);  // 🔧 FIX: UINode* au lieu de void
void ui_set_text_size(UINode* node, int size);     // 🔧 AJOUTÉ: Déclaration de la fonction void

// Événements fluides
UINode* ui_on_click(UINode* node, void (*callback)(UINode*, void*));
UINode* ui_on_hover(UINode* node, void (*callback)(UINode*, void*));

// Hiérarchie fluide
UINode* ui_append_to(UINode* child, UINode* parent);
UINode* ui_append(UINode* parent, UINode* child);

// Button component function
UINode* ui_button(UITree* tree, const char* id, const char* text, void (*onClick)(UINode* node, void* user_data), void* user_data);

// === CONTAINER COMPONENT ===

// Créer un container avec style modal (fond noir transparent, bordure orange)
// + Logo et sous-titre "Stratégie et Tradition" par défaut
UINode* ui_container(UITree* tree, const char* id);

// Container avec taille spécifiée
UINode* ui_container_with_size(UITree* tree, const char* id, int width, int height);

// Container centré automatiquement
UINode* ui_container_centered(UITree* tree, const char* id, int width, int height);

// 🆕 NOUVELLES FONCTIONS pour le contenu par défaut
void ui_container_add_default_logo(UINode* container);
void ui_container_add_default_subtitle(UINode* container);

// Ajouter du contenu au container (positionnement automatique sous le sous-titre)
void ui_container_add_content(UINode* container, UINode* content);

// Ajouter un en-tête orange au container
void ui_container_add_header(UINode* container, const char* header_text);

// Basculer entre style modal et normal
void ui_container_set_modal_style(UINode* container, bool is_modal);

// === NEON BUTTON COMPONENT ===

// 🔧 NOTE: Les neon buttons sont implémentés dans neon_btn.c (pas dans ui_components.c)
// Créer un bouton avec effet neon hover automatique
UINode* ui_neon_button(UITree* tree, const char* id, const char* text, 
                       void (*onClick)(UINode* node, void* user_data), void* user_data);

// Personnaliser la couleur de lueur
void ui_neon_button_set_glow_color(UINode* neon_btn, int r, int g, int b);

// Contrôler la vitesse d'animation
void ui_neon_button_set_animation_speed(UINode* neon_btn, float speed);

// Mettre à jour toutes les animations neon (appelé par le système)
void ui_neon_button_update_all(UITree* tree, float delta_time);

// Button styling functions
void ui_button_set_style(UINode* button, const char* bg_color, const char* text_color, const char* border_color);
void ui_button_set_hover_style(UINode* button, const char* hover_bg_color, const char* hover_text_color);
void ui_button_set_pressed_style(UINode* button, const char* pressed_bg_color, const char* pressed_text_color);

// 🆕 NOUVELLES FONCTIONS pour l'enregistrement des événements
void ui_button_register_events(UINode* button, UITree* tree);
void ui_tree_register_all_events(UITree* tree);

// === NOUVELLES FONCTIONS DE DEBUGGING ===

// Activer/désactiver les logs d'événements
void ui_set_event_logging(bool enabled);
bool ui_is_event_logging_enabled(void);

// 🆕 HITBOX VISUALIZATION SYSTEM
// Contrôler l'affichage des hitboxes (rectangles rouges transparents avec bordure bleue)
void ui_set_hitbox_visualization(bool enabled);
bool ui_is_hitbox_visualization_enabled(void);

// Logs spécifiques pour le debugging
void ui_log_event(const char* source, const char* event_type, const char* element_id, const char* message);

// === GESTION Z-INDEX IMPLICITE ===

// Calculer et assigner les z-index automatiquement
void ui_calculate_implicit_z_index(UITree* tree);
void ui_node_set_implicit_z_index(UINode* node, int base_z_index);

// Obtenir le z-index effectif (explicite ou implicite)
int ui_node_get_effective_z_index(UINode* node);

// === CORRECTIONS BOUTONS ===

// Définir l'image de fond spécifiquement pour les boutons
void ui_button_set_background_image(UINode* button, const char* image_path);

// Corriger l'affichage du texte sur les boutons
void ui_button_fix_text_rendering(UINode* button);

// Calculer la position optimale pour le texte du bouton
void ui_button_calculate_text_position(UINode* button);

// Police par défaut
TTF_Font* ui_get_default_font(void);
void ui_cleanup_fonts(void);

// Fonctions de texte avec TTF
UINode* ui_text_with_font(UITree* tree, const char* id, const char* content, TTF_Font* font);
void ui_set_text_font(UINode* node, TTF_Font* font);
void ui_set_text_size(UINode* node, int size);

// === HELPERS POUR STYLE CSS-LIKE ===

typedef struct {
    int x, y, width, height, z_index;
    char* background_color;
    char* background_image;
    char* color;
    char* display;
    char* align_horizontal;
    char* align_vertical;
    char* flex_direction;
    char* justify_content;
    char* align_items;
    int flex_gap;
    char* font_path;
    int font_size;
    char* text_align;
    bool text_bold;
    bool text_italic;
} UIStyle;

void ui_apply_style(UINode* node, const UIStyle* style);

// === DEBUGGING DU TEXTE ===

// Déboguer le rendu du texte
void ui_debug_text_rendering(UINode* node, const char* context);

// === NOUVELLES FONCTIONS POUR FEEDBACK VISUEL ===

// Gestion des états visuels des boutons
void ui_button_set_pressed_state(UINode* button, bool pressed);
void ui_button_set_hover_state(UINode* button, bool hovered);
void ui_button_reset_visual_state(UINode* button);

// Animation et transitions
void ui_button_animate_click(UINode* button, int duration_ms);
void ui_button_animate_hover(UINode* button, bool entering);

// Couleurs d'état prédéfinies
void ui_button_apply_success_style(UINode* button);   // Vert pour actions positives
void ui_button_apply_danger_style(UINode* button);    // Rouge pour actions dangereuses
void ui_button_apply_info_style(UINode* button);      // Bleu pour informations
void ui_button_apply_warning_style(UINode* button);   // Orange pour avertissements

// Effets visuels avancés
void ui_button_add_glow_effect(UINode* button, const char* color);
void ui_button_add_shadow_effect(UINode* button, int offset_x, int offset_y);
void ui_button_remove_all_effects(UINode* button);

// 🆕 Mouse event handlers for plateau
void ui_plateau_set_mouse_handlers(UINode* plateau);
void ui_plateau_update_visual_feedback(UINode* plateau, float delta_time);

// 🆕 Game state integration
void ui_plateau_set_game_logic(UINode* plateau, void* game_logic);
void* ui_plateau_get_game_logic(UINode* plateau);

// === MACROS POUR FEEDBACK VISUEL ===

#define BUTTON_PRESSED(btn) ui_button_set_pressed_state(btn, true)
#define BUTTON_RELEASED(btn) ui_button_set_pressed_state(btn, false)
#define BUTTON_HOVER_ON(btn) ui_button_set_hover_state(btn, true)
#define BUTTON_HOVER_OFF(btn) ui_button_set_hover_state(btn, false)
#define BUTTON_RESET(btn) ui_button_reset_visual_state(btn)

// Styles prédéfinis
#define BUTTON_SUCCESS(btn) ui_button_apply_success_style(btn)
#define BUTTON_DANGER(btn) ui_button_apply_danger_style(btn)
#define BUTTON_INFO(btn) ui_button_apply_info_style(btn)
#define BUTTON_WARNING(btn) ui_button_apply_warning_style(btn)

// === MACROS POUR SYNTAXE SIMPLIFIÉE ===

#define UI_BUTTON(tree, id, label) ui_button(tree, id, label)
#define UI_DIV(tree, id) ui_div(tree, id)
#define UI_TEXT(tree, id, content) ui_text(tree, id, content)
#define UI_IMAGE(tree, id, texture) ui_image(tree, id, texture)
#define UI_CONTAINER(tree, id) ui_container(tree, id)
#define UI_CONTAINER_CENTERED(tree, id, w, h) ui_container_centered(tree, id, w, h)

// 🔧 FIX: Ajouter les macros manquantes
#define FLEX_ROW(node) ui_set_flex_direction(node, "row")
#define FLEX_COLUMN(node) ui_set_flex_direction(node, "column")
#define APPEND(parent, child) ui_append(parent, child)

// 🆕 NOUVELLES MACROS pour align-self
#define ALIGN_SELF_X(node) ui_set_align_self_center_x(node)
#define ALIGN_SELF_Y(node) ui_set_align_self_center_y(node)
#define ALIGN_SELF_BOTH(node) ui_set_align_self_center_both(node)

// 🆕 NOUVELLES MACROS pour overflow
#define SET_OVERFLOW(node, type) ui_set_overflow(node, type)
#define OVERFLOW_HIDDEN(node) ui_set_overflow_hidden(node)
#define OVERFLOW_VISIBLE(node) ui_set_overflow_visible(node)
#define OVERFLOW_SCROLL(node) ui_set_overflow_scroll(node)
#define OVERFLOW_AUTO(node) ui_set_overflow_auto(node)

// Macro de debugging
#define DEBUG_TEXT(node) ui_debug_text_rendering(node, #node)

// === SIDEBAR COMPONENT ===

// Créer une sidebar complète avec titre, joueurs et boutons
UINode* ui_sidebar(UITree* tree, const char* id);

// Fonctions helper pour les sections de la sidebar
void ui_sidebar_add_player_containers(UINode* sidebar);
UINode* ui_sidebar_create_player_info(UITree* tree, const char* id, const char* name, int captures, const char* time);
void ui_sidebar_add_control_buttons(UINode* sidebar);
UINode* ui_sidebar_create_control_button(UITree* tree, const char* id, const char* icon, const char* text, bool is_prominent);

// === CNT_PLAYABLE COMPONENT ===

// Créer une zone de jeu (2/3 de l'écran)
UINode* ui_cnt_playable(UITree* tree, const char* id);

// Ajouter la zone de jeu centrale
void ui_cnt_playable_add_game_area(UINode* playable_container);

// Container de jeu avec taille spécifiée
UINode* ui_cnt_playable_with_size(UITree* tree, const char* id, int width, int height);

// === PLATEAU COMPONENT ===

// Forward declaration pour Board
typedef struct Board Board;

// Créer un container de plateau avec damier Fanorona et joueurs
UINode* ui_plateau_container(UITree* tree, const char* id);
UINode* ui_plateau_container_with_players(UITree* tree, const char* id, GamePlayer* player1, GamePlayer* player2);

// Plateau avec taille personnalisée
UINode* ui_plateau_container_with_size(UITree* tree, const char* id, int width, int height);

// Configuration d'affichage
void ui_plateau_set_show_intersections(UINode* plateau, bool show);
void ui_plateau_set_show_coordinates(UINode* plateau, bool show);

// Accès aux données du plateau
Board* ui_plateau_get_board(UINode* plateau);
void ui_plateau_update_from_board(UINode* plateau, Board* new_board);

// 🆕 NOUVELLES FONCTIONS pour les joueurs
void ui_plateau_set_players(UINode* plateau, GamePlayer* player1, GamePlayer* player2);
GamePlayer* ui_plateau_get_player1(UINode* plateau);
GamePlayer* ui_plateau_get_player2(UINode* plateau);

// Nettoyage des ressources
void ui_plateau_cleanup(UINode* plateau);

// 🆕 NOUVELLE FONCTION: Destruction complète du container
void ui_plateau_container_destroy(UINode* plateau_container);

// === MACROS POUR LES NOUVEAUX COMPOSANTS ===

#define UI_SIDEBAR(tree, id) ui_sidebar(tree, id)
#define UI_CNT_PLAYABLE(tree, id) ui_cnt_playable(tree, id)
#define UI_PLATEAU(tree, id) ui_plateau_container(tree, id)
#define UI_PLATEAU_SIZED(tree, id, w, h) ui_plateau_container_with_size(tree, id, w, h)

// === MACROS MANQUANTES ===

#define SET_POS(node, x, y) ui_set_position(node, x, y)
#define SET_SIZE(node, w, h) ui_set_size(node, w, h)
#define SET_BG(node, color) ui_set_background(node, color)
#define SET_BG_IMG(node, path) ui_set_background_image(node, path)
#define SET_BG_SIZE(node, size) ui_set_background_size(node, size)
#define SET_BG_REPEAT(node, repeat) ui_set_background_repeat(node, repeat)
#define CENTER(node) ui_center(node)
#define CENTER_X(node) ui_center_x(node)
#define CENTER_Y(node) ui_center_y(node)

#endif // UI_COMPONENTS_H