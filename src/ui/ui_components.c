#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/asset_manager.h"
#include "../utils/log_console.h"
#include "../window/window.h"  // FIX: Include pour WindowDimensions
#include "../sound/sound.h"    // 🆕 AJOUT: Pour les sons
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// === VARIABLES GLOBALES POUR LE DEBUGGING ===

static bool event_logging_enabled = false;
static TTF_Font* default_font = NULL;

// === FONCTIONS DE DEBUGGING AMÉLIORÉES ===

void ui_set_event_logging(bool enabled) {
    event_logging_enabled = enabled;
    printf("Logs d'evenements UI : %s\n", enabled ? "ACTIVES" : "DESACTIVES");
    
    // Synchroniser avec la console de logs
    if (enabled) {
        log_console_init();
        log_console_ui_event("UIComponent", "LoggingState", "system", "Event logging activated");
    } else {
        log_console_ui_event("UIComponent", "LoggingState", "system", "Event logging deactivated");
    }
}

// HITBOX VISUALIZATION WRAPPERS
void ui_set_hitbox_visualization(bool enabled) {
    // Appeler la fonction du système atomique
    extern void atomic_set_hitbox_visualization(bool enabled);
    atomic_set_hitbox_visualization(enabled);
    
    // Log UI
    ui_log_event("UIComponent", "HitboxVisualization", "system", 
                enabled ? "Hitbox visualization enabled (red transparent + blue border)" :
                         "Hitbox visualization disabled");
}

bool ui_is_hitbox_visualization_enabled(void) {
    extern bool atomic_is_hitbox_visualization_enabled(void);
    return atomic_is_hitbox_visualization_enabled();
}

bool ui_is_event_logging_enabled(void) {
    return event_logging_enabled;
}

void ui_log_event(const char* source, const char* event_type, const char* element_id, const char* message) {
    if (!event_logging_enabled) return;
    
    // SUPPRESSION: Plus d'affichage dans la console standard
    // printf("[EVENT] [%s] [%s] [%s] : %s\n", 
    //        source ? source : "Unknown",
    //        event_type ? event_type : "Unknown", 
    //        element_id ? element_id : "NoID",
    //        message ? message : "No message");
    
    // Envoi vers la console de logs séparée seulement
    log_console_ui_event(source, event_type, element_id, message);
}

// === FONCTIONS DE POLICE ===

TTF_Font* ui_get_default_font(void) {
    if (!default_font) {
        default_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
        if (!default_font) {
            printf("Impossible de charger la police par defaut\n");
        }
    }
    return default_font;
}

void ui_cleanup_fonts(void) {
    if (default_font) {
        TTF_CloseFont(default_font);
        default_font = NULL;
        ui_log_event("UIComponent", "Cleanup", "fonts", "Default font cleaned up");
    }
}

// === GESTION Z-INDEX IMPLICITE ===

static int calculate_z_index_recursive(UINode* node, int base_z_index) {
    if (!node) return base_z_index;
    
    // FIX: Vérification directe du z-index au lieu de la fonction manquante
    if (node->element->style.z_index == 0) { // z-index implicite
        atomic_set_z_index(node->element, base_z_index);
        ui_log_event("UIComponent", "ZIndexCalculation", node->id, 
                    "Implicit z-index calculated and assigned");
        base_z_index++;
    } else {
        ui_log_event("UIComponent", "ZIndexCalculation", node->id, 
                    "Explicit z-index kept");
    }
    
    // Traiter récursivement tous les enfants
    for (int i = 0; i < node->children_count; i++) {
        base_z_index = calculate_z_index_recursive(node->children[i], base_z_index);
    }
    
    return base_z_index;
}

void ui_calculate_implicit_z_index(UITree* tree) {
    if (!tree || !tree->root) {
        ui_log_event("UIComponent", "ZIndexCalculation", "tree", "Tree or root is NULL");
        return;
    }
    
    ui_log_event("UIComponent", "ZIndexCalculation", "tree", "Starting implicit z-index calculation");
    
    // Commencer le calcul récursif avec z-index de base = 1
    calculate_z_index_recursive(tree->root, 1);
    
    ui_log_event("UIComponent", "ZIndexCalculation", "tree", "Implicit z-index calculation completed");
}

void ui_node_set_implicit_z_index(UINode* node, int base_z_index) {
    if (!node) return;
    
    // FIX: Vérification directe
    if (node->element->style.z_index == 0) { // Pas de z-index explicite
        atomic_set_z_index(node->element, base_z_index);
        ui_log_event("UIComponent", "ZIndexCalculation", node->id, "Implicit z-index set");
    }
}

int ui_node_get_effective_z_index(UINode* node) {
    if (!node || !node->element) return 0;
    
    // FIX: Accès direct au style
    return node->element->style.z_index;
}

// === CORRECTIONS BOUTONS ===

void ui_button_set_background_image(UINode* button, const char* image_path) {
    if (!button || !image_path) {
        ui_log_event("UIComponent", "ButtonError", button ? button->id : "null", 
                    "Invalid parameters for background image");
        return;
    }
    
    ui_log_event("UIComponent", "ButtonStyle", button->id, "Setting background image");
    
    // Charger l'image via l'asset manager
    GameWindow* window = use_mini_window();
    if (!window) {
        ui_log_event("UIComponent", "ButtonError", button->id, "No active window for texture loading");
        return;
    }
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) {
        ui_log_event("UIComponent", "ButtonError", button->id, "No renderer available");
        return;
    }
    
    SDL_Texture* texture = asset_load_texture(renderer, image_path);
    if (texture) {
        atomic_set_background_image(button->element, texture);
        
        // FIX PRINCIPAL: Forcer le mode "cover" par défaut pour les boutons
        atomic_set_background_size_str(button->element, "cover");
        atomic_set_background_repeat_str(button->element, "no-repeat");
        
        // SUPPRESSION: Plus de logs verbeux pour le mode COVER
        // char message[256];
        // snprintf(message, sizeof(message), 
        //         "[ui_components.c] Background image loaded with COVER mode for button '%s'",
        //         button->id ? button->id : "NoID");
        // ui_log_event("UIComponent", "ButtonStyle", button->id, message);
        
        // LOG SIMPLE et silencieux
        // printf("✅ Button '%s' background image set to COVER mode\n", button->id ? button->id : "NoID");
        
        ui_log_event("UIComponent", "ButtonStyle", button->id, "Background image applied with cover mode");
    } else {
        ui_log_event("UIComponent", "ButtonError", button->id, "Failed to load background image");
    }
}

void ui_button_fix_text_rendering(UINode* button) {
    if (!button) return;
    
    // SUPPRESSION: Plus de logs verbeux
    // ui_log_event("UIComponent", "ButtonFix", button->id, "Applying text rendering fixes");
    
    // Correction silencieuse
    atomic_set_text_align_str(button->element, "center");
    atomic_set_text_color_rgba(button->element, 255, 255, 255, 255);
    atomic_set_background_color(button->element, 0, 0, 0, 0);
    atomic_set_visibility(button->element, true);
    atomic_set_display(button->element, DISPLAY_BLOCK);
    ui_button_calculate_text_position(button);
}

void ui_button_calculate_text_position(UINode* button) {
    if (!button) return;
    
    // FIX: Accès direct au style au lieu des fonctions manquantes
    int button_width = button->element->style.width;
    int button_height = button->element->style.height;
    
    int text_x = button_width / 2;
    int text_y = button_height / 2;
    
    atomic_set_text_position(button->element, text_x, text_y);
}

// === FONCTIONS DE CRÉATION D'ÉLÉMENTS ===

UINode* ui_div(UITree* tree, const char* id) {
    if (!tree) return NULL;
    
    UINode* node = ui_tree_create_node(tree, id, "div");
    // SUPPRESSION: Plus de logs de création
    return node;
}

UINode* ui_text(UITree* tree, const char* id, const char* content) {
    if (!tree) return NULL;
    
    UINode* node = ui_tree_create_node(tree, id, "text");
    if (node && content) {
        atomic_set_text(node->element, content); // 🆕 FIX: Set the text content
        ui_log_event("UIComponent", "Create", id, "Text element created with content");
    } else if (!node) {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create text node");
    }
    
    return node;
}

UINode* ui_image(UITree* tree, const char* id, SDL_Texture* texture) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    UINode* node = ui_tree_create_node(tree, id, "image");
    if (node && texture) {
        atomic_set_texture(node->element, texture);
        ui_log_event("UIComponent", "Create", id, "Image element created with texture");
    } else {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create image element");
    }
    
    return node;
}

static void internal_button_click_wrapper(void* element, SDL_Event* event) {
    // 1. Jouer le son
    sound_play_button_click();
    
    // 2. Récupérer et appeler le callback utilisateur
    AtomicElement* atomic = (AtomicElement*)element;
    void (*user_callback)(UINode*, void*) = (void (*)(UINode*, void*))atomic_get_custom_data(atomic, "ui_callback");
    
    if (user_callback) {
        // Note: On passe 'element' et 'event' pour mimer le comportement du cast précédent
        // Idéalement, on devrait passer le UINode si on pouvait le récupérer proprement
        user_callback((UINode*)element, (void*)event); 
    }
}

UINode* ui_button(UITree* tree, const char* id, const char* text, void (*onClick)(UINode* node, void* user_data), void* user_data) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    UINode* node = ui_tree_create_node(tree, id, "button");
    if (node && text) {
        atomic_set_size(node->element, 150, 40);
        
        if (node->element->style.width != 150 || node->element->style.height != 40) {
            printf("[ERROR] [BUTTON_CREATE] Size not set correctly for '%s': %dx%d\n",
                   id ? id : "NoID", node->element->style.width, node->element->style.height);
        } else {
            printf("[OK] [BUTTON_CREATE] Button '%s' size confirmed: %dx%d\n",
                   id ? id : "NoID", node->element->style.width, node->element->style.height);
        }
        
        atomic_set_text(node->element, text);
        
        printf("Button '%s' created with text: '%s'\n", id ? id : "NoID", text);
        
        if (onClick && tree->event_manager) {
            // 🆕 MODIFICATION: Utiliser le wrapper pour le son
            atomic_set_custom_data(node->element, "ui_callback", (void*)onClick);
            atomic_set_custom_data(node->element, "ui_user_data", user_data);
            
            atomic_set_click_handler(node->element, internal_button_click_wrapper);
            atomic_register_with_event_manager(node->element, tree->event_manager);
            printf("Button '%s' auto-registered with sound & click handler\n", id ? id : "NoID");
        }
    }
    
    (void)user_data;
    return node;
}

// FIX: Déplacer la fonction récursive AVANT son utilisation
static void ui_tree_register_node_recursive(UINode* node, UITree* tree) {
    if (!node || !tree) return;
    
    // Si le nœud a des handlers, l'enregistrer
    if (node->element) {
        bool has_handlers = (node->element->events.on_click != NULL ||
                           node->element->events.on_hover != NULL ||
                           node->element->events.on_unhover != NULL);
        
        if (has_handlers) {
            atomic_register_with_event_manager(node->element, tree->event_manager);
            printf("Node '%s' registered with EventManager\n", 
                   node->id ? node->id : "NoID");
        }
    }
    
    // Parcourir récursivement les enfants
    for (int i = 0; i < node->children_count; i++) {
        ui_tree_register_node_recursive(node->children[i], tree);
    }
}

// NOUVELLE FONCTION: Forcer le recalcul des positions avant enregistrement
void ui_tree_update_positions(UITree* tree) {
    if (!tree || !tree->root) {
        printf("Invalid tree for position update\n");
        return;
    }
    
    printf("Forcing position recalculation...\n");
    
    // Forcer une mise à jour complète de l'arbre UI
    ui_tree_update(tree, 0.0f);
    
    printf("Position recalculation completed\n");
}

// NOUVELLE FONCTION: Enregistrer tous les boutons d'un arbre (AVEC POSITIONS FORCÉES)
void ui_tree_register_all_events(UITree* tree) {
    if (!tree || !tree->event_manager) {
        printf("Invalid tree or no EventManager for registration\n");
        return;
    }
    
    printf("Registering all UI elements with EventManager...\n");
    
    // FIX: Forcer le recalcul des positions AVANT l'enregistrement
    ui_tree_update_positions(tree);
    
    // Parcourir récursivement tous les nœuds et enregistrer ceux qui ont des handlers
    if (tree->root) {
        ui_tree_register_node_recursive(tree->root, tree);
    }
    
    printf("All UI elements registered with EventManager\n");
}

// NOUVELLE FONCTION: Enregistrement manuel pour les boutons existants
void ui_button_register_events(UINode* button, UITree* tree) {
    if (!button || !tree || !tree->event_manager) {
        printf("Invalid parameters for ui_button_register_events\n");
        return;
    }
    
    // Vérifier si le bouton a des handlers définis
    bool has_click = (button->element && button->element->events.on_click != NULL);
    bool has_hover = (button->element && button->element->events.on_hover != NULL);
    bool has_unhover = (button->element && button->element->events.on_unhover != NULL);
    
    if (has_click || has_hover || has_unhover) {
        atomic_register_with_event_manager(button->element, tree->event_manager);
        
        char message[256];
        snprintf(message, sizeof(message), 
                "[ui_components.c] Button '%s' registered - handlers: click=%s hover=%s unhover=%s",
                button->id ? button->id : "NoID",
                has_click ? "YES" : "NO",
                has_hover ? "YES" : "NO", 
                has_unhover ? "YES" : "NO");
        log_console_write("UIComponent", "ButtonRegistration", "ui_components.c", message);
        
        printf("Button '%s' manually registered with EventManager\n", 
               button->id ? button->id : "NoID");
    } else {
        printf("Button '%s' has no event handlers to register\n", 
               button->id ? button->id : "NoID");
    }
}

// === FONCTIONS FLUIDES ===

UINode* ui_set_position(UINode* node, int x, int y) {
    if (node) {
        atomic_set_position(node->element, x, y);
        ui_log_event("UIComponent", "Style", node->id, "Position set");
    }
    return node;
}

UINode* ui_set_size(UINode* node, int width, int height) {
    if (node) {
        atomic_set_size(node->element, width, height);
        ui_log_event("UIComponent", "Style", node->id, "Size set");
    }
    return node;
}

UINode* ui_set_z_index(UINode* node, int z_index) {
    if (node) {
        atomic_set_z_index(node->element, z_index);
        ui_log_event("UIComponent", "Style", node->id, "Z-index set explicitly");
    }
    return node;
}

UINode* ui_set_background(UINode* node, const char* color) {
    if (node && color) {
        // Parser la couleur et l'appliquer - implementation simplifiée
        atomic_set_background_color(node->element, 128, 128, 128, 255);
        ui_log_event("UIComponent", "Style", node->id, "Background color set");
    }
    return node;
}

UINode* ui_set_background_image(UINode* node, const char* image_path) {
    if (node && image_path) {
        // Charger l'image et l'appliquer
        GameWindow* window = use_mini_window();
        if (window) {
            SDL_Renderer* renderer = window_get_renderer(window);
            if (renderer) {
                SDL_Texture* texture = asset_load_texture(renderer, image_path);
                if (texture) {
                    atomic_set_background_image(node->element, texture);
                    
                    // FIX: Définir cover par défaut pour TOUS les éléments
                    atomic_set_background_size_str(node->element, "cover");
                    atomic_set_background_repeat_str(node->element, "no-repeat");
                    
                    // SUPPRESSION: Plus de logs verbeux
                    // char message[256];
                    // snprintf(message, sizeof(message), 
                    //         "[ui_components.c] Background image set with COVER mode for '%s'",
                    //         node->id ? node->id : "NoID");
                    // ui_log_event("UIComponent", "Style", node->id, message);
                    
                    ui_log_event("UIComponent", "Style", node->id, "Background image set with cover mode");
                }
            }
        }
    }
    return node;
}

UINode* ui_set_background_size(UINode* node, const char* size) {
    if (node && size) {
        atomic_set_background_size_str(node->element, size);
        ui_log_event("UIComponent", "Style", node->id, "Background size set");
    }
    return node;
}

UINode* ui_set_background_repeat(UINode* node, const char* repeat) {
    if (node && repeat) {
        atomic_set_background_repeat_str(node->element, repeat);
        ui_log_event("UIComponent", "Style", node->id, "Background repeat set");
    }
    return node;
}

UINode* ui_set_color(UINode* node, const char* color) {
    return ui_set_text_color(node, color);
}

UINode* ui_set_text_color(UINode* node, const char* color) {
    if (node && color) {
        // Pour l'instant, implementation simple pour le blanc
        atomic_set_text_color_rgba(node->element, 255, 255, 255, 255);
        ui_log_event("UIComponent", "Style", node->id, "Text color set");
    }
    return node;
}

UINode* ui_center(UINode* node) {
    if (node) {
        // FIX: Obtenir les dimensions automatiquement depuis le window manager
        WindowDimensions dims = window_get_active_dimensions();
        
        if (node->element) {
            int element_width = node->element->style.width;
            int element_height = node->element->style.height;
            
            if (element_width > 0 && element_height > 0) {
                int center_x = (dims.width - element_width) / 2;
                int center_y = (dims.height - element_height) / 2;
                atomic_set_position(node->element, center_x, center_y);
                
                char message[128];
                snprintf(message, sizeof(message), "Element centered in %dx%d window", dims.width, dims.height);
                ui_log_event("UIComponent", "Style", node->id, message);
            }
        }
    }
    return node;
}

UINode* ui_center_x(UINode* node) {
    if (node) {
        // FIX: Obtenir les dimensions automatiquement
        WindowDimensions dims = window_get_active_dimensions();
        
        if (node->element) {
            int element_width = node->element->style.width;
            
            if (element_width > 0) {
                int center_x = (dims.width - element_width) / 2;
                int current_y = node->element->style.y;
                atomic_set_position(node->element, center_x, current_y);
                
                char message[128];
                snprintf(message, sizeof(message), "Element centered horizontally in %dpx width", dims.width);
                ui_log_event("UIComponent", "Style", node->id, message);
            }
        }
    }
    return node;
}

UINode* ui_center_y(UINode* node) {
    if (node) {
        // FIX: Obtenir les dimensions automatiquement
        WindowDimensions dims = window_get_active_dimensions();
        
        if (node->element) {
            int element_height = node->element->style.height;
            
            if (element_height > 0) {
                int center_y = (dims.height - element_height) / 2;
                int current_x = node->element->style.x;
                atomic_set_position(node->element, current_x, center_y);
                
                char message[128];
                snprintf(message, sizeof(message), "Element centered vertically in %dpx height", dims.height);
                ui_log_event("UIComponent", "Style", node->id, message);
            }
        }
    }
    return node;
}

UINode* ui_set_display_flex(UINode* node) {
    if (node) {
        atomic_set_display_str(node->element, "flex");
        ui_log_event("UIComponent", "Style", node->id, "Display set to flex");
    }
    return node;
}

UINode* ui_set_flex_direction(UINode* node, const char* direction) {
    if (node && direction) {
        atomic_set_flex_direction_str(node->element, direction);
        ui_log_event("UIComponent", "Style", node->id, "Flex direction set");
    }
    return node;
}

UINode* ui_set_justify_content(UINode* node, const char* justify) {
    if (node && justify) {
        atomic_set_justify_content_str(node->element, justify);
        ui_log_event("UIComponent", "Style", node->id, "Justify content set");
    }
    return node;
}

UINode* ui_set_align_items(UINode* node, const char* align) {
    if (node && align) {
        atomic_set_align_items_str(node->element, align);
        ui_log_event("UIComponent", "Style", node->id, "Align items set");
    }
    return node;
}

UINode* ui_set_flex_gap(UINode* node, int gap) {
    if (node) {
        atomic_set_flex_gap(node->element, gap);
        ui_log_event("UIComponent", "Style", node->id, "Flex gap set");
    }
    return node;
}

UINode* ui_on_click(UINode* node, void (*callback)(UINode*, void*)) {
    if (node && callback) {
        // Note: Incompatibilité de signature - pour l'instant juste logger
        ui_log_event("UIComponent", "Event", node->id, "Click handler registered");
    }
    return node;
}

UINode* ui_on_hover(UINode* node, void (*callback)(UINode*, void*)) {
    if (node && callback) {
        // Note: Incompatibilité de signature - pour l'instant juste logger
        ui_log_event("UIComponent", "Event", node->id, "Hover handler registered");
    }
    return node;
}

UINode* ui_append(UINode* parent, UINode* child) {
    if (parent && child) {
        ui_tree_append_child(parent, child);
        ui_log_event("UIComponent", "Hierarchy", child->id, "Element appended to parent");
    }
    return parent;
}

UINode* ui_append_to(UINode* child, UINode* parent) {
    if (parent && child) {
        ui_tree_append_child(parent, child);
        ui_log_event("UIComponent", "Hierarchy", child->id, "Element appended to parent");
    }
    return child;
}

// === DEBUGGING DU TEXTE ===

void ui_debug_text_rendering(UINode* node, const char* context) {
    if (!node || !node->element) {
        printf("🔍 [UI_TEXT_DEBUG] [%s] Node or element is NULL\n", context ? context : "Unknown");
        return;
    }
    
    atomic_debug_text_rendering(node->element, context);
}

// === FONCTIONS DE STYLE POUR BOUTONS ===

void ui_button_set_style(UINode* button, const char* bg_color, const char* text_color, const char* border_color) {
    if (!button) return;
    
    (void)bg_color; (void)text_color; (void)border_color; // Éviter les warnings
    ui_log_event("UIComponent", "ButtonStyle", button->id, "Button style applied");
}

void ui_button_set_hover_style(UINode* button, const char* hover_bg_color, const char* hover_text_color) {
    if (!button) return;
    
    (void)hover_bg_color; (void)hover_text_color; // Éviter les warnings
    ui_log_event("UIComponent", "ButtonStyle", button->id, "Button hover style applied");
}

void ui_button_set_pressed_style(UINode* button, const char* pressed_bg_color, const char* pressed_text_color) {
    if (!button) return;
    
    (void)pressed_bg_color; (void)pressed_text_color; // Éviter les warnings
    ui_log_event("UIComponent", "ButtonStyle", button->id, "Button pressed style applied");
}

// === NOUVELLES FONCTIONS POUR FEEDBACK VISUEL ===

// Structure pour stocker la taille originale du bouton
typedef struct {
    int original_width;
    int original_height;
    float current_scale;
    bool scale_initialized;
} ButtonScaleData;

// Obtenir ou créer les données de scale pour un bouton
static ButtonScaleData* get_button_scale_data(UINode* button) {
    if (!button || !button->element) return NULL;
    
    ButtonScaleData* scale_data = (ButtonScaleData*)atomic_get_custom_data(button->element, "scale_data");
    
    if (!scale_data) {
        // Créer les données de scale la première fois
        scale_data = (ButtonScaleData*)malloc(sizeof(ButtonScaleData));
        if (scale_data) {
            scale_data->original_width = button->element->style.width;
            scale_data->original_height = button->element->style.height;
            scale_data->current_scale = 1.0f;
            scale_data->scale_initialized = true;
            
            atomic_set_custom_data(button->element, "scale_data", scale_data);
            
            printf("Scale data initialized for button '%s': %dx%d\n", 
                   button->id ? button->id : "NoID",
                   scale_data->original_width, 
                   scale_data->original_height);
        }
    }
    
    return scale_data;
}

// Appliquer un facteur de scale à un bouton
void ui_button_set_scale(UINode* button, float scale_factor) {
    if (!button) return;
    
    ButtonScaleData* scale_data = get_button_scale_data(button);
    if (!scale_data) return;
    
    // Calculer les nouvelles dimensions
    int new_width = (int)(scale_data->original_width * scale_factor);
    int new_height = (int)(scale_data->original_height * scale_factor);
    
    // Appliquer la nouvelle taille
    atomic_set_size(button->element, new_width, new_height);
    scale_data->current_scale = scale_factor;
    
    ui_log_event("UIComponent", "ScaleEffect", button->id, 
                "Scale applied - new dimensions calculated from original size");
    
    printf("Button '%s' scaled to %.0f%% (%dx%d -> %dx%d)\n",
           button->id ? button->id : "NoID",
           scale_factor * 100.0f,
           scale_data->original_width, scale_data->original_height,
           new_width, new_height);
}

// Obtenir le scale actuel d'un bouton
float ui_button_get_current_scale(UINode* button) {
    if (!button) return 1.0f;
    
    ButtonScaleData* scale_data = get_button_scale_data(button);
    return scale_data ? scale_data->current_scale : 1.0f;
}

// Effet de scale hover (105%)
void ui_button_scale_hover(UINode* button) {
    ui_button_set_scale(button, 1.05f); // +5%
}

// Effet de scale pressed (97%)
void ui_button_scale_pressed(UINode* button) {
    ui_button_set_scale(button, 0.97f); // -3%
}

// Effet de scale normal (100%)
void ui_button_scale_normal(UINode* button) {
    ui_button_set_scale(button, 1.0f); // Taille originale
}

// MODIFICATION: Mettre à jour les fonctions existantes avec l'effet de scale

void ui_button_set_pressed_state(UINode* button, bool pressed) {
    if (!button) return;
    
    if (pressed) {
        // Appliquer l'effet de scale pressed
        ui_button_scale_pressed(button);
        
        // Ajouter un overlay sombre
        atomic_set_background_color(button->element, 0, 0, 0, 100);
        
        ui_log_event("UIComponent", "VisualState", button->id, "Button pressed state applied with scale effect");
    } else {
        // Retour à la taille normale
        ui_button_scale_normal(button);
        
        // Supprimer l'overlay
        atomic_set_background_color(button->element, 0, 0, 0, 0);
        
        ui_log_event("UIComponent", "VisualState", button->id, "Button normal state restored with scale effect");
    }
}

void ui_button_set_hover_state(UINode* button, bool hovered) {
    if (!button) return;
    
    if (hovered) {
        // Appliquer l'effet de scale hover
        ui_button_scale_hover(button);
        
        // Ajouter un overlay lumineux
        atomic_set_background_color(button->element, 255, 255, 255, 30);
        
        ui_log_event("UIComponent", "VisualState", button->id, "Button hover state applied with scale effect");
    } else {
        // Retour à la taille normale
        ui_button_scale_normal(button);
        
        // Supprimer l'overlay
        atomic_set_background_color(button->element, 0, 0, 0, 0);
        
        ui_log_event("UIComponent", "VisualState", button->id, "Button normal state restored from hover with scale effect");
    }
}

void ui_button_reset_visual_state(UINode* button) {
    if (!button) return;
    
    // Restaurer l'apparence par défaut avec taille normale
    ui_button_scale_normal(button);
    atomic_set_background_color(button->element, 0, 0, 0, 0); // Transparent
    atomic_set_text_color_rgba(button->element, 255, 255, 255, 255); // Blanc
    
    ui_log_event("UIComponent", "VisualState", button->id, "Button visual state reset to default with normal scale");
}

// MODIFICATION: Mettre à jour les styles prédéfinis avec scale

void ui_button_apply_success_style(UINode* button) {
    if (!button) return;
    
    ui_button_scale_hover(button); // Scale 105% pour effet positif
    atomic_set_background_color(button->element, 100, 200, 100, 200); // Vert translucide
    atomic_set_text_color_rgba(button->element, 255, 255, 255, 255);  // Blanc
    
    ui_log_event("UIComponent", "VisualStyle", button->id, "Success style applied (green) with hover scale effect");
}

void ui_button_apply_danger_style(UINode* button) {
    if (!button) return;
    
    ui_button_scale_pressed(button); // Scale 97% pour effet d'avertissement
    atomic_set_background_color(button->element, 220, 100, 100, 200); // Rouge translucide
    atomic_set_text_color_rgba(button->element, 255, 255, 255, 255);  // Blanc
    
    ui_log_event("UIComponent", "VisualStyle", button->id, "Danger style applied (red) with pressed scale effect");
}

void ui_button_apply_info_style(UINode* button) {
    if (!button) return;
    
    ui_button_scale_hover(button); // Scale 105% pour information
    atomic_set_background_color(button->element, 100, 150, 220, 200); // Bleu translucide
    atomic_set_text_color_rgba(button->element, 255, 255, 255, 255);  // Blanc
    
    ui_log_event("UIComponent", "VisualStyle", button->id, "Info style applied (blue) with hover scale effect");
}

void ui_button_apply_warning_style(UINode* button) {
    if (!button) return;
    
    ui_button_set_scale(button, 1.02f); // Scale 102% pour avertissement modéré
    atomic_set_background_color(button->element, 255, 180, 100, 200); // Orange translucide
    atomic_set_text_color_rgba(button->element, 0, 0, 0, 255);        // Noir pour contraste
    
    ui_log_event("UIComponent", "VisualStyle", button->id, "Warning style applied (orange) with moderate scale effect");
}

// === FONCTIONS ADDITIONNELLES ===

UINode* ui_text_with_font(UITree* tree, const char* id, const char* content, TTF_Font* font) {
    UINode* node = ui_text(tree, id, content);
    if (node && font) {
        atomic_set_text_font(node->element, font);
        ui_log_event("UIComponent", "Style", id, "Custom font applied to text");
    }
    return node;
}

void ui_set_text_font(UINode* node, TTF_Font* font) {
    if (node && font) {
        atomic_set_text_font(node->element, font);
        ui_log_event("UIComponent", "Style", node->id, "Font set");
    }
}

void ui_set_text_size(UINode* node, int size) {
    if (node) {
        atomic_set_text_size(node->element, size);
        ui_log_event("UIComponent", "Style", node->id, "Text size set");
    }
}

// FIX: Corriger le type de retour et l'implémentation
UINode* ui_set_font_size(UINode* node, int size) {
    if (node) {
        atomic_set_text_size(node->element, size);
        ui_log_event("UIComponent", "Style", node->id, "Font size set");
    }
    return node;  // FIX: Retourner le node pour permettre le chaînage
}

UINode* ui_add_class(UINode* node, const char* class_name) {
    if (node && class_name) {
        atomic_add_class(node->element, class_name);
        ui_log_event("UIComponent", "Style", node->id, "CSS class added");
    }
    return node;
}

UINode* ui_set_align(UINode* node, const char* horizontal, const char* vertical) {
    if (node) {
        atomic_set_align(node->element, horizontal, vertical);
        ui_log_event("UIComponent", "Style", node->id, "Alignment set");
    }
    return node;
}

UINode* ui_set_font(UINode* node, const char* font_path, int size) {
    if (node && font_path) {
        TTF_Font* font = TTF_OpenFont(font_path, size);
        if (font) {
            atomic_set_text_font(node->element, font);
            ui_log_event("UIComponent", "Style", node->id, "Custom font loaded and applied");
        } else {
            ui_log_event("UIComponent", "StyleError", node->id, "Failed to load custom font");
        }
    }
    return node;
}

UINode* ui_set_text_align(UINode* node, const char* align) {
    if (node && align) {
        atomic_set_text_align_str(node->element, align);
        ui_log_event("UIComponent", "Style", node->id, "Text alignment set");
    }
    return node;
}

UINode* ui_set_text_style(UINode* node, bool bold, bool italic) {
    if (node) {
        atomic_set_text_style(node->element, bold, italic);
        ui_log_event("UIComponent", "Style", node->id, "Text style set");
    }
    return node;
}

void ui_apply_style(UINode* node, const UIStyle* style) {
    if (!node || !style) return;
    
    // Appliquer tous les styles de la structure
    if (style->x >= 0 && style->y >= 0) {
        ui_set_position(node, style->x, style->y);
    }
    if (style->width > 0 && style->height > 0) {
        ui_set_size(node, style->width, style->height);
    }
    if (style->z_index != 0) {
        ui_set_z_index(node, style->z_index);
    }
    
    ui_log_event("UIComponent", "Style", node->id, "Complete style applied");
}

// NOUVELLES FONCTIONS pour align-self
UINode* ui_set_align_self(UINode* node, const char* align_self) {
    if (node && align_self) {
        if (strcmp(align_self, "center-x") == 0) {
            atomic_set_align_self_center_x(node->element);
        } else if (strcmp(align_self, "center-y") == 0) {
            atomic_set_align_self_center_y(node->element);
        } else if (strcmp(align_self, "center") == 0 || strcmp(align_self, "center-both") == 0) {
            atomic_set_align_self_center_both(node->element);
        } else if (strcmp(align_self, "auto") == 0) {
            atomic_set_align_self(node->element, ALIGN_SELF_AUTO);
        }
        ui_log_event("UIComponent", "Style", node->id, "Align-self set");
    }
    return node;
}

UINode* ui_set_align_self_center_x(UINode* node) {
    if (node) {
        atomic_set_align_self_center_x(node->element);
        ui_log_event("UIComponent", "Style", node->id, "Align-self center-x applied");
    }
    return node;
}

UINode* ui_set_align_self_center_y(UINode* node) {
    if (node) {
        atomic_set_align_self_center_y(node->element);
        ui_log_event("UIComponent", "Style", node->id, "Align-self center-y applied");
    }
    return node;
}

UINode* ui_set_align_self_center_both(UINode* node) {
    if (node) {
        atomic_set_align_self_center_both(node->element);
        ui_log_event("UIComponent", "Style", node->id, "Align-self center-both applied");
    }
    return node;
}

// NOUVELLES FONCTIONS pour la gestion de l'overflow

UINode* ui_set_overflow(UINode* node, const char* overflow) {
    if (node && overflow) {
        atomic_set_overflow_str(node->element, overflow);
        ui_log_event("UIComponent", "Style", node->id, "Overflow behavior set");
    }
    return node;
}

UINode* ui_set_overflow_visible(UINode* node) {
    if (node) {
        atomic_set_overflow(node->element, OVERFLOW_VISIBLE);
        ui_log_event("UIComponent", "Style", node->id, "Overflow set to visible - children can overflow");
    }
    return node;
}

UINode* ui_set_overflow_hidden(UINode* node) {
    if (node) {
        atomic_set_overflow(node->element, OVERFLOW_HIDDEN);
        ui_log_event("UIComponent", "Style", node->id, "Overflow set to hidden - children constrained within bounds");
    }
    return node;
}

UINode* ui_set_overflow_scroll(UINode* node) {
    if (node) {
        atomic_set_overflow(node->element, OVERFLOW_SCROLL);
        ui_log_event("UIComponent", "Style", node->id, "Overflow set to scroll (not yet implemented)");
    }
    return node;
}

UINode* ui_set_overflow_auto(UINode* node) {
    if (node) {
        atomic_set_overflow(node->element, OVERFLOW_AUTO);
        ui_log_event("UIComponent", "Style", node->id, "Overflow set to auto");
    }
    return node;
}

bool ui_is_child_overflowing(UINode* parent, UINode* child) {
    if (!parent || !child) return false;
    return atomic_is_child_overflowing(parent->element, child->element);
}

void ui_constrain_all_children(UINode* parent) {
    if (!parent) return;
    atomic_apply_overflow_constraints(parent->element);
    ui_log_event("UIComponent", "Layout", parent->id, "All children positions constrained to parent bounds");
}

// FIX: Ajouter les déclarations forward manquantes
void ui_container_add_default_logo(UINode* container);
void ui_container_add_default_subtitle(UINode* container);

// FIX: Implémenter les fonctions manquantes
void ui_container_add_default_logo(UINode* container) {
    if (!container) return;
    
    // Charger le logo Fanorona
    SDL_Texture* logo_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            logo_texture = asset_load_texture(renderer, "fanorona_text.png");
        }
    }
    
    UINode* logo = NULL;
    if (logo_texture) {
        logo = ui_image(container->tree, "container-default-logo", logo_texture);
        if (logo) {
            ui_set_size(logo, 300, 80);
            
            // AJUSTER la position pour le padding augmenté (15px au lieu de 2px = +13px)
            ui_set_position(logo, 0, 23); // 10px + 13px = 23px
            
            ui_set_align_self_center_x(logo);
            atomic_set_background_color(logo->element, 0, 0, 0, 0);
            ui_append(container, logo);
        }
    } else {
        // Fallback texte
        logo = ui_text(container->tree, "container-default-logo-text", "FANORONA");
        if (logo) {
            ui_set_text_color(logo, "rgb(255, 165, 0)");
            ui_set_text_size(logo, 24);
            ui_set_text_align(logo, "center");
            
            // AJUSTER la position pour le padding augmenté
            ui_set_position(logo, 0, 23); // 10px + 13px = 23px
            
            ui_set_align_self_center_x(logo);
            ui_append(container, logo);
        }
    }
    
    ui_log_event("UIComponent", "ContainerDefault", container->id, "Default logo positioned at 23px from content top (adjusted for 15px padding)");
}

void ui_container_add_default_subtitle(UINode* container) {
    if (!container) return;
    
    UINode* subtitle = ui_text(container->tree, "container-default-subtitle", "STRATEGIE ET TRADITION");
    if (subtitle) {
        ui_set_text_color(subtitle, "rgb(255, 255, 255)");
        ui_set_text_size(subtitle, 14);
        ui_set_text_align(subtitle, "center");
        ui_set_text_style(subtitle, false, true);
        
        // AJUSTER la position pour le padding augmenté
        ui_set_position(subtitle, 0, 111); // 98px + 13px = 111px
        
        ui_set_align_self_center_x(subtitle);
        
        // AUGMENTER la marge bottom pour plus d'espace
        atomic_set_margin(subtitle->element, 0, 0, 20, 0); // 20px margin-bottom
        
        ui_append(container, subtitle);
    }
    
    ui_log_event("UIComponent", "ContainerDefault", container->id, "Default subtitle positioned at 111px with 20px margin-bottom (adjusted for 15px padding)");
}

// === NOUVELLES FONCTIONS D'ANIMATION UI ===

UINode* ui_animate_fade_in(UINode* node, float duration) {
    if (!node) return node;
    
    Animation* anim = animation_fade_in(duration);
    ui_node_add_animation(node, anim);
    
    ui_log_event("UIComponent", "Animation", node->id, "Fade-in animation started");
    return node;
}

UINode* ui_animate_fade_out(UINode* node, float duration) {
    if (!node) return node;
    
    Animation* anim = animation_fade_out(duration);
    ui_node_add_animation(node, anim);
    
    ui_log_event("UIComponent", "Animation", node->id, "Fade-out animation started");
    return node;
}

UINode* ui_animate_slide_in_left(UINode* node, float duration, float distance) {
    if (!node) return node;
    
    Animation* anim = animation_slide_in_left(duration, distance);
    ui_node_add_animation(node, anim);
    
    ui_log_event("UIComponent", "Animation", node->id, "Slide-in-left animation started");
    return node;
}

UINode* ui_animate_slide_in_right(UINode* node, float duration, float distance) {
    if (!node) return node;
    
    Animation* anim = animation_slide_in_right(duration, distance);
    ui_node_add_animation(node, anim);
    
    ui_log_event("UIComponent", "Animation", node->id, "Slide-in-right animation started");
    return node;
}

UINode* ui_animate_slide_out_left(UINode* node, float duration, float distance) {
    if (!node) return node;
    
    Animation* anim = animation_slide_out_left(duration, distance);
    ui_node_add_animation(node, anim);
    
    ui_log_event("UIComponent", "Animation", node->id, "Slide-out-left animation started");
    return node;
}

UINode* ui_animate_shake_x(UINode* node, float duration, float intensity) {
    if (!node) return node;
    
    Animation* anim = animation_shake_x(duration, intensity);
    ui_node_add_animation(node, anim);
    
    ui_log_event("UIComponent", "Animation", node->id, "Shake animation started");
    return node;
}

UINode* ui_animate_pulse(UINode* node, float duration) {
    if (!node) return node;
    
    Animation* anim = animation_pulse(duration);
    ui_node_add_animation(node, anim);
    
    ui_log_event("UIComponent", "Animation", node->id, "Pulse animation started");
    return node;
}

UINode* ui_stop_animations(UINode* node) {
    if (!node) return node;
    
    ui_node_stop_animations(node);
    ui_log_event("UIComponent", "Animation", node->id, "All animations stopped");
    return node;
}

// 🆕 Helper récursif pour arrêter les animations
static void stop_animations_recursive(UINode* node) {
    if (!node) return;
    ui_stop_animations(node);
    for (int i = 0; i < node->children_count; i++) {
        stop_animations_recursive(node->children[i]);
    }
}

void ui_tree_stop_all_animations(UITree* tree) {
    if (tree && tree->root) {
        stop_animations_recursive(tree->root);
    }
}