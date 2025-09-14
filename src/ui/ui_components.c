#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include <stdlib.h>
#include <string.h>

// === CRÉATION DE COMPOSANTS ===


UINode* ui_div(UITree* tree, const char* id) {
    return ui_tree_create_node(tree, "div", id);
}

UINode* ui_text(UITree* tree, const char* id, const char* content) {
    UINode* node = ui_tree_create_node(tree, "text", id);
    if (node && content) {
        ui_node_set_text(node, content);
    }
    return node;
}

// === FONCTIONS FLUIDES ===

UINode* ui_set_position(UINode* node, int x, int y) {
    if (node) {
        atomic_set_position(node->element, x, y);
    }
    return node;
}

UINode* ui_set_size(UINode* node, int width, int height) {
    if (node) {
        atomic_set_size(node->element, width, height);
    }
    return node;
}

UINode* ui_set_z_index(UINode* node, int z_index) {
    if (node) {
        atomic_set_z_index(node->element, z_index);
    }
    return node;
}

UINode* ui_set_background(UINode* node, const char* color) {
    if (node && color) {
        ui_node_set_style(node, "background-color", color);
    }
    return node;
}

UINode* ui_set_background_image(UINode* node, const char* image_path) {
    if (node && image_path && node->tree && node->tree->event_manager) {
        // Pour l'instant, stocker le chemin - il faudra un renderer pour charger
        free(node->element->style.background_image_path);
        node->element->style.background_image_path = strdup(image_path);
    }
    return node;
}

UINode* ui_set_color(UINode* node, const char* color) {
    if (node && color) {
        ui_node_set_style(node, "color", color);
    }
    return node;
}

UINode* ui_add_class(UINode* node, const char* class_name) {
    if (node && class_name) {
        ui_node_add_class(node, class_name);
    }
    return node;
}

// === ALIGNEMENT ET POSITIONNEMENT ===

UINode* ui_set_align(UINode* node, const char* horizontal, const char* vertical) {
    if (!node) return node;
    
    AlignType h_align = ALIGN_LEFT, v_align = ALIGN_TOP;
    
    // Parser l'alignement horizontal
    if (horizontal) {
        if (strcmp(horizontal, "center") == 0) h_align = ALIGN_CENTER;
        else if (strcmp(horizontal, "right") == 0) h_align = ALIGN_RIGHT;
        else if (strcmp(horizontal, "left") == 0) h_align = ALIGN_LEFT;
    }
    
    // Parser l'alignement vertical
    if (vertical) {
        if (strcmp(vertical, "middle") == 0 || strcmp(vertical, "center") == 0) v_align = ALIGN_MIDDLE;
        else if (strcmp(vertical, "bottom") == 0) v_align = ALIGN_BOTTOM;
        else if (strcmp(vertical, "top") == 0) v_align = ALIGN_TOP;
    }
    
    atomic_set_alignment(node->element, h_align, v_align);
    return node;
}

UINode* ui_center(UINode* node) {
    if (node) {
        atomic_set_auto_center(node->element, true, true);
    }
    return node;
}

UINode* ui_center_x(UINode* node) {
    if (node) {
        atomic_set_auto_center(node->element, true, false);
    }
    return node;
}

UINode* ui_center_y(UINode* node) {
    if (node) {
        atomic_set_auto_center(node->element, false, true);
    }
    return node;
}

// === FLEXBOX ===

UINode* ui_set_flex_direction(UINode* node, const char* direction) {
    if (!node || !direction) return node;
    
    FlexDirection dir = FLEX_DIRECTION_ROW;
    if (strcmp(direction, "column") == 0) dir = FLEX_DIRECTION_COLUMN;
    else if (strcmp(direction, "row-reverse") == 0) dir = FLEX_DIRECTION_ROW_REVERSE;
    else if (strcmp(direction, "column-reverse") == 0) dir = FLEX_DIRECTION_COLUMN_REVERSE;
    
    atomic_set_flex_direction(node->element, dir);
    return node;
}

UINode* ui_set_justify_content(UINode* node, const char* justify) {
    if (!node || !justify) return node;
    
    JustifyType j = JUSTIFY_START;
    if (strcmp(justify, "center") == 0) j = JUSTIFY_CENTER;
    else if (strcmp(justify, "end") == 0 || strcmp(justify, "flex-end") == 0) j = JUSTIFY_END;
    else if (strcmp(justify, "space-between") == 0) j = JUSTIFY_SPACE_BETWEEN;
    else if (strcmp(justify, "space-around") == 0) j = JUSTIFY_SPACE_AROUND;
    else if (strcmp(justify, "space-evenly") == 0) j = JUSTIFY_SPACE_EVENLY;
    
    atomic_set_justify_content(node->element, j);
    return node;
}

UINode* ui_set_align_items(UINode* node, const char* align) {
    if (!node || !align) return node;
    
    AlignType a = ALIGN_TOP;
    if (strcmp(align, "center") == 0) a = ALIGN_CENTER;
    else if (strcmp(align, "end") == 0 || strcmp(align, "flex-end") == 0) a = ALIGN_BOTTOM;
    else if (strcmp(align, "stretch") == 0) a = ALIGN_STRETCH;
    
    atomic_set_align_items(node->element, a);
    return node;
}

UINode* ui_set_flex_gap(UINode* node, int gap) {
    if (node) {
        atomic_set_flex_gap(node->element, gap);
    }
    return node;
}

UINode* ui_set_display_flex(UINode* node) {
    if (node) {
        atomic_set_display(node->element, DISPLAY_FLEX);
    }
    return node;
}

// === TEXTE ET POLICES ===

UINode* ui_set_font(UINode* node, const char* font_path, int size) {
    if (node && font_path) {
        atomic_set_font(node->element, font_path, size);
    }
    return node;
}

UINode* ui_set_text_color(UINode* node, const char* color) {
    if (node && color) {
        // Parser la couleur (format simple rgb(r,g,b))
        int r, g, b;
        if (sscanf(color, "rgb(%d,%d,%d)", &r, &g, &b) == 3) {
            atomic_set_text_color(node->element, r, g, b, 255);
        }
    }
    return node;
}

UINode* ui_set_text_align(UINode* node, const char* align) {
    if (!node || !align) return node;
    
    TextAlign a = TEXT_ALIGN_LEFT;
    if (strcmp(align, "center") == 0) a = TEXT_ALIGN_CENTER;
    else if (strcmp(align, "right") == 0) a = TEXT_ALIGN_RIGHT;
    else if (strcmp(align, "justify") == 0) a = TEXT_ALIGN_JUSTIFY;
    
    atomic_set_text_align(node->element, a);
    return node;
}

UINode* ui_set_text_style(UINode* node, bool bold, bool italic) {
    if (node) {
        atomic_set_text_style(node->element, bold, italic);
    }
    return node;
}

UINode* ui_on_click(UINode* node, void (*callback)(UINode*, void*)) {
    if (node && callback) {
        ui_node_add_event_listener(node, "click", callback, NULL);
    }
    return node;
}

UINode* ui_on_hover(UINode* node, void (*callback)(UINode*, void*)) {
    if (node && callback) {
        ui_node_add_event_listener(node, "hover", callback, NULL);
    }
    return node;
}

UINode* ui_append_to(UINode* child, UINode* parent) {
    if (child && parent) {
        ui_tree_append_child(parent, child);
    }
    return child;
}

UINode* ui_append(UINode* parent, UINode* child) {
    if (parent && child) {
        ui_tree_append_child(parent, child);
    }
    return parent;
}

// === STYLE CSS-LIKE ===

void ui_apply_style(UINode* node, const UIStyle* style) {
    if (!node || !style) return;
    
    if (style->x >= 0 && style->y >= 0) {
        ui_set_position(node, style->x, style->y);
    }
    
    if (style->width > 0 && style->height > 0) {
        ui_set_size(node, style->width, style->height);
    }
    
    if (style->z_index != 0) {
        ui_set_z_index(node, style->z_index);
    }
    
    if (style->background_color) {
        ui_set_background(node, style->background_color);
    }
    
    if (style->background_image) {
        ui_set_background_image(node, style->background_image);
    }
    
    if (style->color) {
        ui_set_text_color(node, style->color);
    }
    
    if (style->display) {
        ui_node_set_style(node, "display", style->display);
    }
    
    if (style->align_horizontal || style->align_vertical) {
        ui_set_align(node, style->align_horizontal, style->align_vertical);
    }
    
    if (style->flex_direction) {
        ui_set_flex_direction(node, style->flex_direction);
    }
    
    if (style->justify_content) {
        ui_set_justify_content(node, style->justify_content);
    }
    
    if (style->align_items) {
        ui_set_align_items(node, style->align_items);
    }
    
    if (style->flex_gap > 0) {
        ui_set_flex_gap(node, style->flex_gap);
    }
    
    if (style->font_path && style->font_size > 0) {
        ui_set_font(node, style->font_path, style->font_size);
    }
    
    if (style->text_align) {
        ui_set_text_align(node, style->text_align);
    }
    
    if (style->text_bold || style->text_italic) {
        ui_set_text_style(node, style->text_bold, style->text_italic);
    }
}