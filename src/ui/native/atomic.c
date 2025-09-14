#define _POSIX_C_SOURCE 200809L
#include "atomic.h"
#include "../../utils/asset_manager.h"
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// Callback interne pour l'event manager
static void atomic_event_callback(SDL_Event* event, void* user_data) {
    AtomicElement* element = (AtomicElement*)user_data;
    if (element) {
        atomic_handle_event(element, event);
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

void atomic_set_focus_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_focus = handler;
}

// === FONCTIONS UTILITAIRES ===

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

// === FONCTIONS DE RENDU (VERSION UNIQUE ET COMPLÈTE) ===

void atomic_render(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer || !element->style.visible || element->style.display == DISPLAY_NONE) {
        return;
    }
    
    SDL_Rect render_rect = atomic_get_render_rect(element);
    SDL_Rect content_rect = atomic_get_content_rect(element);
    
    // Sauvegarder le blend mode actuel
    SDL_BlendMode old_blend_mode;
    SDL_GetRenderDrawBlendMode(renderer, &old_blend_mode);
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
        SDL_SetTextureAlphaMod(element->style.background_image, element->style.opacity);
        
        // Calculer le rectangle de destination selon background-size
        SDL_Rect bg_dest = calculate_background_dest_rect(element, element->style.background_image);
        
        // Gérer background-repeat
        if (element->style.background_repeat == BACKGROUND_REPEAT_NO_REPEAT) {
            SDL_RenderCopy(renderer, element->style.background_image, NULL, &bg_dest);
        } else {
            // TODO: Implémenter repeat, repeat-x, repeat-y
            SDL_RenderCopy(renderer, element->style.background_image, NULL, &bg_dest);
        }
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
    }
    
    // RENDU DU TEXTE AMÉLIORÉ AVEC DEBUGGING
    if (element->content.text) {
        atomic_debug_text_rendering(element, "RENDER");
        
        SDL_SetRenderDrawColor(renderer, 
                             element->style.text.color.r,
                             element->style.text.color.g,
                             element->style.text.color.b,
                             (Uint8)((element->style.text.color.a * element->style.opacity) / 255));
        
        // Calculer la position du texte selon l'alignement
        int text_width = (int)strlen(element->content.text) * 8; // Approximation
        int text_height = element->style.text.font_size;
        int text_x = content_rect.x;
        int text_y = content_rect.y + (content_rect.h - text_height) / 2;
        
        switch (element->style.text.align) {
            case TEXT_ALIGN_CENTER:
                text_x = content_rect.x + (content_rect.w - text_width) / 2;
                printf("🎯 [TEXT_DEBUG] Center alignment: text_x = %d (content_rect.x=%d, w=%d, text_width=%d)\n", 
                       text_x, content_rect.x, content_rect.w, text_width);
                break;
            case TEXT_ALIGN_RIGHT:
                text_x = content_rect.x + content_rect.w - text_width;
                break;
            default: // TEXT_ALIGN_LEFT
                text_x = content_rect.x + 5;
                break;
        }
        
        printf("📝 [TEXT_DEBUG] Final text position: (%d, %d), size: %dx%d\n", 
               text_x, text_y, text_width, text_height);
        
        // Dessiner des rectangles pour représenter le texte (mode debug)
        for (int i = 0; i < (int)strlen(element->content.text); i++) {
            SDL_Rect letter_rect = {
                text_x + i * 8,
                text_y,
                6,
                text_height
            };
            SDL_RenderFillRect(renderer, &letter_rect);
            printf("🔤 [TEXT_DEBUG] Letter %d: rect (%d,%d,%d,%d)\n", 
                   i, letter_rect.x, letter_rect.y, letter_rect.w, letter_rect.h);
        }
        
        printf("✅ [TEXT_DEBUG] Text '%s' rendered with %d letters\n", 
               element->content.text, (int)strlen(element->content.text));
    }
    
    // Rendu personnalisé
    if (element->custom_render) {
        element->custom_render(element, renderer);
    }
    
    // Rendre les enfants
    for (int i = 0; i < element->content.children_count; i++) {
        atomic_render(element->content.children[i], renderer);
    }
    
    // Restaurer le blend mode
    SDL_SetRenderDrawBlendMode(renderer, old_blend_mode);
}

void atomic_update(AtomicElement* element, float delta_time) {
    if (!element) return;
    
    // Appliquer le layout flexbox si nécessaire
    if (element->style.display == DISPLAY_FLEX) {
        atomic_apply_flex_layout(element);
    }
    
    // Appliquer le centrage automatique
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
    
    // Mise à jour personnalisée
    if (element->custom_update) {
        element->custom_update(element, delta_time);
    }
    
    // Mettre à jour les enfants
    for (int i = 0; i < element->content.children_count; i++) {
        atomic_update(element->content.children[i], delta_time);
    }
}

// === NOUVELLES IMPLÉMENTATIONS ===

bool atomic_has_explicit_z_index(AtomicElement* element) {
    if (!element) return false;
    // Si z_index != 0, on considère qu'il a été défini explicitement
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
    
    // Stocker les valeurs d'alignement comme chaînes dans un membre étendu
    // Pour l'instant, utiliser les valeurs existantes
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
    
    // Implementation simplifiée - utiliser un champ texte dans element->text
    // Pour une vraie implémentation, il faudrait étendre la structure
    printf("🏷️ CSS Class '%s' ajoutée à l'élément\n", class_name);
}

bool atomic_has_class(AtomicElement* element, const char* class_name) {
    if (!element || !class_name) return false;
    
    // Implementation simplifiée
    return false;
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

// === FONCTIONS D'ÉVÉNEMENTS ===

void atomic_handle_event(AtomicElement* element, SDL_Event* event) {
    if (!element || !event) return;
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (atomic_is_point_inside(element, event->button.x, event->button.y)) {
                    element->is_pressed = true;
                    if (element->events.on_click) {
                        element->events.on_click(element, event);
                    }
                }
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT) {
                element->is_pressed = false;
            }
            break;
            
        case SDL_MOUSEMOTION: {
            bool was_hovered = element->is_hovered;
            element->is_hovered = atomic_is_point_inside(element, event->motion.x, event->motion.y);
            
            if (element->is_hovered && !was_hovered) {
                if (element->events.on_hover) {
                    element->events.on_hover(element, event);
                }
            } else if (!element->is_hovered && was_hovered) {
                if (element->events.on_blur) {
                    element->events.on_blur(element, event);
                }
            }
            break;
        }
        
        case SDL_KEYDOWN:
            if (element->is_focused && element->events.on_key_down) {
                element->events.on_key_down(element, event);
            }
            break;
            
        case SDL_KEYUP:
            if (element->is_focused && element->events.on_key_up) {
                element->events.on_key_up(element, event);
            }
            break;
    }
}

void atomic_register_with_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    
    SDL_Rect rect = atomic_get_render_rect(element);
    event_manager_subscribe(manager, rect.x, rect.y, rect.w, rect.h,
                           element->style.z_index, element->style.visible,
                           atomic_event_callback, element);
}

void atomic_unregister_from_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    
    event_manager_unsubscribe(manager, atomic_event_callback, element);
}

// === FONCTIONS STATIQUES (DÉCLARATIONS AVANT UTILISATION) ===

// Calculer le rectangle de destination pour l'image de fond selon background-size
static SDL_Rect calculate_background_dest_rect(AtomicElement* element, SDL_Texture* texture) {
    SDL_Rect element_rect = atomic_get_render_rect(element);
    SDL_Rect dest_rect = element_rect;
    
    if (!texture) return dest_rect;
    
    int texture_w, texture_h;
    SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h);
    
    switch (element->style.background_size) {
        case BACKGROUND_SIZE_COVER: {
            // Couvrir tout l'élément en gardant les proportions
            float scale_x = (float)element_rect.w / texture_w;
            float scale_y = (float)element_rect.h / texture_h;
            float scale = fmaxf(scale_x, scale_y);
            
            dest_rect.w = (int)(texture_w * scale);
            dest_rect.h = (int)(texture_h * scale);
            dest_rect.x = element_rect.x + (element_rect.w - dest_rect.w) / 2;
            dest_rect.y = element_rect.y + (element_rect.h - dest_rect.h) / 2;
            break;
        }
        case BACKGROUND_SIZE_CONTAIN: {
            // Contenir dans l'élément en gardant les proportions
            float scale_x = (float)element_rect.w / texture_w;
            float scale_y = (float)element_rect.h / texture_h;
            float scale = fminf(scale_x, scale_y);
            
            dest_rect.w = (int)(texture_w * scale);
            dest_rect.h = (int)(texture_h * scale);
            dest_rect.x = element_rect.x + (element_rect.w - dest_rect.w) / 2;
            dest_rect.y = element_rect.y + (element_rect.h - dest_rect.h) / 2;
            break;
        }
        case BACKGROUND_SIZE_AUTO:
            // Taille originale de l'image
            dest_rect.w = texture_w;
            dest_rect.h = texture_h;
            dest_rect.x = element_rect.x;
            dest_rect.y = element_rect.y;
            break;
        case BACKGROUND_SIZE_STRETCH:
        default:
            // Étirer pour remplir (comportement par défaut)
            break;
    }
    
    return dest_rect;
}

// === DEBUGGING DU TEXTE ===

void atomic_debug_text_rendering(AtomicElement* element, const char* context) {
    if (!element) {
        printf("🔍 [TEXT_DEBUG] [%s] Element is NULL\n", context ? context : "Unknown");
        return;
    }
    
    printf("🔍 [TEXT_DEBUG] [%s] Element '%s' :\n", context ? context : "Unknown", element->id ? element->id : "NoID");
    printf("   📝 Text content: '%s'\n", element->content.text ? element->content.text : "NULL");
    printf("   📐 Element size: %dx%d\n", element->style.width, element->style.height);
    printf("   📍 Text position: (%d, %d)\n", element->style.text_x, element->style.text_y);
    printf("   🎨 Text color: rgba(%d,%d,%d,%d)\n", 
           element->style.text.color.r, element->style.text.color.g, 
           element->style.text.color.b, element->style.text.color.a);
    printf("   📏 Font size: %d\n", element->style.text.font_size);
    printf("   🔤 Text align: %d\n", element->style.text.align);
    printf("   👁️ Visible: %s\n", element->style.visible ? "true" : "false");
    printf("   📋 Display: %d\n", element->style.display);
}

// === NOUVELLES FONCTIONS POUR BACKGROUND CSS ===

void atomic_set_background_size(AtomicElement* element, BackgroundSize size) {
    if (!element) return;
    element->style.background_size = size;
    printf("🖼️ Background size défini : %d\n", size);
}

void atomic_set_background_repeat(AtomicElement* element, BackgroundRepeat repeat) {
    if (!element) return;
    element->style.background_repeat = repeat;
    printf("🔄 Background repeat défini : %d\n", repeat);
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