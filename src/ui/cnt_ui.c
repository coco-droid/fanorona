#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === COMPOSANT CONTAINER ===

UINode* ui_container(UITree* tree, const char* id) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    // Créer un div de base
    UINode* container = ui_div(tree, id);
    if (!container) {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create container div");
        return NULL;
    }
    
    // Style modal : fond noir transparent avec bordure orange
    atomic_set_background_color(container->element, 0, 0, 0, 180); // Noir transparent
    // 🔧 FIX: Utiliser atomic_set_border au lieu des fonctions séparées
    atomic_set_border(container->element, 2, 255, 165, 0, 255); // Bordure orange de 2px
    atomic_set_padding(container->element, 1, 1, 1, 1); // Padding interne de 1px
    
    // Configuration flexbox pour layout vertical centré
    ui_set_display_flex(container);
    FLEX_COLUMN(container);
    ui_set_justify_content(container, "center");
    ui_set_align_items(container, "center");
    ui_set_flex_gap(container, 20);
    
    // Log de création
    ui_log_event("UIComponent", "Create", id, "Container created with modal style (black overlay, orange border)");
    printf("📦 Container '%s' créé avec style modal\n", id ? id : "NoID");
    
    return container;
}

UINode* ui_container_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* container = ui_container(tree, id);
    if (container) {
        SET_SIZE(container, width, height);
        ui_log_event("UIComponent", "Style", id, "Container size set");
    }
    return container;
}

UINode* ui_container_centered(UITree* tree, const char* id, int width, int height) {
    UINode* container = ui_container_with_size(tree, id, width, height);
    if (container) {
        CENTER(container);
        ui_log_event("UIComponent", "Style", id, "Container centered");
    }
    return container;
}

void ui_container_add_header(UINode* container, const char* header_text) {
    if (!container || !header_text) {
        ui_log_event("UIComponent", "ContainerError", container ? container->id : "null", 
                    "Invalid parameters for header");
        return;
    }
    
    UINode* header = UI_TEXT(container->tree, "container-header", header_text);
    if (header) {
        ui_set_text_color(header, "rgb(255, 165, 0)"); // Orange pour contraste
        ui_set_text_size(header, 20);
        ui_set_text_align(header, "center");
        APPEND(container, header);
        
        ui_log_event("UIComponent", "ContainerHeader", container->id, "Header added to container");
    }
}

void ui_container_add_content(UINode* container, UINode* content) {
    if (!container || !content) {
        ui_log_event("UIComponent", "ContainerError", container ? container->id : "null", 
                    "Invalid parameters for content");
        return;
    }
    
    APPEND(container, content);
    ui_log_event("UIComponent", "ContainerContent", container->id, "Content added to container");
}

void ui_container_set_modal_style(UINode* container, bool is_modal) {
    if (!container) return;
    
    if (is_modal) {
        // Style modal complet : overlay sombre
        atomic_set_background_color(container->element, 0, 0, 0, 200);
        // 🔧 FIX: Utiliser atomic_set_border
        atomic_set_border(container->element, 3, 255, 165, 0, 255); // Bordure orange de 3px
        ui_set_z_index(container, 1000); // Au-dessus de tout
        
        ui_log_event("UIComponent", "ContainerStyle", container->id, "Modal style applied");
    } else {
        // Style normal
        atomic_set_background_color(container->element, 255, 255, 255, 230);
        // 🔧 FIX: Utiliser atomic_set_border
        atomic_set_border(container->element, 1, 128, 128, 128, 255); // Bordure grise de 1px
        
        ui_log_event("UIComponent", "ContainerStyle", container->id, "Normal style applied");
    }
}
