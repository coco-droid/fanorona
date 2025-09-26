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

// 🔧 FIX: Utiliser les fonctions de ui_components.c au lieu de redéfinir
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
    atomic_set_background_color(container->element, 0, 0, 0, 180);
    atomic_set_border(container->element, 2, 255, 165, 0, 255);
    atomic_set_padding(container->element, 2, 2, 2, 2);
    atomic_set_overflow(container->element, OVERFLOW_HIDDEN);
    
    // Utiliser les fonctions de ui_components.c
    ui_container_add_default_logo(container);
    ui_container_add_default_subtitle(container);
    
    ui_log_event("UIComponent", "Create", id, "Container created with overflow:hidden and default content");
    
    return container;
}

// 🆕 NOUVELLE FONCTION: Ajouter le logo par défaut avec align-self

// 🆕 NOUVELLE FONCTION: Ajouter le sous-titre par défaut avec calcul correct

// 🆕 NOUVELLE FONCTION: Ajouter du contenu avec calcul correct
void ui_container_add_content(UINode* container, UINode* content) {
    if (!container || !content) {
        ui_log_event("UIComponent", "ContainerError", container ? container->id : "null", 
                    "Invalid parameters for content");
        return;
    }
    
    // 🔧 FIX MAJEUR: Calcul correct basé sur le content_rect
    // Sous-titre à Y=98 + hauteur texte≈20 + espacement=8 = 126px depuis le content_rect
    SET_POS(content, 0, 126);
    ALIGN_SELF_X(content);
    
    APPEND(container, content);
    ui_log_event("UIComponent", "ContainerContent", container->id, "Content positioned 8px after subtitle");
    
    printf("📦 Contenu positionné à 126px depuis l'intérieur (sous-titre + 8px)\n");
}

// === FONCTIONS EXISTANTES (compatibilité) ===

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
        ALIGN_SELF_BOTH(container); // 🆕 UTILISER align-self pour centrage
        ui_log_event("UIComponent", "Style", id, "Container centered with align-self");
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
        
        // 🆕 UTILISER align-self pour centrage
        ALIGN_SELF_X(header);
        APPEND(container, header);
        
        ui_log_event("UIComponent", "ContainerHeader", container->id, "Header added to container with align-self");
    }
}

void ui_container_set_modal_style(UINode* container, bool is_modal) {
    if (!container) return;
    
    if (is_modal) {
        // Style modal complet : overlay sombre
        atomic_set_background_color(container->element, 0, 0, 0, 200);
        atomic_set_border(container->element, 3, 255, 165, 0, 255); // Bordure orange de 3px
        atomic_set_padding(container->element, 2, 2, 2, 2); // 🔧 FIX: Maintenir padding 2px en mode modal
        ui_set_z_index(container, 1000); // Au-dessus de tout
        
        ui_log_event("UIComponent", "ContainerStyle", container->id, "Modal style applied with 2px padding");
    } else {
        // Style normal
        atomic_set_background_color(container->element, 255, 255, 255, 230);
        atomic_set_border(container->element, 1, 128, 128, 128, 255); // Bordure grise de 1px
        atomic_set_padding(container->element, 2, 2, 2, 2); // 🔧 FIX: Maintenir padding 2px en mode normal
        
        ui_log_event("UIComponent", "ContainerStyle", container->id, "Normal style applied with 2px padding");
    }
}
