#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "ui_tree.h"

// === FONCTIONS DE CRÉATION DE COMPOSANTS ===

// Syntaxe simplifiée pour créer des composants
UINode* ui_div(UITree* tree, const char* id);
UINode* ui_text(UITree* tree, const char* id, const char* content);

// === FONCTIONS FLUIDES (CHAÎNABLES) ===

// Positionnement et taille
UINode* ui_set_position(UINode* node, int x, int y);
UINode* ui_set_size(UINode* node, int width, int height);
UINode* ui_set_z_index(UINode* node, int z_index);

// Style fluide
UINode* ui_set_background(UINode* node, const char* color);
UINode* ui_set_background_image(UINode* node, const char* image_path);
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

// Événements fluides
UINode* ui_on_click(UINode* node, void (*callback)(UINode*, void*));
UINode* ui_on_hover(UINode* node, void (*callback)(UINode*, void*));

// Hiérarchie fluide
UINode* ui_append_to(UINode* child, UINode* parent);
UINode* ui_append(UINode* parent, UINode* child);

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

// === MACROS POUR SYNTAXE SIMPLIFIÉE ===

#define UI_BUTTON(tree, id, label) ui_button(tree, id, label)
#define UI_DIV(tree, id) ui_div(tree, id)
#define UI_TEXT(tree, id, content) ui_text(tree, id, content)

#define SET_POS(node, x, y) ui_set_position(node, x, y)
#define SET_SIZE(node, w, h) ui_set_size(node, w, h)
#define SET_BG(node, color) ui_set_background(node, color)
#define SET_BG_IMG(node, path) ui_set_background_image(node, path)
#define CENTER(node) ui_center(node)
#define FLEX_ROW(node) ui_set_flex_direction(node, "row")
#define FLEX_COLUMN(node) ui_set_flex_direction(node, "column")

#define ON_CLICK(node, callback) ui_on_click(node, callback)
#define APPEND_TO(child, parent) ui_append_to(child, parent)
#define APPEND(parent, child) ui_append(parent, child)

#endif // UI_COMPONENTS_H