#define _POSIX_C_SOURCE 200809L
#include "atomic.h"
#include "../../utils/asset_manager.h"
#include "../../utils/log_console.h"  // 🆕 AJOUT: Include manquant pour log_console_write
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// === SUPPRESSION DES FONCTIONS INUTILISÉES ===
// Supprimer calculate_background_dest_rect et atomic_event_callback car elles sont déjà dans optimum.c

// === IMPLÉMENTATION DES FONCTIONS MANQUANTES ===

// Gérer les événements d'un élément atomique
// 🆕 Signature conforme à EventManager: (SDL_Event*, void*)
void atomic_handle_event(SDL_Event* event, void* user_data) {
    if (!event || !user_data) return;
    AtomicElement* element = (AtomicElement*)user_data;
    if (!element) return;
    
    // Traiter les événements selon leur type
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            if (element->events.on_click) {
                element->events.on_click(element, event);
            }
            break;
            
        case SDL_MOUSEMOTION: {
            // Détecter le survol
            int mouse_x, mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);
            
            bool was_hovered = element->is_hovered;
            bool is_now_hovered = atomic_is_point_inside(element, mouse_x, mouse_y);
            
            if (!was_hovered && is_now_hovered) {
                // Entrée en survol
                element->is_hovered = true;
                if (element->events.on_hover) {
                    element->events.on_hover(element, event);
                }
            } else if (was_hovered && !is_now_hovered) {
                // Sortie de survol
                element->is_hovered = false;
                if (element->events.on_unhover) {
                    element->events.on_unhover(element, event);
                }
            }
            break;
        }
            
        default:
            // Autres événements
            break;
    }
}

// Enregistrer un élément avec l'EventManager
void atomic_register_with_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    
    SDL_Rect rect = atomic_get_render_rect(element);
    
    // Enregistrer avec l'EventManager — maintenant la signature est correcte
    event_manager_subscribe(manager, 
                          rect.x, rect.y, rect.w, rect.h, 
                          element->style.z_index, 
                          element->style.visible,
                          atomic_handle_event,
                          element);
}

// Désenregistrer un élément de l'EventManager
void atomic_unregister_from_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    
    event_manager_unsubscribe(manager, 
                             atomic_handle_event, 
                             element);
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

// === FONCTIONS DE RENDU ET MISE À JOUR ===

void atomic_render(AtomicElement* element, SDL_Renderer* renderer) {
    // 🔧 REDIRECTION: Le rendu est maintenant géré par le moteur Optimum
    // Cette fonction est conservée pour compatibilité mais redirige vers Optimum
    #include "optimum.h"
    optimum_render_element(element, renderer);
}

// === FONCTIONS DE GESTION DE L'OVERFLOW ===

void atomic_set_overflow(AtomicElement* element, OverflowType overflow) {
    if (!element) return;
    element->style.overflow = overflow;
}

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

bool atomic_is_child_overflowing(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return false;
    
    SDL_Rect parent_rect = atomic_get_content_rect(parent);
    SDL_Rect child_rect = atomic_get_render_rect(child);
    
    return (child_rect.x < parent_rect.x ||
            child_rect.y < parent_rect.y ||
            child_rect.x + child_rect.w > parent_rect.x + parent_rect.w ||
            child_rect.y + child_rect.h > parent_rect.y + parent_rect.h);
}

SDL_Rect atomic_constrain_child_position(AtomicElement* parent, AtomicElement* child, int desired_x, int desired_y) {
    if (!parent || !child) return (SDL_Rect){0, 0, 0, 0};
    
    SDL_Rect parent_rect = atomic_get_content_rect(parent);
    SDL_Rect constrained_rect = {desired_x, desired_y, child->style.width, child->style.height};
    
    // Contraindre la position pour rester dans les limites du parent
    if (constrained_rect.x < parent_rect.x) {
        constrained_rect.x = parent_rect.x;
    }
    if (constrained_rect.y < parent_rect.y) {
        constrained_rect.y = parent_rect.y;
    }
    if (constrained_rect.x + constrained_rect.w > parent_rect.x + parent_rect.w) {
        constrained_rect.x = parent_rect.x + parent_rect.w - constrained_rect.w;
    }
    if (constrained_rect.y + constrained_rect.h > parent_rect.y + parent_rect.h) {
        constrained_rect.y = parent_rect.y + parent_rect.h - constrained_rect.h;
    }
    
    return constrained_rect;
}

void atomic_apply_overflow_constraints(AtomicElement* element) {
    if (!element || element->style.overflow == OVERFLOW_VISIBLE) return;
    
    // Appliquer les contraintes aux enfants
    for (int i = 0; i < element->content.children_count; i++) {
        AtomicElement* child = element->content.children[i];
        if (!child) continue;
        
        if (atomic_is_child_overflowing(element, child)) {
            SDL_Rect constrained = atomic_constrain_child_position(element, child, 
                                                                  child->style.x, 
                                                                  child->style.y);
            child->style.x = constrained.x;
            child->style.y = constrained.y;
        }
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