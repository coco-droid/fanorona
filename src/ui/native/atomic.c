#define _POSIX_C_SOURCE 200809L
#include "atomic.h"
#include "../../utils/asset_manager.h"
#include "../../utils/log_console.h"  // 🆕 AJOUT: Include manquant pour log_console_write
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// === FONCTIONS STATIQUES (DÉCLARATIONS AVANT UTILISATION) ===

// Calculer le rectangle de destination pour l'image de fond selon background-size
// 🔧 FIX: Calculer le rectangle de destination pour background "cover"
static SDL_Rect calculate_background_dest_rect(AtomicElement* element, SDL_Texture* texture) {
    // 🔧 FIX: Utiliser render_rect au lieu des coordonnées brutes
    SDL_Rect element_rect = atomic_get_render_rect(element);
    
    if (!texture) {
        return element_rect;
    }
    
    int texture_width, texture_height;
    SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
    
    SDL_Rect dest_rect = element_rect;
    
    switch (element->style.background_size) {
        case BACKGROUND_SIZE_COVER: {
            float element_aspect = (float)element_rect.w / element_rect.h;
            float texture_aspect = (float)texture_width / texture_height;
            
            if (texture_aspect > element_aspect) {
                dest_rect.h = element_rect.h;
                dest_rect.w = (int)(element_rect.h * texture_aspect);
                dest_rect.x = element_rect.x - (dest_rect.w - element_rect.w) / 2;
                dest_rect.y = element_rect.y;
            } else {
                dest_rect.w = element_rect.w;
                dest_rect.h = (int)(element_rect.w / texture_aspect);
                dest_rect.x = element_rect.x;
                dest_rect.y = element_rect.y - (dest_rect.h - element_rect.h) / 2;
            }
            break;
        }
        case BACKGROUND_SIZE_CONTAIN: {
            float element_aspect = (float)element_rect.w / element_rect.h;
            float texture_aspect = (float)texture_width / texture_height;
            
            if (texture_aspect > element_aspect) {
                dest_rect.w = element_rect.w;
                dest_rect.h = (int)(element_rect.w / texture_aspect);
                dest_rect.x = element_rect.x;
                dest_rect.y = element_rect.y + (element_rect.h - dest_rect.h) / 2;
            } else {
                dest_rect.h = element_rect.h;
                dest_rect.w = (int)(element_rect.h * texture_aspect);
                dest_rect.x = element_rect.x + (element_rect.w - dest_rect.w) / 2;
                dest_rect.y = element_rect.y;
            }
            break;
        }
        case BACKGROUND_SIZE_STRETCH:
        default:
            dest_rect = element_rect;
            break;
    }
    
    return dest_rect;
}

// Callback interne pour l'event manager
static void atomic_event_callback(SDL_Event* event, void* user_data) {
    AtomicElement* element = (AtomicElement*)user_data;
    
    if (element) {
        // 🔧 LOG SEULEMENT LES ÉVÉNEMENTS IMPORTANTS
        if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP || 
            event->type == SDL_KEYDOWN) {
            char message[512];
            snprintf(message, sizeof(message), 
                    "[atomic.c] 🔗 atomic_event_callback for '%s' - event type %d", 
                    element->id ? element->id : "NoID", event->type);
            log_console_write("AtomicCallback", "CallbackBridge", "atomic.c", message);
        }
        
        atomic_handle_event(element, event);
    } else {
        log_console_write("AtomicCallback", "CallbackError", "atomic.c", 
                         "[atomic.c] atomic_event_callback called with NULL element!");
    }
}

// === DEBUGGING DU TEXTE ===

void atomic_debug_text_rendering(AtomicElement* element, const char* context) {
    if (!element) {
        // Silencieux - pas de logs pour les éléments NULL
        return;
    }
    
    // 🔧 LOGS RÉDUITS - Seulement les erreurs critiques
    if (!element->content.text) {
        // Pas de log pour les éléments sans texte - c'est normal
        return;
    }
    
    // Log seulement si il y a un vrai problème
    if (element->style.text.font_size <= 0) {
        printf("⚠️ [TEXT_DEBUG] [%s] Element '%s' has invalid font size: %d\n", 
               context ? context : "Unknown",
               element->id ? element->id : "NoID", 
               element->style.text.font_size);
    }
}

// Créer un nouvel élément atomique
AtomicElement* atomic_create(const char* id) {
    AtomicElement* element = (AtomicElement*)calloc(1, sizeof(AtomicElement));
    if (!element) {
        printf("Erreur: Impossible d'allouer la mémoire pour l'élément atomique\n");
        return NULL;
    }
    
    // Initialiser l'ID
    if (id) {
        element->id = strdup(id);
    }
    
    // Initialiser le style avec des valeurs par défaut
    element->style.position = POSITION_STATIC;
    element->style.x = 0;
    element->style.y = 0;
    element->style.width = 100;
    element->style.height = 50;
    element->style.display = DISPLAY_BLOCK;
    element->style.z_index = 0;
    element->style.visible = true;
    
    // PAS de couleur de fond par défaut (transparent)
    element->style.background_color = (SDL_Color){0, 0, 0, 0};
    
    // PAS de bordure par défaut
    element->style.border_color = (SDL_Color){0, 0, 0, 0};
    element->style.border_width = 0; // Bordure désactivée par défaut
    
    element->style.opacity = 255;
    
    // Propriétés de background CSS
    element->style.background_size = BACKGROUND_SIZE_COVER; // Cover par défaut
    element->style.background_repeat = BACKGROUND_REPEAT_NO_REPEAT;
    
    // Initialiser les propriétés de texte étendues
    element->style.text_x = 0;
    element->style.text_y = 0;
    element->style.font = NULL;
    element->style.font_size = 16;
    element->style.text.color = (SDL_Color){0, 0, 0, 255};
    element->style.text.align = TEXT_ALIGN_LEFT;
    element->style.text.bold = false;
    element->style.text.italic = false;
    
    // 🆕 AJOUT: Initialiser l'overflow par défaut
    element->style.overflow = OVERFLOW_VISIBLE; // Par défaut, pas de contrainte
    
    // Initialiser align-self
    element->style.alignment.align_self = ALIGN_SELF_AUTO; // 🆕 AJOUT
    
    // Initialiser le contenu
    element->content.children_capacity = 4;
    element->content.children = (AtomicElement**)calloc(element->content.children_capacity, sizeof(AtomicElement*));
    
    // Initialiser les gestionnaires d'événements
    memset(&element->events, 0, sizeof(AtomicEventHandlers));
    
    return element;
}

// Détruire un élément atomique
void atomic_destroy(AtomicElement* element) {
    if (!element) return;
    
    // Libérer l'ID et la classe
    free(element->id);
    free(element->class_name);
    
    // Libérer le texte
    free(element->content.text);
    
    // Libérer les ressources d'image de fond
    free(element->style.background_image_path);
    
    // Libérer les ressources de police
    free(element->style.text.font_path);
    
    // Détruire les enfants
    for (int i = 0; i < element->content.children_count; i++) {
        atomic_destroy(element->content.children[i]);
    }
    free(element->content.children);
    
    // Libérer l'élément
    free(element);
}

// === FONCTIONS DE STYLE ===

void atomic_set_position(AtomicElement* element, int x, int y) {
    if (!element) return;
    element->style.x = x;
    element->style.y = y;
}

void atomic_set_size(AtomicElement* element, int width, int height) {
    if (!element) return;
    element->style.width = width;
    element->style.height = height;
}

void atomic_set_margin(AtomicElement* element, int top, int right, int bottom, int left) {
    if (!element) return;
    element->style.margin.top = top;
    element->style.margin.right = right;
    element->style.margin.bottom = bottom;
    element->style.margin.left = left;
}

void atomic_set_padding(AtomicElement* element, int top, int right, int bottom, int left) {
    if (!element) return;
    element->style.padding.top = top;
    element->style.padding.right = right;
    element->style.padding.bottom = bottom;
    element->style.padding.left = left;
}

void atomic_set_background_color(AtomicElement* element, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!element) return;
    element->style.background_color = (SDL_Color){r, g, b, a};
}

void atomic_set_border(AtomicElement* element, int width, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!element) return;
    element->style.border_width = width;
    element->style.border_color = (SDL_Color){r, g, b, a};
}

void atomic_set_z_index(AtomicElement* element, int z_index) {
    if (!element) return;
    element->style.z_index = z_index;
}

void atomic_set_display(AtomicElement* element, DisplayType display) {
    if (!element) return;
    element->style.display = display;
}

void atomic_set_visibility(AtomicElement* element, bool visible) {
    if (!element) return;
    element->style.visible = visible;
}

void atomic_set_opacity(AtomicElement* element, Uint8 opacity) {
    if (!element) return;
    element->style.opacity = opacity;
}

// === FONCTIONS D'IMAGES DE FOND ===

void atomic_set_background_image(AtomicElement* element, SDL_Texture* texture) {
    if (!element) return;
    element->style.background_image = texture;
}

void atomic_set_background_image_path(AtomicElement* element, const char* path, SDL_Renderer* renderer) {
    if (!element || !path || !renderer) return;
    
    // Libérer l'ancien chemin
    free(element->style.background_image_path);
    element->style.background_image_path = strdup(path);
    
    // Charger la texture
    element->style.background_image = asset_load_texture(renderer, path);
}

// === FONCTIONS DE POSITIONNEMENT ET ALIGNEMENT ===

void atomic_set_alignment(AtomicElement* element, AlignType horizontal, AlignType vertical) {
    if (!element) return;
    element->style.alignment.horizontal = horizontal;
    element->style.alignment.vertical = vertical;
}

void atomic_set_auto_center(AtomicElement* element, bool center_x, bool center_y) {
    if (!element) return;
    element->style.alignment.auto_center_x = center_x;
    element->style.alignment.auto_center_y = center_y;
}

void atomic_center_in_parent(AtomicElement* element) {
    if (!element || !element->parent) return;
    
    AtomicElement* parent = element->parent;
    SDL_Rect parent_rect = atomic_get_content_rect(parent);
    
    int center_x = parent_rect.x + (parent_rect.w - element->style.width) / 2;
    int center_y = parent_rect.y + (parent_rect.h - element->style.height) / 2;
    
    atomic_set_position(element, center_x, center_y);
}

// === FONCTIONS FLEXBOX ===

void atomic_set_flex_direction(AtomicElement* element, FlexDirection direction) {
    if (!element) return;
    element->style.flex.direction = direction;
}

void atomic_set_justify_content(AtomicElement* element, JustifyType justify) {
    if (!element) return;
    element->style.flex.justify_content = justify;
}

void atomic_set_align_items(AtomicElement* element, AlignType align) {
    if (!element) return;
    element->style.flex.align_items = align;
}

void atomic_set_flex_wrap(AtomicElement* element, bool wrap) {
    if (!element) return;
    element->style.flex.wrap = wrap;
}

void atomic_set_flex_gap(AtomicElement* element, int gap) {
    if (!element) return;
    element->style.flex.gap = gap;
}

void atomic_apply_flex_layout(AtomicElement* element) {
    if (!element || element->style.display != DISPLAY_FLEX) return;
    
    SDL_Rect container_rect = atomic_get_content_rect(element);
    int children_count = element->content.children_count;
    
    if (children_count == 0) return;
    
    bool is_row = (element->style.flex.direction == FLEX_DIRECTION_ROW || 
                   element->style.flex.direction == FLEX_DIRECTION_ROW_REVERSE);
    
    // Calculer l'espace disponible
    int available_space = is_row ? container_rect.w : container_rect.h;
    int total_children_size = 0;
    int gaps = (children_count - 1) * element->style.flex.gap;
    
    // Calculer la taille totale des enfants
    for (int i = 0; i < children_count; i++) {
        AtomicElement* child = element->content.children[i];
        total_children_size += is_row ? child->style.width : child->style.height;
    }
    
    // Calculer l'espace libre
    int free_space = available_space - total_children_size - gaps;
    
    // Positionner les enfants selon justify_content
    int current_pos = container_rect.x;
    if (!is_row) current_pos = container_rect.y;
    
    switch (element->style.flex.justify_content) {
        case JUSTIFY_CENTER:
            current_pos += free_space / 2;
            break;
        case JUSTIFY_END:
            current_pos += free_space;
            break;
        case JUSTIFY_SPACE_BETWEEN:
            if (children_count > 1) {
                gaps = free_space / (children_count - 1);
            }
            break;
        case JUSTIFY_SPACE_AROUND:
            if (children_count > 0) {
                int space_around = free_space / children_count;
                current_pos += space_around / 2;
                gaps = space_around;
            }
            break;
        case JUSTIFY_SPACE_EVENLY:
            if (children_count > 0) {
                gaps = free_space / (children_count + 1);
                current_pos += gaps;
            }
            break;
        default: // JUSTIFY_START
            break;
    }
    
    // Positionner chaque enfant
    for (int i = 0; i < children_count; i++) {
        AtomicElement* child = element->content.children[i];
        
        if (is_row) {
            child->style.x = current_pos;
            
            // Alignement vertical
            switch (element->style.flex.align_items) {
                case ALIGN_CENTER:
                case ALIGN_MIDDLE:
                    child->style.y = container_rect.y + (container_rect.h - child->style.height) / 2;
                    break;
                case ALIGN_BOTTOM:
                    child->style.y = container_rect.y + container_rect.h - child->style.height;
                    break;
                default: // ALIGN_TOP
                    child->style.y = container_rect.y;
                    break;
            }
            
            current_pos += child->style.width + gaps;
        } else {
            child->style.y = current_pos;
            
            // Alignement horizontal
            switch (element->style.flex.align_items) {
                case ALIGN_CENTER:
                    child->style.x = container_rect.x + (container_rect.w - child->style.width) / 2;
                    break;
                case ALIGN_RIGHT:
                    child->style.x = container_rect.x + container_rect.w - child->style.width;
                    break;
                default: // ALIGN_LEFT
                    child->style.x = container_rect.x;
                    break;
            }
            
            current_pos += child->style.height + gaps;
        }
    }
}

// === FONCTIONS DE TEXTE ET POLICE ===

void atomic_set_font(AtomicElement* element, const char* font_path, int size) {
    if (!element || !font_path) return;
    
    free(element->style.text.font_path);
    element->style.text.font_path = strdup(font_path);
    element->style.text.font_size = size;
}

void atomic_set_font_ttf(AtomicElement* element, TTF_Font* font) {
    if (!element) return;
    element->style.text.ttf_font = font;
}

void atomic_set_text_color(AtomicElement* element, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!element) return;
    element->style.text.color = (SDL_Color){r, g, b, a};
}

void atomic_set_text_align(AtomicElement* element, TextAlign align) {
    if (!element) return;
    element->style.text.align = align;
}

void atomic_set_text_style(AtomicElement* element, bool bold, bool italic) {
    if (!element) return;
    element->style.text.bold = bold;
    element->style.text.italic = italic;
}

// === FONCTIONS DE CONTENU ===

void atomic_set_text(AtomicElement* element, const char* text) {
    if (!element) return;
    
    free(element->content.text);
    element->content.text = text ? strdup(text) : NULL;
}

void atomic_set_texture(AtomicElement* element, SDL_Texture* texture) {
    if (!element) return;
    element->content.texture = texture;
}

void atomic_add_child(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return;
    
    // Vérifier si on a besoin de redimensionner le tableau
    if (parent->content.children_count >= parent->content.children_capacity) {
        parent->content.children_capacity *= 2;
        parent->content.children = (AtomicElement**)realloc(parent->content.children, 
                                                           parent->content.children_capacity * sizeof(AtomicElement*));
    }
    
    // Ajouter l'enfant
    parent->content.children[parent->content.children_count] = child;
    parent->content.children_count++;
    child->parent = parent;
}

void atomic_remove_child(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return;
    
    for (int i = 0; i < parent->content.children_count; i++) {
        if (parent->content.children[i] == child) {
            // Décaler les éléments suivants
            for (int j = i; j < parent->content.children_count - 1; j++) {
                parent->content.children[j] = parent->content.children[j + 1];
            }
            parent->content.children_count--;
            child->parent = NULL;
            break;
        }
    }
}

// === FONCTIONS D'ÉVÉNEMENTS ===

void atomic_set_click_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_click = handler;
}

void atomic_set_hover_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_hover = handler;
}

// 🆕 IMPLÉMENTATION MANQUANTE: atomic_set_unhover_handler
void atomic_set_unhover_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_unhover = handler;
    
    #ifdef ATOMIC_DEBUG_LOGS
    if (handler) {
        printf("🔗 [ATOMIC] Unhover handler set for element '%s'\n", 
               element->id ? element->id : "NoID");
    } else {
        printf("🔗 [ATOMIC] Unhover handler removed for element '%s'\n", 
               element->id ? element->id : "NoID");
    }
    #endif
}

void atomic_set_focus_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_focus = handler;
}

// === FONCTIONS DE MISE À JOUR ===

void atomic_update(AtomicElement* element, float delta_time) {
    if (!element) return;
    
    // Appliquer le layout flexbox si nécessaire
    if (element->style.display == DISPLAY_FLEX) {
        atomic_apply_flex_layout(element);
    }
    
    // Appliquer le centrage automatique (ancien système)
    if (element->style.alignment.auto_center_x || element->style.alignment.auto_center_y) {
        if (element->parent) {
            SDL_Rect parent_rect = atomic_get_content_rect(element->parent);
            
            if (element->style.alignment.auto_center_x) {
                element->style.x = parent_rect.x + (parent_rect.w - element->style.width) / 2;
            }
            
            if (element->style.alignment.auto_center_y) {
                element->style.y = parent_rect.y + (parent_rect.h - element->style.height) / 2;
            }
        }
    }
    
    // 🆕 AJOUT: Appliquer align-self (nouveau système)
    atomic_apply_align_self(element);
    
    // 🆕 AJOUT: Appliquer les contraintes d'overflow APRÈS les positionnements
    atomic_apply_overflow_constraints(element);
    
    // Mise à jour personnalisée
    if (element->custom_update) {
        element->custom_update(element, delta_time);
    }
    
    // Mettre à jour les enfants
    for (int i = 0; i < element->content.children_count; i++) {
        atomic_update(element->content.children[i], delta_time);
    }
}

// === NOUVELLES IMPLÉMENTATIONS CORRECTES ===

void atomic_set_text_position(AtomicElement* element, int x, int y) {
    if (!element) return;
    element->style.text_x = x;
    element->style.text_y = y;
}

void atomic_set_text_font(AtomicElement* element, TTF_Font* font) {
    if (!element) return;
    element->style.font = font;
    element->style.text.ttf_font = font; // Maintenir la cohérence
}

void atomic_set_text_size(AtomicElement* element, int size) {
    if (!element) return;
    element->style.font_size = size;
    element->style.text.font_size = size; // Maintenir la cohérence
}

void atomic_set_align(AtomicElement* element, const char* horizontal, const char* vertical) {
    if (!element) return;
    
    // Stocker les valeurs d'alignement
    if (horizontal && strcmp(horizontal, "center") == 0) {
        // Centrer horizontalement
        element->style.x = (800 - element->style.width) / 2; // Approximation
    }
    if (vertical && strcmp(vertical, "middle") == 0) {
        // Centrer verticalement
        element->style.y = (600 - element->style.height) / 2; // Approximation
    }
}

void atomic_add_class(AtomicElement* element, const char* class_name) {
    if (!element || !class_name) return;
    
    // Implementation simplifiée - utiliser un champ texte dans element->class_name
    if (element->class_name) {
        free(element->class_name);
    }
    element->class_name = strdup(class_name);
    printf("🏷️ CSS Class '%s' ajoutée à l'élément\n", class_name);
}

bool atomic_has_class(AtomicElement* element, const char* class_name) {
    if (!element || !class_name || !element->class_name) return false;
    
    return strcmp(element->class_name, class_name) == 0;
}

// 🆕 NOUVELLES FONCTIONS pour align-self
void atomic_set_align_self(AtomicElement* element, AlignSelf align_self) {
    if (!element) return;
    element->style.alignment.align_self = align_self;
}

void atomic_set_align_self_center_x(AtomicElement* element) {
    if (!element) return;
    element->style.alignment.align_self = ALIGN_SELF_CENTER_X;
}

void atomic_set_align_self_center_y(AtomicElement* element) {
    if (!element) return;
    element->style.alignment.align_self = ALIGN_SELF_CENTER_Y;
}

void atomic_set_align_self_center_both(AtomicElement* element) {
    if (!element) return;
    element->style.alignment.align_self = ALIGN_SELF_CENTER_BOTH;
}

void atomic_apply_align_self(AtomicElement* element) {
    if (!element || !element->parent) return;
    
    SDL_Rect parent_rect = atomic_get_content_rect(element->parent);
    
    switch (element->style.alignment.align_self) {
        case ALIGN_SELF_CENTER_X:
            element->style.x = parent_rect.x + (parent_rect.w - element->style.width) / 2;
            break;
            
        case ALIGN_SELF_CENTER_Y:
            element->style.y = parent_rect.y + (parent_rect.h - element->style.height) / 2;
            break;
            
        case ALIGN_SELF_CENTER_BOTH:
            element->style.x = parent_rect.x + (parent_rect.w - element->style.width) / 2;
            element->style.y = parent_rect.y + (parent_rect.h - element->style.height) / 2;
            break;
            
        case ALIGN_SELF_AUTO:
        default:
            // Pas de centrage automatique
            break;
    }
}

// === VERSIONS STRING POUR COMPATIBILITÉ ===

void atomic_set_text_align_str(AtomicElement* element, const char* align) {
    if (!element || !align) return;
    
    if (strcmp(align, "left") == 0) {
        atomic_set_text_align(element, TEXT_ALIGN_LEFT);
    } else if (strcmp(align, "center") == 0) {
        atomic_set_text_align(element, TEXT_ALIGN_CENTER);
    } else if (strcmp(align, "right") == 0) {
        atomic_set_text_align(element, TEXT_ALIGN_RIGHT);
    }
}

void atomic_set_text_color_rgba(AtomicElement* element, int r, int g, int b, int a) {
    if (!element) return;
    atomic_set_text_color(element, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
}

void atomic_set_display_str(AtomicElement* element, const char* display) {
    if (!element || !display) return;
    
    if (strcmp(display, "block") == 0) {
        atomic_set_display(element, DISPLAY_BLOCK);
    } else if (strcmp(display, "none") == 0) {
        atomic_set_display(element, DISPLAY_NONE);
    } else if (strcmp(display, "flex") == 0) {
        atomic_set_display(element, DISPLAY_FLEX);
    }
}

void atomic_set_flex_direction_str(AtomicElement* element, const char* direction) {
    if (!element || !direction) return;
    
    if (strcmp(direction, "row") == 0) {
        atomic_set_flex_direction(element, FLEX_DIRECTION_ROW);
    } else if (strcmp(direction, "column") == 0) {
        atomic_set_flex_direction(element, FLEX_DIRECTION_COLUMN);
    }
}

void atomic_set_justify_content_str(AtomicElement* element, const char* justify) {
    if (!element || !justify) return;
    
    if (strcmp(justify, "start") == 0) {
        atomic_set_justify_content(element, JUSTIFY_START);
    } else if (strcmp(justify, "center") == 0) {
        atomic_set_justify_content(element, JUSTIFY_CENTER);
    } else if (strcmp(justify, "end") == 0) {
        atomic_set_justify_content(element, JUSTIFY_END);
    }
}

void atomic_set_align_items_str(AtomicElement* element, const char* align) {
    if (!element || !align) return;
    
    if (strcmp(align, "start") == 0) {
        atomic_set_align_items(element, ALIGN_TOP); // Utiliser ALIGN_TOP pour "start"
    } else if (strcmp(align, "center") == 0) {
        atomic_set_align_items(element, ALIGN_CENTER);
    } else if (strcmp(align, "end") == 0) {
        atomic_set_align_items(element, ALIGN_BOTTOM); // Utiliser ALIGN_BOTTOM pour "end"
    } else if (strcmp(align, "stretch") == 0) {
        atomic_set_align_items(element, ALIGN_STRETCH);
    }
}

// === NOUVELLES FONCTIONS POUR BACKGROUND CSS ===

void atomic_set_background_size(AtomicElement* element, BackgroundSize size) {
    if (!element) return;
    element->style.background_size = size;
    // 🔧 SUPPRESSION: Plus de logs verbeux
    // printf("🖼️ Background size défini : %d\n", size);
}

void atomic_set_background_repeat(AtomicElement* element, BackgroundRepeat repeat) {
    if (!element) return;
    element->style.background_repeat = repeat;
    // 🔧 SUPPRESSION: Plus de logs verbeux
    // printf("🔄 Background repeat défini : %d\n", repeat);
}

void atomic_set_background_size_str(AtomicElement* element, const char* size) {
    if (!element || !size) return;
    
    if (strcmp(size, "cover") == 0) {
        atomic_set_background_size(element, BACKGROUND_SIZE_COVER);
    } else if (strcmp(size, "contain") == 0) {
        atomic_set_background_size(element, BACKGROUND_SIZE_CONTAIN);
    } else if (strcmp(size, "stretch") == 0) {
        atomic_set_background_size(element, BACKGROUND_SIZE_STRETCH);
    } else if (strcmp(size, "auto") == 0) {
        atomic_set_background_size(element, BACKGROUND_SIZE_AUTO);
    }
}

void atomic_set_background_repeat_str(AtomicElement* element, const char* repeat) {
    if (!element || !repeat) return;
    
    if (strcmp(repeat, "no-repeat") == 0) {
        atomic_set_background_repeat(element, BACKGROUND_REPEAT_NO_REPEAT);
    } else if (strcmp(repeat, "repeat") == 0) {
        atomic_set_background_repeat(element, BACKGROUND_REPEAT_REPEAT);
    } else if (strcmp(repeat, "repeat-x") == 0) {
        atomic_set_background_repeat(element, BACKGROUND_REPEAT_REPEAT_X);
    } else if (strcmp(repeat, "repeat-y") == 0) {
        atomic_set_background_repeat(element, BACKGROUND_REPEAT_REPEAT_Y);
    }
}

// === FONCTIONS UTILITAIRES MANQUANTES ===

bool atomic_is_point_inside(AtomicElement* element, int x, int y) {
    if (!element || !element->style.visible) return false;
    
    SDL_Rect rect = atomic_get_render_rect(element);
    return (x >= rect.x && x < rect.x + rect.w && 
            y >= rect.y && y < rect.y + rect.h);
}

SDL_Rect atomic_get_render_rect(AtomicElement* element) {
    SDL_Rect rect = {0, 0, 0, 0};
    if (!element) return rect;
    
    rect.x = element->style.x + element->style.margin.left;
    rect.y = element->style.y + element->style.margin.top;
    rect.w = element->style.width;
    rect.h = element->style.height;
    
    return rect;
}

SDL_Rect atomic_get_content_rect(AtomicElement* element) {
    SDL_Rect rect = atomic_get_render_rect(element);
    if (!element) return rect;
    
    rect.x += element->style.padding.left;
    rect.y += element->style.padding.top;
    rect.w -= element->style.padding.left + element->style.padding.right;
    rect.h -= element->style.padding.top + element->style.padding.bottom;
    
    return rect;
}

// === FONCTIONS DE RENDU COMPLÈTES ===


// 🆕 NOUVELLE FONCTION: Obtenir la police par défaut
TTF_Font* atomic_get_default_font(void) {
    static TTF_Font* default_font = NULL;
    
    if (!default_font) {
        // 🔧 Essayer plusieurs polices système courantes
        const char* font_paths[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/System/Library/Fonts/Arial.ttf",                    // macOS
            "C:\\Windows\\Fonts\\arial.ttf",                      // Windows
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf"
        };
        
        for (int i = 0; i < 6; i++) {
            default_font = TTF_OpenFont(font_paths[i], 16);
            if (default_font) {
                printf("✅ Police par défaut chargée: %s\n", font_paths[i]);
                break;
            }
        }
        
        if (!default_font) {
            printf("❌ ERREUR: Aucune police système trouvée!\n");
            printf("   Polices testées:\n");
            for (int i = 0; i < 6; i++) {
                printf("   - %s\n", font_paths[i]);
            }
        }
    }
    
    return default_font;
}

void atomic_handle_event(AtomicElement* element, SDL_Event* event) {
    if (!element) return;
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            if (atomic_is_point_inside(element, event->button.x, event->button.y)) {
                element->is_pressed = true;
                
                // 🔧 LOG DÉTAILLÉ: Callback sur le point d'être appelé
                char message[512];
                snprintf(message, sizeof(message), 
                        "[atomic.c] 🎯 Mouse click detected in '%s' - calling click handler", 
                        element->id ? element->id : "NoID");
                log_console_write("AtomicElement", "ClickDetected", "atomic.c", message);
                
                if (element->events.on_click) {
                    log_console_write("AtomicElement", "CallbackExecuting", "atomic.c", 
                                     "[atomic.c] ✅ Executing click callback");
                    element->events.on_click(element, event);
                    log_console_write("AtomicElement", "CallbackDone", "atomic.c", 
                                     "[atomic.c] ✅ Click callback executed successfully");
                } else {
                    log_console_write("AtomicElement", "NoCallback", "atomic.c", 
                                     "[atomic.c] ❌ No click callback defined for this element");
                }
            }
            break;
            
        case SDL_MOUSEMOTION: {
            bool was_hovered = element->is_hovered;
            bool is_inside = atomic_is_point_inside(element, event->motion.x, event->motion.y);
            
            if (is_inside && !was_hovered) {
                // MOUSE ENTER - Trigger hover
                element->is_hovered = true;
                if (element->events.on_hover) {
                    element->events.on_hover(element, event);
                }
            } else if (!is_inside && was_hovered) {
                // MOUSE LEAVE - Trigger unhover
                element->is_hovered = false;
                if (element->events.on_unhover) {
                    element->events.on_unhover(element, event);
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP:
            if (element->is_pressed) {
                element->is_pressed = false;
            }
            break;
            
        // 🔧 SUPPRESSION MASSIVE des logs pour événements non critiques
        case SDL_WINDOWEVENT:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_TEXTINPUT:
        case SDL_MOUSEWHEEL:
            // Ces événements sont normaux, ne pas logger
            break;
            
        default:
            // 🔧 LOG SEULEMENT les types vraiment inconnus
            if (event->type != SDL_MOUSEMOTION && 
                event->type != SDL_WINDOWEVENT && 
                event->type != SDL_KEYDOWN && 
                event->type != SDL_KEYUP &&
                event->type != SDL_TEXTINPUT) {
                char message[256];
                snprintf(message, sizeof(message), 
                        "[atomic.c] Unknown event type %d for element '%s'", 
                        event->type, element->id ? element->id : "NoID");
                log_console_write("AtomicElement", "UnknownEvent", "atomic.c", message);
            }
            break;
    }
}

void atomic_register_with_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) {
        printf("❌ [ATOMIC_REGISTER] Element ou Manager NULL\n");
        return;
    }
    
    // 🔧 FIX PRINCIPAL: Utiliser les vraies coordonnées calculées
    SDL_Rect rect = atomic_get_render_rect(element);
    
    // 🆕 VÉRIFICATION: Afficher les coordonnées pour debugging
    char debug_message[512];
    snprintf(debug_message, sizeof(debug_message), 
            "[atomic.c] 🔧 Element '%s' - Style position: (%d,%d) size: %dx%d", 
            element->id ? element->id : "NoID",
            element->style.x, element->style.y, 
            element->style.width, element->style.height);
    log_console_write("AtomicRegister", "StyleDebug", "atomic.c", debug_message);
    
    snprintf(debug_message, sizeof(debug_message), 
            "[atomic.c] 🔧 Element '%s' - Calculated rect: (%d,%d) size: %dx%d z=%d", 
            element->id ? element->id : "NoID",
            rect.x, rect.y, rect.w, rect.h, element->style.z_index);
    log_console_write("AtomicRegister", "RectDebug", "atomic.c", debug_message);
    
    // 🚨 ALERTE si les coordonnées sont à (0,0)
    if (rect.x == 0 && rect.y == 0) {
        char warning[256];
        snprintf(warning, sizeof(warning), 
                "[atomic.c] ⚠️ WARNING: Element '%s' registered at (0,0) - this may cause hit detection issues!", 
                element->id ? element->id : "NoID");
        log_console_write("AtomicRegister", "PositionWarning", "atomic.c", warning);
    }
    
    event_manager_subscribe(manager, rect.x, rect.y, rect.w, rect.h,
                           element->style.z_index, element->style.visible,
                           atomic_event_callback, element);
    
    // 🔧 LOG: Confirmation simple
    char message[512];
    snprintf(message, sizeof(message), 
            "[atomic.c] ✅ Registration completed for '%s' at bounds(%d,%d,%dx%d)", 
            element->id ? element->id : "NoID", rect.x, rect.y, rect.w, rect.h);
    log_console_write("AtomicRegister", "RegistrationSuccess", "atomic.c", message);
}

void atomic_unregister_from_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    
    event_manager_unsubscribe(manager, atomic_event_callback, element);
}

// === FONCTIONS UTILITAIRES MANQUANTES (AJOUTÉES) ===

bool atomic_has_explicit_z_index(AtomicElement* element) {
    if (!element) return false;
    
    // Considérer qu'un z-index est explicite s'il n'est pas 0
    // (car par défaut on initialise à 0 dans atomic_create)
    return element->style.z_index != 0;
}

int atomic_get_z_index(AtomicElement* element) {
    if (!element) return 0;
    return element->style.z_index;
}

int atomic_get_width(AtomicElement* element) {
    if (!element) return 0;
    return element->style.width;
}

int atomic_get_height(AtomicElement* element) {
    if (!element) return 0;
    return element->style.height;
}

// 🆕 NOUVELLE FONCTION: Définir le type d'overflow
void atomic_set_overflow(AtomicElement* element, OverflowType overflow) {
    if (!element) return;
    element->style.overflow = overflow;
    
    // Appliquer immédiatement les contraintes si nécessaire
    if (overflow == OVERFLOW_HIDDEN) {
        atomic_apply_overflow_constraints(element);
    }
}

// 🆕 NOUVELLE FONCTION: Définir l'overflow avec string
void atomic_set_overflow_str(AtomicElement* element, const char* overflow) {
    if (!element || !overflow) return;
    
    if (strcmp(overflow, "visible") == 0) {
        atomic_set_overflow(element, OVERFLOW_VISIBLE);
    } else if (strcmp(overflow, "hidden") == 0) {
        atomic_set_overflow(element, OVERFLOW_HIDDEN);
    } else if (strcmp(overflow, "scroll") == 0) {
        atomic_set_overflow(element, OVERFLOW_SCROLL);
    } else if (strcmp(overflow, "auto") == 0) {
        atomic_set_overflow(element, OVERFLOW_AUTO);
    }
}

// 🆕 NOUVELLE FONCTION: Calculer la position contrainte d'un enfant
SDL_Rect atomic_constrain_child_position(AtomicElement* parent, AtomicElement* child, int desired_x, int desired_y) {
    if (!parent || !child) {
        return (SDL_Rect){desired_x, desired_y, child ? child->style.width : 0, child ? child->style.height : 0};
    }
    
    // Si le parent permet le débordement, retourner la position désirée
    if (parent->style.overflow == OVERFLOW_VISIBLE) {
        return (SDL_Rect){desired_x, desired_y, child->style.width, child->style.height};
    }
    
    // Obtenir la zone de contenu du parent (sans padding/bordure)
    SDL_Rect parent_content = atomic_get_content_rect(parent);
    
    // Calculer les positions contraintes
    int constrained_x = desired_x;
    int constrained_y = desired_y;
    int constrained_width = child->style.width;
    int constrained_height = child->style.height;
    
    // Contraindre horizontalement
    if (constrained_x < parent_content.x) {
        constrained_x = parent_content.x;
    } else if (constrained_x + constrained_width > parent_content.x + parent_content.w) {
        // Si l'enfant est trop large, le positionner au bord droit
        constrained_x = parent_content.x + parent_content.w - constrained_width;
        
        // Si même ainsi il dépasse à gauche, réduire la largeur (pour overflow HIDDEN)
        if (constrained_x < parent_content.x) {
            constrained_x = parent_content.x;
            constrained_width = parent_content.w;
        }
    }
    
    // Contraindre verticalement
    if (constrained_y < parent_content.y) {
        constrained_y = parent_content.y;
    } else if (constrained_y + constrained_height > parent_content.y + parent_content.h) {
        // Si l'enfant est trop haut, le positionner au bord bas
        constrained_y = parent_content.y + parent_content.h - constrained_height;
        
        // Si même ainsi il dépasse en haut, réduire la hauteur (pour overflow HIDDEN)
        if (constrained_y < parent_content.y) {
            constrained_y = parent_content.y;
            constrained_height = parent_content.h;
        }
    }
    
    return (SDL_Rect){constrained_x, constrained_y, constrained_width, constrained_height};
}

// 🆕 NOUVELLE FONCTION: Appliquer les contraintes d'overflow à tous les enfants
void atomic_apply_overflow_constraints(AtomicElement* parent) {
    if (!parent || parent->style.overflow == OVERFLOW_VISIBLE) return;
    
    for (int i = 0; i < parent->content.children_count; i++) {
        AtomicElement* child = parent->content.children[i];
        if (!child) continue;
        
        // Calculer la position contrainte
        SDL_Rect constrained = atomic_constrain_child_position(parent, child, child->style.x, child->style.y);
        
        // Appliquer les contraintes
        child->style.x = constrained.x;
        child->style.y = constrained.y;
        
        // Pour overflow HIDDEN, ajuster aussi la taille si nécessaire
        if (parent->style.overflow == OVERFLOW_HIDDEN) {
            child->style.width = constrained.w;
            child->style.height = constrained.h;
        }
        
        // Appliquer récursivement aux enfants
        atomic_apply_overflow_constraints(child);
    }
}

// 🆕 NOUVELLE FONCTION: Vérifier si un enfant déborde
bool atomic_is_child_overflowing(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return false;
    
    SDL_Rect parent_content = atomic_get_content_rect(parent);
    SDL_Rect child_rect = atomic_get_render_rect(child);
    
    // Vérifier les débordements
    bool overflow_left = child_rect.x < parent_content.x;
    bool overflow_right = (child_rect.x + child_rect.w) > (parent_content.x + parent_content.w);
    bool overflow_top = child_rect.y < parent_content.y;
    bool overflow_bottom = (child_rect.y + child_rect.h) > (parent_content.y + parent_content.h);
    
    return overflow_left || overflow_right || overflow_top || overflow_bottom;
}

// === FONCTIONS DE RENDU SANS CLIPPING ===

void atomic_render(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer || !element->style.visible || element->style.display == DISPLAY_NONE) {
        return;
    }
    
    // 🔧 FIX MAJEUR: Utiliser les rectangles calculés au lieu des coordonnées brutes
    SDL_Rect render_rect = atomic_get_render_rect(element);
    SDL_Rect content_rect = atomic_get_content_rect(element);
    
    // 🔧 FIX: Sauvegarder et restaurer l'état du renderer
    SDL_BlendMode old_blend_mode;
    SDL_GetRenderDrawBlendMode(renderer, &old_blend_mode);
    
    Uint8 old_r, old_g, old_b, old_a;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
    
    // Activer le blending pour les transparences
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Dessiner le background SEULEMENT si alpha > 0
    if (element->style.background_color.a > 0) {
        SDL_SetRenderDrawColor(renderer, 
                             element->style.background_color.r,
                             element->style.background_color.g,
                             element->style.background_color.b,
                             (Uint8)((element->style.background_color.a * element->style.opacity) / 255));
        SDL_RenderFillRect(renderer, &render_rect);
    }
    
    // Dessiner l'image de fond si présente avec support CSS
    if (element->style.background_image) {
        // 🔧 FIX: Gérer l'alpha de la texture
        Uint8 texture_alpha = element->style.opacity;
        SDL_SetTextureAlphaMod(element->style.background_image, texture_alpha);
        
        // 🔧 FIX: Utiliser le rectangle calculé pour le background
        SDL_Rect bg_dest = calculate_background_dest_rect(element, element->style.background_image);
        
        // Gérer background-repeat
        if (element->style.background_repeat == BACKGROUND_REPEAT_NO_REPEAT) {
            SDL_RenderCopy(renderer, element->style.background_image, NULL, &bg_dest);
        } else {
            SDL_RenderCopy(renderer, element->style.background_image, NULL, &bg_dest);
        }
        
        // 🔧 Restaurer l'alpha de la texture
        SDL_SetTextureAlphaMod(element->style.background_image, 255);
    }
    
    // Dessiner la bordure SEULEMENT si width > 0 et alpha > 0
    if (element->style.border_width > 0 && element->style.border_color.a > 0) {
        SDL_SetRenderDrawColor(renderer,
                             element->style.border_color.r,
                             element->style.border_color.g,
                             element->style.border_color.b,
                             (Uint8)((element->style.border_color.a * element->style.opacity) / 255));
        
        for (int i = 0; i < element->style.border_width; i++) {
            SDL_Rect border_rect = {
                render_rect.x - i,
                render_rect.y - i,
                render_rect.w + 2 * i,
                render_rect.h + 2 * i
            };
            SDL_RenderDrawRect(renderer, &border_rect);
        }
    }
    
    // Dessiner la texture si présente (pour les composants image)
    if (element->content.texture) {
        SDL_SetTextureAlphaMod(element->content.texture, element->style.opacity);
        SDL_RenderCopy(renderer, element->content.texture, NULL, &content_rect);
        SDL_SetTextureAlphaMod(element->content.texture, 255); // Restaurer
    }
    
    // 🔧 FIX PRINCIPAL: RENDU DU TEXTE avec coordonnées correctes
    if (element->content.text && strlen(element->content.text) > 0) {
        TTF_Font* font = element->style.font;
        if (!font) {
            font = atomic_get_default_font();
            if (!font) {
                printf("⚠️ No font available for text rendering of '%s'\n", 
                       element->id ? element->id : "NoID");
                goto skip_text_rendering;
            }
        }
        
        SDL_Color text_color = {
            element->style.text.color.r,
            element->style.text.color.g,
            element->style.text.color.b,
            (Uint8)((element->style.text.color.a * element->style.opacity) / 255)
        };
        
        SDL_Surface* text_surface = TTF_RenderText_Blended(font, element->content.text, text_color);
        if (!text_surface) {
            printf("⚠️ Failed to create text surface: %s\n", TTF_GetError());
            goto skip_text_rendering;
        }
        
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        if (!text_texture) {
            SDL_FreeSurface(text_surface);
            printf("⚠️ Failed to create text texture: %s\n", SDL_GetError());
            goto skip_text_rendering;
        }
        
        // 🔧 FIX: Calculer la position du texte dans le content_rect (qui respecte le padding)
        int text_width = text_surface->w;
        int text_height = text_surface->h;
        SDL_FreeSurface(text_surface);
        
        int text_x = content_rect.x;
        int text_y = content_rect.y + (content_rect.h - text_height) / 2;
        
        switch (element->style.text.align) {
            case TEXT_ALIGN_CENTER:
                text_x = content_rect.x + (content_rect.w - text_width) / 2;
                break;
            case TEXT_ALIGN_RIGHT:
                text_x = content_rect.x + content_rect.w - text_width;
                break;
            default: // TEXT_ALIGN_LEFT
                text_x = content_rect.x + 5;
                break;
        }
        
        SDL_Rect text_rect = { text_x, text_y, text_width, text_height };
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        
        SDL_DestroyTexture(text_texture);
    }
    
skip_text_rendering:
    
    // Rendu personnalisé
    if (element->custom_render) {
        element->custom_render(element, renderer);
    }
    
    // 🔧 FIX MAJEUR: Rendre les enfants en tenant compte du padding parent
    for (int i = 0; i < element->content.children_count; i++) {
        AtomicElement* child = element->content.children[i];
        if (!child) continue;
        
        // 🔧 FIX: Calculer la position de l'enfant relative au content_rect du parent
        // Si l'enfant utilise align-self, la position sera recalculée dans atomic_update
        atomic_render(child, renderer);
    }
    
    // 🔧 FIX: Restaurer l'état original du renderer
    SDL_SetRenderDrawBlendMode(renderer, old_blend_mode);
    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}