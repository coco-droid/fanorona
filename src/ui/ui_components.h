#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "ui_tree.h"
#include <SDL2/SDL_ttf.h>

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

#define SET_POS(node, x, y) ui_set_position(node, x, y)
#define SET_SIZE(node, w, h) ui_set_size(node, w, h)
#define SET_BG(node, color) ui_set_background(node, color)
#define SET_BG_IMG(node, path) ui_set_background_image(node, path)
#define SET_BG_SIZE(node, size) ui_set_background_size(node, size)
#define SET_BG_REPEAT(node, repeat) ui_set_background_repeat(node, repeat)
#define CENTER(node) ui_center(node)
#define FLEX_ROW(node) ui_set_flex_direction(node, "row")
#define FLEX_COLUMN(node) ui_set_flex_direction(node, "column")

#define ON_CLICK(node, callback) ui_on_click(node, callback)
#define APPEND_TO(child, parent) ui_append_to(child, parent)
#define APPEND(parent, child) ui_append(parent, child)

// Macro de debugging
#define DEBUG_TEXT(node) ui_debug_text_rendering(node, #node)

#endif // UI_COMPONENTS_H