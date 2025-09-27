#define _POSIX_C_SOURCE 200809L
#include "atomic.h"
#include "../../utils/asset_manager.h"
#include "../../utils/log_console.h"
#include "../../window/window.h"
#include "optimum.h" // Moved from atomic_render for proper practice
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// === GLOBAL CONTEXT ===
static AtomicContext g_atomic_context = {0};

// 🆕 HITBOX VISUALIZATION SYSTEM
static bool g_hitbox_visualization_enabled = true; // Activé par défaut

void atomic_set_hitbox_visualization(bool enabled) {
    g_hitbox_visualization_enabled = enabled;
    
    char message[128];
    snprintf(message, sizeof(message), 
            "[atomic.c] Hitbox visualization %s", enabled ? "ENABLED" : "DISABLED");
    log_console_write("AtomicHitbox", "VisualizationToggle", "atomic.c", message);
    
    printf("🎯 Visualisation des hitboxes : %s\n", enabled ? "ACTIVÉE" : "DÉSACTIVÉE");
}

bool atomic_is_hitbox_visualization_enabled(void) {
    return g_hitbox_visualization_enabled;
}

void atomic_render_hitbox(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer || !g_hitbox_visualization_enabled) return;
    
    // Obtenir la position finale calculée
    SDL_Rect hitbox_rect = atomic_get_final_render_rect(element);
    
    // 🆕 VÉRIFICATION: Ne pas dessiner de hitbox pour des tailles invalides
    if (hitbox_rect.w <= 0 || hitbox_rect.h <= 0) {
        printf("⚠️ [HITBOX_SKIP] Element '%s' has invalid size: %dx%d\n",
               element->id ? element->id : "NoID", hitbox_rect.w, hitbox_rect.h);
        return;
    }
    
    // 🆕 DEBUG DÉTAILLÉ: Log des dimensions pour chaque hitbox rendue
    printf("🎯 [HITBOX_DEBUG] Element '%s':\n", element->id ? element->id : "NoID");
    printf("   📐 Dimensions: %dx%d pixels\n", hitbox_rect.w, hitbox_rect.h);
    printf("   📍 Position: (%d, %d)\n", hitbox_rect.x, hitbox_rect.y);
    printf("   🔲 Bounds: x=%d→%d, y=%d→%d\n", 
           hitbox_rect.x, hitbox_rect.x + hitbox_rect.w,
           hitbox_rect.y, hitbox_rect.y + hitbox_rect.h);
    
    // 🆕 DEBUG: Vérifiui_set_hitbox_visualization(true);cations spécifiques
    if (hitbox_rect.h < 10) {
        printf("🚨 [HITBOX_WARNING] Element '%s' has suspiciously small height: %d\n",
               element->id ? element->id : "NoID", hitbox_rect.h);
    }
    
    if (hitbox_rect.w < 50) {
        printf("🚨 [HITBOX_WARNING] Element '%s' has suspiciously small width: %d\n",
               element->id ? element->id : "NoID", hitbox_rect.w);
    }
    
    // 🆕 DEBUG: Comparaison avec le style original
    printf("   🔧 Style original: %dx%d à (%d,%d)\n",
           element->style.width, element->style.height,
           element->style.x, element->style.y);
    
    // Sauvegarder l'état du renderer
    SDL_BlendMode old_blend_mode;
    SDL_GetRenderDrawBlendMode(renderer, &old_blend_mode);
    Uint8 old_r, old_g, old_b, old_a;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
    
    // Activer le blending pour la transparence
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // 🔴 FOND ROUGE TRANSPARENT (alpha: 80 pour être plus visible)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 80);
    SDL_RenderFillRect(renderer, &hitbox_rect);
    
    // 🔵 BORDURE BLEUE OPAQUE (4 pixels d'épaisseur pour être plus visible)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    
    // Dessiner 4 rectangles pour faire une bordure de 4px
    for (int i = 0; i < 4; i++) {
        SDL_Rect border_rect = {
            hitbox_rect.x - i,
            hitbox_rect.y - i,
            hitbox_rect.w + 2 * i,
            hitbox_rect.h + 2 * i
        };
        SDL_RenderDrawRect(renderer, &border_rect);
    }
    
    printf("   ✅ Hitbox rendue avec bordure bleue 4px\n\n");
    
    // Restaurer l'état du renderer
    SDL_SetRenderDrawBlendMode(renderer, old_blend_mode);
    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

void atomic_set_context(AtomicContext* context) {
    if (context) {
        g_atomic_context = *context;
    }
}

AtomicContext* atomic_get_context(void) {
    return &g_atomic_context;
}

// === UNIFIED POSITIONING SYSTEM ===

static PositioningSystem atomic_determine_positioning_system(AtomicElement* element) {
    if (!element) return POSITIONING_RELATIVE;
    
    if (element->style.display == DISPLAY_FLEX) {
        return POSITIONING_FLEXBOX;
    }
    
    if (element->style.position == POSITION_ABSOLUTE) {
        return POSITIONING_ABSOLUTE;
    }
    
    return POSITIONING_RELATIVE;
}

static int atomic_calculate_total_children_size(AtomicElement* container) {
    if (!container || container->content.children_count == 0) return 0;
    
    bool is_row = (container->style.flex.direction == FLEX_DIRECTION_ROW);
    int total = 0;
    
    for (int i = 0; i < container->content.children_count; i++) {
        AtomicElement* child = container->content.children[i];
        if (!child || !child->style.visible) continue;
        
        if (is_row) {
            total += child->style.width + child->style.margin.left + child->style.margin.right;
        } else {
            total += child->style.height + child->style.margin.top + child->style.margin.bottom;
        }
    }
    
    if (container->content.children_count > 1) {
        total += (container->content.children_count - 1) * container->style.flex.gap;
    }
    
    return total;
}

static void atomic_apply_justify_content(AtomicElement* container, int free_space) {
    if (!container) return;
    
    SDL_Rect content_rect = atomic_get_content_rect(container);
    bool is_row = (container->style.flex.direction == FLEX_DIRECTION_ROW);
    int children_count = container->content.children_count;
    
    int start_pos = is_row ? content_rect.x : content_rect.y;
    int gap = container->style.flex.gap;
    
    switch (container->style.flex.justify_content) {
        case JUSTIFY_CENTER:
            start_pos += free_space / 2;
            break;
        case JUSTIFY_END:
            start_pos += free_space;
            break;
        case JUSTIFY_SPACE_BETWEEN:
            if (children_count > 1) {
                gap += free_space / (children_count - 1);
            }
            break;
        case JUSTIFY_SPACE_AROUND:
            if (children_count > 0) {
                int space_around = free_space / children_count;
                start_pos += space_around / 2;
                gap += space_around;
            }
            break;
        case JUSTIFY_SPACE_EVENLY:
            if (children_count > 0) {
                int space_per_item = free_space / (children_count + 1);
                start_pos += space_per_item;
                gap += space_per_item;
            }
            break;
        case JUSTIFY_START:
        default:
            break;
    }
    
    int current_pos = start_pos;
    for (int i = 0; i < children_count; i++) {
        AtomicElement* child = container->content.children[i];
        if (!child || !child->style.visible) continue;
        
        if (is_row) {
            child->style.x = current_pos + child->style.margin.left;
            current_pos += child->style.width + child->style.margin.left + child->style.margin.right + gap;
        } else {
            child->style.y = current_pos + child->style.margin.top;
            current_pos += child->style.height + child->style.margin.top + child->style.margin.bottom + gap;
        }
    }
}

static void atomic_apply_align_item(AtomicElement* container, AtomicElement* child) {
    if (!container || !child) return;
    
    SDL_Rect container_rect = atomic_get_content_rect(container);
    bool is_row = (container->style.flex.direction == FLEX_DIRECTION_ROW);
    
    if (is_row) {
        switch (container->style.flex.align_items) {
            case ALIGN_CENTER:
            case ALIGN_MIDDLE:
                child->style.y = container_rect.y + (container_rect.h - child->style.height) / 2;
                break;
            case ALIGN_BOTTOM:
                child->style.y = container_rect.y + container_rect.h - child->style.height - child->style.margin.bottom;
                break;
            case ALIGN_STRETCH:
                child->style.height = container_rect.h - child->style.margin.top - child->style.margin.bottom;
                child->style.y = container_rect.y + child->style.margin.top;
                break;
            case ALIGN_TOP:
            default:
                child->style.y = container_rect.y + child->style.margin.top;
                break;
        }
    } else { // Column direction
        switch (container->style.flex.align_items) {
            case ALIGN_CENTER:
                child->style.x = container_rect.x + (container_rect.w - child->style.width) / 2;
                break;
            case ALIGN_RIGHT:
                child->style.x = container_rect.x + container_rect.w - child->style.width - child->style.margin.right;
                break;
            case ALIGN_STRETCH:
                child->style.width = container_rect.w - child->style.margin.left - child->style.margin.right;
                child->style.x = container_rect.x + child->style.margin.left;
                break;
            case ALIGN_LEFT:
            default:
                child->style.x = container_rect.x + child->style.margin.left;
                break;
        }
    }
}

// === IMPROVED FLEXBOX SYSTEM ===

void atomic_apply_flex_shrink(AtomicElement* container) {
    if (!container || container->style.display != DISPLAY_FLEX) return;
    
    SDL_Rect container_rect = atomic_get_content_rect(container);
    bool is_row = (container->style.flex.direction == FLEX_DIRECTION_ROW);
    int available_space = is_row ? container_rect.w : container_rect.h;
    
    // Calculer l'espace total nécessaire
    int total_needed = atomic_calculate_total_children_size(container);
    
    if (total_needed <= available_space) {
        return; // Pas besoin de rétrécir
    }
    
    // Calculer l'excès à répartir
    int overflow = total_needed - available_space;
    int total_shrink_factor = 0;
    
    // Calculer le facteur de rétrécissement total
    for (int i = 0; i < container->content.children_count; i++) {
        AtomicElement* child = container->content.children[i];
        if (child && child->style.visible && child->style.flex.shrink > 0) {
            total_shrink_factor += child->style.flex.shrink;
        }
    }
    
    if (total_shrink_factor == 0) return;
    
    // Appliquer le rétrécissement
    for (int i = 0; i < container->content.children_count; i++) {
        AtomicElement* child = container->content.children[i];
        if (!child || !child->style.visible || child->style.flex.shrink <= 0) continue;
        
        int shrink_amount = (overflow * child->style.flex.shrink) / total_shrink_factor;
        
        if (is_row) {
            child->style.width = (child->style.width > shrink_amount) ? 
                                child->style.width - shrink_amount : 1;
        } else {
            child->style.height = (child->style.height > shrink_amount) ? 
                                 child->style.height - shrink_amount : 1;
        }
    }
    
    log_console_write("AtomicFlex", "ShrinkApplied", "atomic.c", 
                     "[atomic.c] Flex shrink applied to children");
}

static void atomic_apply_flex_wrap(AtomicElement* container) {
    if (!container || !container->style.flex.wrap) return;
    
    // TODO: Implémentation complète du wrap
    // Pour l'instant, log seulement
    log_console_write("AtomicFlex", "WrapRequested", "atomic.c", 
                     "[atomic.c] Flex wrap requested but not yet fully implemented");
}

void atomic_apply_flex_layout_improved(AtomicElement* container) {
    if (!container || container->style.display != DISPLAY_FLEX) return;
    
    SDL_Rect container_rect = atomic_get_content_rect(container);
    int children_count = container->content.children_count;
    if (children_count == 0) return;
    
    bool is_row = (container->style.flex.direction == FLEX_DIRECTION_ROW);
    int available_space = is_row ? container_rect.w : container_rect.h;
    
    // 1. Appliquer le shrink si nécessaire
    atomic_apply_flex_shrink(container);
    
    // 2. Appliquer le wrap si activé
    if (container->style.flex.wrap) {
        atomic_apply_flex_wrap(container);
    }
    
    // 3. Calculer et appliquer grow/justify/align comme avant
    int total_children_size = atomic_calculate_total_children_size(container);
    int free_space = available_space - total_children_size;
    
    int flexible_children = 0;
    int total_flex_grow = 0;
    
    for (int i = 0; i < children_count; i++) {
        AtomicElement* child = container->content.children[i];
        if (!child || !child->style.visible) continue;
        if (child->style.flex.grow > 0) {
            flexible_children++;
            total_flex_grow += child->style.flex.grow;
        }
    }
    
    if (flexible_children > 0 && free_space > 0 && total_flex_grow > 0) {
        for (int i = 0; i < children_count; i++) {
            AtomicElement* child = container->content.children[i];
            if (!child || !child->style.visible || child->style.flex.grow <= 0) continue;
            
            int space_to_add = (free_space * child->style.flex.grow) / total_flex_grow;
            if (is_row) {
                child->style.width += space_to_add;
            } else {
                child->style.height += space_to_add;
            }
        }
        total_children_size = atomic_calculate_total_children_size(container);
        free_space = available_space - total_children_size;
    }
    
    atomic_apply_justify_content(container, free_space);
    
    for (int i = 0; i < children_count; i++) {
        AtomicElement* child = container->content.children[i];
        if (!child || !child->style.visible) continue;
        atomic_apply_align_item(container, child);
        if (child->style.alignment.align_self != ALIGN_SELF_AUTO) {
            atomic_apply_align_self(child);
        }
    }
    
    log_console_write("AtomicFlex", "CompleteLayout", "atomic.c", 
                     "[atomic.c] Complete flexbox layout applied with shrink and wrap support");
}

// === ABSOLUTE POSITIONING SYSTEM ===

static void atomic_apply_absolute_positioning(AtomicElement* element) {
    if (!element || !element->parent) return;
    SDL_Rect parent_rect = atomic_get_content_rect(element->parent);
    element->style.x += parent_rect.x;
    element->style.y += parent_rect.y;
}

// === RELATIVE POSITIONING SYSTEM ===

static void atomic_apply_relative_positioning(AtomicElement* element) {
    if (!element) return;
    // Relative positioning maintains its coordinates as is.
    // Alignment will be handled by align-self if necessary.
}

// === MAIN UNIFIED SYSTEM ===

void atomic_calculate_layout(AtomicElement* element) {
    if (!element) return;
    
    PositioningSystem system = atomic_determine_positioning_system(element);
    
    switch (system) {
        case POSITIONING_FLEXBOX:
            atomic_apply_flex_layout_improved(element);
            break;
        case POSITIONING_ABSOLUTE:
            atomic_apply_absolute_positioning(element);
            break;
        case POSITIONING_RELATIVE:
            atomic_apply_relative_positioning(element);
            break;
        default:
            break;
    }
}

// === EVENT HANDLING FUNCTIONS ===

// 🆕 NEW FUNCTION TO GET FINAL CALCULATED POSITION - MOVED BEFORE USAGE
/**
 * Obtenir le rectangle final calculé après tous les calculs de layout
 * Cette fonction retourne la position réelle à l'écran après flexbox, align-self, etc.
 */
SDL_Rect atomic_get_final_render_rect(AtomicElement* element) {
    if (!element) return (SDL_Rect){0,0,0,0};

    // Obtenir le rectangle de rendu calculé (peut venir du layout)
    SDL_Rect rect = atomic_get_render_rect(element);

    // Si le layout n'a pas fixé une taille, utiliser la taille par défaut définie dans style
    // (ou une valeur fallback si style n'a rien)
    if (rect.w <= 0) {
        rect.w = (element->style.width > 0) ? element->style.width : 100;
    }
    if (rect.h <= 0) {
        rect.h = (element->style.height > 0) ? element->style.height : 50;
    }

    return rect;
}

void atomic_handle_event(SDL_Event* event, void* user_data) {
    if (!event || !user_data) return;
    AtomicElement* element = (AtomicElement*)user_data;
    if (!element || !element->style.visible) return;
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            if (element->events.on_click) {
                element->events.on_click(element, event);
            }
            break;
        case SDL_MOUSEMOTION: {
            int mouse_x, mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);
            
            bool was_hovered = element->is_hovered;
            bool is_now_hovered = atomic_is_point_inside(element, mouse_x, mouse_y);
            
            if (!was_hovered && is_now_hovered) {
                element->is_hovered = true;
                if (element->events.on_hover) {
                    element->events.on_hover(element, event);
                }
            } else if (was_hovered && !is_now_hovered) {
                element->is_hovered = false;
                if (element->events.on_unhover) {
                    element->events.on_unhover(element, event);
                }
            }
            break;
        }
        default:
            break;
    }
}

void atomic_register_with_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    
    // 🔧 FIX: Utiliser la position finale calculée au lieu de la position de style brute
    SDL_Rect rect = atomic_get_final_render_rect(element);
    
    event_manager_subscribe(manager, rect.x, rect.y, rect.w, rect.h, 
                          element->style.z_index, element->style.visible,
                          atomic_handle_event, element);
    
    log_console_write("AtomicEvent", "RegisteredWithFinalPosition", "atomic.c", 
                     "[atomic.c] Element registered with final calculated position for accurate hit testing");
}

void atomic_unregister_from_event_manager(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    event_manager_unsubscribe(manager, atomic_handle_event, element);
}

// === STYLE FUNCTIONS ===

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

// === FLEXBOX FUNCTIONS ===

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

void atomic_set_flex_grow(AtomicElement* element, int grow) {
    if (!element) return;
    element->style.flex.grow = grow;
}

void atomic_set_flex_shrink(AtomicElement* element, int shrink) {
    if (!element) return;
    element->style.flex.shrink = shrink;
}

// === TEXT FUNCTIONS ===

void atomic_set_font(AtomicElement* element, const char* font_path, int size) {
    if (!element || !font_path) return;
    free(element->style.text.font_path);
    element->style.text.font_path = strdup(font_path);
    element->style.text.font_size = size;
}

void atomic_set_text_color(AtomicElement* element, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!element) return;
    element->style.text.color = (SDL_Color){r, g, b, a};
}

void atomic_set_text_color_rgba(AtomicElement* element, int r, int g, int b, int a) {
    if (!element) return;
    element->style.text.color = (SDL_Color){(Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a};
}

void atomic_set_text_align(AtomicElement* element, TextAlign align) {
    if (!element) return;
    element->style.text.align = align;
}

void atomic_set_text(AtomicElement* element, const char* text) {
    if (!element) return;
    free(element->content.text);
    element->content.text = text ? strdup(text) : NULL;
}

// === CONTENT FUNCTIONS ===

void atomic_add_child(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return;
    
    if (parent->content.children_count >= parent->content.children_capacity) {
        int new_capacity = parent->content.children_capacity == 0 ? 4 : parent->content.children_capacity * 2;
        AtomicElement** new_children = realloc(parent->content.children, new_capacity * sizeof(AtomicElement*));
        
        if (!new_children) {
            // Handle realloc failure
            return;
        }
        parent->content.children = new_children;
        parent->content.children_capacity = new_capacity;
    }
    
    parent->content.children[parent->content.children_count++] = child;
    child->parent = parent;
}

void atomic_remove_child(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return;
    
    for (int i = 0; i < parent->content.children_count; i++) {
        if (parent->content.children[i] == child) {
            // Shift remaining children
            for (int j = i; j < parent->content.children_count - 1; j++) {
                parent->content.children[j] = parent->content.children[j + 1];
            }
            parent->content.children_count--;
            child->parent = NULL;
            break;
        }
    }
}

// === ALIGNMENT FUNCTIONS ===

void atomic_set_align_self(AtomicElement* element, AlignSelf align_self) {
    if (!element) return;
    element->style.alignment.align_self = align_self;
}

void atomic_apply_align_self(AtomicElement* element) {
    if (!element || !element->parent || element->style.alignment.align_self == ALIGN_SELF_AUTO) return;
    
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
        default:
            break;
    }
}

// === ERROR HANDLING IMPLEMENTATION ===

static void atomic_set_error(AtomicElement* element, AtomicError error, const char* message) {
    if (!element) return;
    
    element->last_error = error;
    free(element->error_message);
    element->error_message = message ? strdup(message) : NULL;
    
    // Log l'erreur
    char error_log[512];
    snprintf(error_log, sizeof(error_log), 
            "[atomic.c] ERROR %d in element '%s': %s", 
            error, element->id ? element->id : "NoID", message ? message : "Unknown error");
    log_console_write("AtomicError", "Error", "atomic.c", error_log);
}

AtomicError atomic_get_last_error(AtomicElement* element) {
    return element ? element->last_error : ATOMIC_ERROR_NULL_POINTER;
}

const char* atomic_get_error_message(AtomicElement* element) {
    return (element && element->error_message) ? element->error_message : "No error message";
}

void atomic_clear_error(AtomicElement* element) {
    if (!element) return;
    element->last_error = ATOMIC_SUCCESS;
    free(element->error_message);
    element->error_message = NULL;
}

const char* atomic_error_to_string(AtomicError error) {
    switch (error) {
        case ATOMIC_SUCCESS: return "Success";
        case ATOMIC_ERROR_NULL_POINTER: return "Null pointer";
        case ATOMIC_ERROR_INVALID_PARAMETER: return "Invalid parameter";
        case ATOMIC_ERROR_MEMORY_ALLOCATION: return "Memory allocation failed";
        case ATOMIC_ERROR_TEXTURE_LOADING: return "Texture loading failed";
        case ATOMIC_ERROR_CIRCULAR_REFERENCE: return "Circular reference detected";
        case ATOMIC_ERROR_MAX_CHILDREN_EXCEEDED: return "Maximum children exceeded";
        case ATOMIC_ERROR_FONT_LOADING: return "Font loading failed";
        default: return "Unknown error";
    }
}

// === GLOBAL TEXTURE REFERENCE SYSTEM ===
static AtomicTextureRef* g_texture_refs = NULL;
static SDL_mutex* g_texture_mutex = NULL;

static void atomic_texture_ref_init(void) {
    if (!g_texture_mutex) {
        g_texture_mutex = SDL_CreateMutex();
    }
}

void atomic_texture_ref_add(const char* path, SDL_Texture* texture) {
    if (!path || !texture) return;
    
    atomic_texture_ref_init();
    SDL_LockMutex(g_texture_mutex);
    
    // Chercher si la texture existe déjà
    AtomicTextureRef* current = g_texture_refs;
    while (current) {
        if (strcmp(current->path, path) == 0) {
            current->ref_count++;
            SDL_UnlockMutex(g_texture_mutex);
            return;
        }
        current = current->next;
    }
    
    // Créer une nouvelle référence
    AtomicTextureRef* new_ref = (AtomicTextureRef*)malloc(sizeof(AtomicTextureRef));
    if (new_ref) {
        new_ref->texture = texture;
        new_ref->path = strdup(path);
        new_ref->ref_count = 1;
        new_ref->next = g_texture_refs;
        g_texture_refs = new_ref;
        
        log_console_write("AtomicTexture", "RefAdded", "atomic.c", 
                         "[atomic.c] Texture reference added with count=1");
    }
    
    SDL_UnlockMutex(g_texture_mutex);
}

SDL_Texture* atomic_texture_ref_get(const char* path) {
    if (!path) return NULL;
    
    atomic_texture_ref_init();
    SDL_LockMutex(g_texture_mutex);
    
    AtomicTextureRef* current = g_texture_refs;
    while (current) {
        if (strcmp(current->path, path) == 0) {
            current->ref_count++;
            SDL_UnlockMutex(g_texture_mutex);
            return current->texture;
        }
        current = current->next;
    }
    
    SDL_UnlockMutex(g_texture_mutex);
    return NULL;
}

void atomic_texture_ref_release(const char* path) {
    if (!path) return;
    
    SDL_LockMutex(g_texture_mutex);
    
    AtomicTextureRef* current = g_texture_refs;
    AtomicTextureRef* prev = NULL;
    
    while (current) {
        if (strcmp(current->path, path) == 0) {
            current->ref_count--;
            
            if (current->ref_count <= 0) {
                // Supprimer la référence et détruire la texture
                if (prev) {
                    prev->next = current->next;
                } else {
                    g_texture_refs = current->next;
                }
                
                SDL_DestroyTexture(current->texture);
                free(current->path);
                free(current);
                
                log_console_write("AtomicTexture", "RefReleased", "atomic.c", 
                                 "[atomic.c] Texture reference released and destroyed");
            }
            break;
        }
        prev = current;
        current = current->next;
    }
    
    SDL_UnlockMutex(g_texture_mutex);
}

void atomic_texture_ref_cleanup_all(void) {
    if (!g_texture_mutex) return;
    
    SDL_LockMutex(g_texture_mutex);
    
    AtomicTextureRef* current = g_texture_refs;
    while (current) {
        AtomicTextureRef* next = current->next;
        SDL_DestroyTexture(current->texture);
        free(current->path);
        free(current);
        current = next;
    }
    
    g_texture_refs = NULL;
    SDL_UnlockMutex(g_texture_mutex);
    
    SDL_DestroyMutex(g_texture_mutex);
    g_texture_mutex = NULL;
    
    log_console_write("AtomicTexture", "Cleanup", "atomic.c", 
                     "[atomic.c] All texture references cleaned up");
}

// === SAFE DESTRUCTION SYSTEM ===

// 🔧 FIX: Remove unused function to fix warning
// static bool atomic_detect_circular_reference(AtomicElement* element, AtomicElement* potential_child) {
//     // Function removed - was unused and causing warning
// }

bool atomic_is_destroying(AtomicElement* element) {
    return element && (element->state == ELEMENT_STATE_DESTROYING || 
                      element->state == ELEMENT_STATE_DESTROYED);
}

AtomicError atomic_destroy_safe(AtomicElement* element) {
    if (!element) return ATOMIC_ERROR_NULL_POINTER;
    
    if (element->state == ELEMENT_STATE_DESTROYING) {
        return ATOMIC_SUCCESS; // Déjà en cours de destruction
    }
    
    if (element->state == ELEMENT_STATE_DESTROYED) {
        return ATOMIC_ERROR_INVALID_PARAMETER; // Déjà détruit
    }
    
    // Marquer comme en cours de destruction
    element->state = ELEMENT_STATE_DESTROYING;
    
    log_console_write("AtomicDestruction", "Starting", "atomic.c", 
                     "[atomic.c] Starting safe destruction");
    
    // Détruire récursivement tous les enfants
    for (int i = 0; i < element->content.children_count; i++) {
        AtomicElement* child = element->content.children[i];
        if (child && !atomic_is_destroying(child)) {
            child->parent = NULL; // Éviter la référence circulaire
            atomic_destroy_safe(child);
        }
    }
    
    // Supprimer l'élément de son parent
    if (element->parent) {
        atomic_remove_child_safe(element->parent, element);
    }
    
    // 🔧 FIX: Properly handle texture_ref cleanup
    if (element->texture_ref) {
        if (element->texture_ref->path) {
            atomic_texture_ref_release(element->texture_ref->path);
        }
        free(element->texture_ref);
        element->texture_ref = NULL;
    }
    
    // Libérer la mémoire
    free(element->id);
    free(element->class_name);
    free(element->content.text);
    free(element->style.background_image_path);
    free(element->style.text.font_path);
    free(element->error_message);
    free(element->content.children);
    
    // Marquer comme détruit
    element->state = ELEMENT_STATE_DESTROYED;
    free(element);
    
    log_console_write("AtomicDestruction", "Completed", "atomic.c", 
                     "[atomic.c] Safe destruction completed");
    
    return ATOMIC_SUCCESS;
}

AtomicError atomic_remove_child_safe(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return ATOMIC_ERROR_NULL_POINTER;
    
    if (atomic_is_destroying(parent) || atomic_is_destroying(child)) {
        return ATOMIC_SUCCESS; // Déjà en cours de destruction
    }
    
    for (int i = 0; i < parent->content.children_count; i++) {
        if (parent->content.children[i] == child) {
            // Décaler les éléments suivants
            for (int j = i; j < parent->content.children_count - 1; j++) {
                parent->content.children[j] = parent->content.children[j + 1];
            }
            parent->content.children_count--;
            child->parent = NULL;
            
            log_console_write("AtomicHierarchy", "ChildRemoved", "atomic.c", 
                             "[atomic.c] Child safely removed from parent");
            return ATOMIC_SUCCESS;
        }
    }
    
    return ATOMIC_ERROR_INVALID_PARAMETER;
}

// === IMPROVED TEXTURE LOADING WITH REFERENCE COUNTING ===

AtomicError atomic_set_background_image_with_path(AtomicElement* element, const char* path, SDL_Renderer* renderer) {
    if (!element || !path || !renderer) {
        if (element) {
            atomic_set_error(element, ATOMIC_ERROR_NULL_POINTER, "Invalid parameters for background image");
        }
        return ATOMIC_ERROR_NULL_POINTER;
    }
    
    if (atomic_is_destroying(element)) {
        return ATOMIC_ERROR_INVALID_PARAMETER;
    }
    
    // 🔧 FIX: Properly handle texture_ref cleanup
    if (element->texture_ref) {
        if (element->texture_ref->path) {
            atomic_texture_ref_release(element->texture_ref->path);
        }
        free(element->texture_ref);
        element->texture_ref = NULL;
    }
    
    // Essayer d'obtenir une texture existante
    SDL_Texture* texture = atomic_texture_ref_get(path);
    
    if (!texture) {
        // Charger la nouvelle texture
        SDL_Surface* surface = IMG_Load(path);
        if (!surface) {
            atomic_set_error(element, ATOMIC_ERROR_TEXTURE_LOADING, "Failed to load image surface");
            return ATOMIC_ERROR_TEXTURE_LOADING;
        }
        
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        if (!texture) {
            atomic_set_error(element, ATOMIC_ERROR_TEXTURE_LOADING, "Failed to create texture from surface");
            return ATOMIC_ERROR_TEXTURE_LOADING;
        }
        
        // Ajouter à la référence
        atomic_texture_ref_add(path, texture);
    }
    
    // Mettre à jour l'élément
    element->style.background_image = texture;
    
    // 🔧 FIX: Properly allocate and initialize texture_ref
    element->texture_ref = (AtomicTextureRef*)malloc(sizeof(AtomicTextureRef));
    if (!element->texture_ref) {
        atomic_set_error(element, ATOMIC_ERROR_MEMORY_ALLOCATION, "Failed to allocate texture reference");
        return ATOMIC_ERROR_MEMORY_ALLOCATION;
    }
    
    element->texture_ref->texture = texture;
    element->texture_ref->path = strdup(path);
    element->texture_ref->ref_count = 1;
    element->texture_ref->next = NULL;
    
    atomic_clear_error(element);
    log_console_write("AtomicTexture", "BackgroundSet", "atomic.c", 
                     "[atomic.c] Background image set with reference counting");
    
    return ATOMIC_SUCCESS;
}

// === UPDATED CREATION FUNCTION WITH ERROR HANDLING ===

AtomicElement* atomic_create(const char* id) {
    AtomicElement* element = calloc(1, sizeof(AtomicElement));
    if (!element) return NULL;
    
    // Initialiser l'état
    element->state = ELEMENT_STATE_ACTIVE;
    element->last_error = ATOMIC_SUCCESS;
    element->error_message = NULL;
    element->texture_ref = NULL;
    
    if (id) {
        element->id = strdup(id);
        if (!element->id) {
            free(element);
            return NULL;
        }
    }
    
    // Default values
    element->style.width = 100;
    element->style.height = 50;
    element->style.visible = true;
    element->style.opacity = 255;
    element->style.display = DISPLAY_BLOCK;
    element->style.position = POSITION_STATIC;
    element->style.overflow = OVERFLOW_VISIBLE;
    element->style.alignment.align_self = ALIGN_SELF_AUTO;
    
    // 🔧 FIX: Désactiver le shrink par défaut pour éviter les problèmes
    element->style.flex.shrink = 0;  // 🆕 Changé de 1 à 0 pour éviter le shrink automatique
    element->style.flex.wrap = false; // Pas de wrap par défaut
    
    element->content.children_capacity = 4;
    element->content.children = calloc(element->content.children_capacity, sizeof(AtomicElement*));
    
    if (!element->content.children) {
        free(element->id);
        free(element);
        return NULL;
    }
    
    log_console_write("AtomicElement", "Created", "atomic.c", 
                     "[atomic.c] Element created with error handling");
    
    return element;
}

// === COMPATIBILITY WRAPPER ===

void atomic_destroy(AtomicElement* element) {
    atomic_destroy_safe(element);
}

// === IMPROVED UPDATE FUNCTION ===

void atomic_update(AtomicElement* element, float delta_time) {
    if (!element || !element->style.visible) return;
    
    // 1. Custom update first
    if (element->custom_update) {
        element->custom_update(element, delta_time);
    }
    
    // 2. Main layout calculation (one system)
    atomic_calculate_layout(element);
    
    // 3. Individual alignment (only if not in a flex container)
    if (element->style.alignment.align_self != ALIGN_SELF_AUTO &&
        element->parent && 
        element->parent->style.display != DISPLAY_FLEX) {
        atomic_apply_align_self(element);
    }
    
    // 4. Final constraints (overflow)
    atomic_apply_overflow_constraints(element);
    
    // ❌ SUPPRESSION: Ne plus synchroniser ici pour éviter la synchronisation prématurée
    // La synchronisation des hitboxes se fera maintenant dans optimum_sync_all_hitboxes_post_layout()
    // après que TOUS les calculs de l'arbre soient terminés
    
    // 5. Update children
    for (int i = 0; i < element->content.children_count; i++) {
        atomic_update(element->content.children[i], delta_time);
    }
    
    // 🆕 LOG de fin de mise à jour individuelle
    static int update_counter = 0;
    if (update_counter++ % 120 == 0) { // Log toutes les 120 frames pour éviter le spam
        char update_message[256];
        snprintf(update_message, sizeof(update_message),
                "[atomic.c] Element '%s' individual update completed - waiting for tree-wide hitbox sync",
                element->id ? element->id : "NoID");
        log_console_write("AtomicUpdate", "IndividualDone", "atomic.c", update_message);
    }
}

// === NEW UTILITY FUNCTION FOR EVENT MANAGER SYNC ===

/**
 * Synchroniser la position d'un élément avec l'event manager
 * À appeler après les calculs de layout pour mettre à jour la hitbox
 */
void atomic_sync_event_manager_position(AtomicElement* element, EventManager* manager) {
    if (!element || !manager) return;
    
    // 🔧 AMÉLIORATION: Désenregistrer ET réenregistrer pour une synchronisation complète
    atomic_unregister_from_event_manager(element, manager);
    
    // Réenregistrer avec la position finale calculée (après tous les calculs)
    atomic_register_with_event_manager(element, manager);
    
    // Log détaillé de la synchronisation
    SDL_Rect final_rect = atomic_get_final_render_rect(element);
    char sync_message[512];
    snprintf(sync_message, sizeof(sync_message),
            "[atomic.c] Element '%s' synchronized - Final position: (%d,%d,%dx%d)",
            element->id ? element->id : "NoID",
            final_rect.x, final_rect.y, final_rect.w, final_rect.h);
    log_console_write("AtomicSync", "ElementSynced", "atomic.c", sync_message);
}

// === UTILITY FUNCTIONS ===

bool atomic_is_point_inside(AtomicElement* element, int x, int y) {
    if (!element || !element->style.visible) return false;

    // Utiliser la position/tailles finales pour la détection de clics/hover
    SDL_Rect rect = atomic_get_final_render_rect(element);
    return (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h);
}

SDL_Rect atomic_get_render_rect(AtomicElement* element) {
    SDL_Rect rect = {0, 0, 0, 0};
    if (!element) return rect;
    rect.x = element->style.x; // Render rect does not include margins
    rect.y = element->style.y;
    rect.w = element->style.width;
    rect.h = element->style.height;
    return rect;
}

SDL_Rect atomic_get_content_rect(AtomicElement* element) {
    SDL_Rect rect = atomic_get_render_rect(element);
    if (!element) return rect;
    rect.x += element->style.padding.left;
    rect.y += element->style.padding.top;
    rect.w -= (element->style.padding.left + element->style.padding.right);
    rect.h -= (element->style.padding.top + element->style.padding.bottom);
    return rect;
}

// === OVERFLOW HANDLING ===

void atomic_set_overflow(AtomicElement* element, OverflowType overflow) {
    if (!element) return;
    element->style.overflow = overflow;
}

bool atomic_is_child_overflowing(AtomicElement* parent, AtomicElement* child) {
    if (!parent || !child) return false;
    SDL_Rect parent_rect = atomic_get_content_rect(parent);
    SDL_Rect child_rect = atomic_get_render_rect(child);
    return (child_rect.x < parent_rect.x ||
            child_rect.y < parent_rect.y ||
            (child_rect.x + child_rect.w) > (parent_rect.x + parent_rect.w) ||
            (child_rect.y + child_rect.h) > (parent_rect.y + parent_rect.h));
}

void atomic_apply_overflow_constraints(AtomicElement* element) {
    if (!element || element->style.overflow == OVERFLOW_VISIBLE) return;
    
    for (int i = 0; i < element->content.children_count; i++) {
        AtomicElement* child = element->content.children[i];
        if (!child) continue;
        
        if (atomic_is_child_overflowing(element, child)) {
            // Note: This implements clamping. For OVERFLOW_HIDDEN, clipping
            // would happen at render time. This logic prevents elements from moving
            // outside the parent's content box.
            SDL_Rect parent_rect = atomic_get_content_rect(element);
            
            if (child->style.x < parent_rect.x) {
                child->style.x = parent_rect.x;
            }
            if (child->style.y < parent_rect.y) {
                child->style.y = parent_rect.y;
            }
            if (child->style.x + child->style.width > parent_rect.x + parent_rect.w) {
                child->style.x = parent_rect.x + parent_rect.w - child->style.width;
            }
            if (child->style.y + child->style.height > parent_rect.y + parent_rect.h) {
                child->style.y = parent_rect.y + parent_rect.h - child->style.height;
            }
        }
    }
}

// === MISSING FUNCTION IMPLEMENTATIONS ===

void atomic_set_background_image(AtomicElement* element, SDL_Texture* texture) {
    if (!element) return;
    element->style.background_image = texture;
}

void atomic_set_hover_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_hover = handler;
}

void atomic_set_unhover_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_unhover = handler;
}

void atomic_set_click_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_click = handler;
}

// === ADDITIONAL MISSING FUNCTION IMPLEMENTATIONS ===

void atomic_set_background_size_str(AtomicElement* element, const char* size) {
    if (!element || !size) return;
    
    if (strcmp(size, "cover") == 0) {
        element->style.background_size = BACKGROUND_SIZE_COVER;
    } else if (strcmp(size, "contain") == 0) {
        element->style.background_size = BACKGROUND_SIZE_CONTAIN;
    } else if (strcmp(size, "stretch") == 0) {
        element->style.background_size = BACKGROUND_SIZE_STRETCH;
    } else {
        element->style.background_size = BACKGROUND_SIZE_AUTO;
    }
}

void atomic_set_background_repeat_str(AtomicElement* element, const char* repeat) {
    if (!element || !repeat) return;
    
    if (strcmp(repeat, "no-repeat") == 0) {
        element->style.background_repeat = BACKGROUND_REPEAT_NO_REPEAT;
    } else if (strcmp(repeat, "repeat-x") == 0) {
        element->style.background_repeat = BACKGROUND_REPEAT_REPEAT_X;
    } else if (strcmp(repeat, "repeat-y") == 0) {
        element->style.background_repeat = BACKGROUND_REPEAT_REPEAT_Y;
    } else {
        element->style.background_repeat = BACKGROUND_REPEAT_REPEAT;
    }
}

void atomic_set_text_align_str(AtomicElement* element, const char* align) {
    if (!element || !align) return;
    
    if (strcmp(align, "center") == 0) {
        element->style.text.align = TEXT_ALIGN_CENTER;
    } else if (strcmp(align, "right") == 0) {
        element->style.text.align = TEXT_ALIGN_RIGHT;
    } else if (strcmp(align, "justify") == 0) {
        element->style.text.align = TEXT_ALIGN_JUSTIFY;
    } else {
        element->style.text.align = TEXT_ALIGN_LEFT;
    }
}

void atomic_set_text_position(AtomicElement* element, int x, int y) {
    if (!element) return;
    element->style.text_x = x;
    element->style.text_y = y;
}

void atomic_set_texture(AtomicElement* element, SDL_Texture* texture) {
    if (!element) return;
    element->content.texture = texture;
}

void atomic_set_display_str(AtomicElement* element, const char* display) {
    if (!element || !display) return;
    if (strcmp(display, "flex") == 0) atomic_set_display(element, DISPLAY_FLEX);
    else if (strcmp(display, "none") == 0) atomic_set_display(element, DISPLAY_NONE);
    else atomic_set_display(element, DISPLAY_BLOCK);
}

void atomic_set_flex_direction_str(AtomicElement* element, const char* direction) {
    if (!element || !direction) return;
    if (strcmp(direction, "column") == 0) {
        atomic_set_flex_direction(element, FLEX_DIRECTION_COLUMN);
    } else {
        atomic_set_flex_direction(element, FLEX_DIRECTION_ROW);
    }
}

void atomic_set_justify_content_str(AtomicElement* element, const char* justify) {
    if (!element || !justify) return;
    
    if (strcmp(justify, "center") == 0) {
        element->style.flex.justify_content = JUSTIFY_CENTER;
    } else if (strcmp(justify, "end") == 0) {
        element->style.flex.justify_content = JUSTIFY_END;
    } else if (strcmp(justify, "space-between") == 0) {
        element->style.flex.justify_content = JUSTIFY_SPACE_BETWEEN;
    } else if (strcmp(justify, "space-around") == 0) {
        element->style.flex.justify_content = JUSTIFY_SPACE_AROUND;
    } else if (strcmp(justify, "space-evenly") == 0) {
        element->style.flex.justify_content = JUSTIFY_SPACE_EVENLY;
    } else {
        element->style.flex.justify_content = JUSTIFY_START;
    }
}

void atomic_set_align_items_str(AtomicElement* element, const char* align) {
    if (!element || !align) return;
    
    if (strcmp(align, "center") == 0) {
        element->style.flex.align_items = ALIGN_CENTER;
    } else if (strcmp(align, "end") == 0) {
        element->style.flex.align_items = ALIGN_END;
    } else if (strcmp(align, "stretch") == 0) {
        element->style.flex.align_items = ALIGN_STRETCH;
    } else {
        element->style.flex.align_items = ALIGN_START;
    }
}

void atomic_debug_text_rendering(AtomicElement* element, const char* context) {
    if (!element) {
        printf("🔍 [ATOMIC_DEBUG] [%s] Element is NULL\n", context ? context : "Unknown");
        return;
    }
    
    printf("🔍 [ATOMIC_DEBUG] [%s] Element ID: %s\n", 
           context ? context : "Unknown", element->id ? element->id : "NoID");
    printf("   Text: %s\n", element->content.text ? element->content.text : "NULL");
    printf("   Visible: %s\n", element->style.visible ? "true" : "false");
    printf("   Size: %dx%d\n", element->style.width, element->style.height);
    printf("   Position: (%d,%d)\n", element->style.x, element->style.y);
    printf("   Text Color: rgba(%d,%d,%d,%d)\n", 
           element->style.text.color.r, element->style.text.color.g,
           element->style.text.color.b, element->style.text.color.a);
}

void atomic_set_text_font(AtomicElement* element, TTF_Font* font) {
    if (!element) return;
    element->style.text.font = font;
    element->style.font = font; // For compatibility
}

void atomic_set_text_size(AtomicElement* element, int size) {
    if (!element) return;
    element->style.text.font_size = size;
    element->style.font_size = size; // For compatibility
}

void atomic_add_class(AtomicElement* element, const char* class_name) {
    if (!element || !class_name) return;
    
    // Simple implementation - just store one class name
    free(element->class_name);
    element->class_name = strdup(class_name);
}

void atomic_set_align(AtomicElement* element, const char* horizontal, const char* vertical) {
    if (!element) return;
    
    // Parse horizontal alignment
    if (horizontal) {
        if (strcmp(horizontal, "center") == 0) {
            element->style.alignment.horizontal = ALIGN_CENTER;
        } else if (strcmp(horizontal, "right") == 0) {
            element->style.alignment.horizontal = ALIGN_RIGHT;
        } else {
            element->style.alignment.horizontal = ALIGN_LEFT;
        }
    }
    
    // Parse vertical alignment
    if (vertical) {
        if (strcmp(vertical, "middle") == 0 || strcmp(vertical, "center") == 0) {
            element->style.alignment.vertical = ALIGN_MIDDLE;
        } else if (strcmp(vertical, "bottom") == 0) {
            element->style.alignment.vertical = ALIGN_BOTTOM;
        } else {
            element->style.alignment.vertical = ALIGN_TOP;
        }
    }
}

void atomic_set_text_style(AtomicElement* element, bool bold, bool italic) {
    if (!element) return;
    element->style.text.bold = bold;
    element->style.text.italic = italic;
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

void atomic_set_overflow_str(AtomicElement* element, const char* overflow) {
    if (!element || !overflow) return;
    
    if (strcmp(overflow, "hidden") == 0) {
        element->style.overflow = OVERFLOW_HIDDEN;
    } else if (strcmp(overflow, "scroll") == 0) {
        element->style.overflow = OVERFLOW_SCROLL;
    } else if (strcmp(overflow, "auto") == 0) {
        element->style.overflow = OVERFLOW_AUTO;
    } else {
        element->style.overflow = OVERFLOW_VISIBLE;
    }
}

// === COMPATIBILITY FUNCTIONS ===

void atomic_render(AtomicElement* element, SDL_Renderer* renderer) {
    // Redirect to the Optimum engine
    optimum_render_element(element, renderer);
}

// Remove the unused function to fix the warning
// static bool atomic_detect_circular_reference(AtomicElement* element, AtomicElement* potential_child) {
//     // Function removed to fix unused warning
// }

// === COMPATIBILITY FUNCTIONS ===

void atomic_set_background_image_path(AtomicElement* element, const char* path, SDL_Renderer* renderer) {
    if (!element || !path || !renderer) return;
    
    // Free existing path
    free(element->style.background_image_path);
    element->style.background_image_path = strdup(path);
    
    // Load texture from path (basic implementation)
    // In a full implementation, you would use the asset manager
    SDL_Surface* surface = IMG_Load(path);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (texture) {
            element->style.background_image = texture;
        }
    }
}

void atomic_set_background_size(AtomicElement* element, BackgroundSize size) {
    if (!element) return;
    element->style.background_size = size;
}

void atomic_set_background_repeat(AtomicElement* element, BackgroundRepeat repeat) {
    if (!element) return;
    element->style.background_repeat = repeat;
}

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
    
    SDL_Rect parent_rect = atomic_get_content_rect(element->parent);
    int center_x = parent_rect.x + (parent_rect.w - element->style.width) / 2;
    int center_y = parent_rect.y + (parent_rect.h - element->style.height) / 2;
    
    atomic_set_position(element, center_x, center_y);
}

void atomic_apply_flex_layout(AtomicElement* element) {
    // Redirect to the improved version
    atomic_apply_flex_layout_improved(element);
}

void atomic_set_font_ttf(AtomicElement* element, TTF_Font* font) {
    if (!element) return;
    element->style.text.ttf_font = font;
    element->style.text.font = font; // For compatibility
    element->style.font = font; // For compatibility
}

bool atomic_has_explicit_z_index(AtomicElement* element) {
    if (!element) return false;
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

bool atomic_has_class(AtomicElement* element, const char* class_name) {
    if (!element || !class_name || !element->class_name) return false;
    return strcmp(element->class_name, class_name) == 0;
}

SDL_Rect atomic_constrain_child_position(AtomicElement* parent, AtomicElement* child, int desired_x, int desired_y) {
    SDL_Rect constrained_rect = {desired_x, desired_y, child->style.width, child->style.height};
    
    if (!parent || !child) return constrained_rect;
    
    SDL_Rect parent_content = atomic_get_content_rect(parent);
    
    // Constrain to parent bounds
    if (constrained_rect.x < parent_content.x) {
        constrained_rect.x = parent_content.x;
    }
    if (constrained_rect.y < parent_content.y) {
        constrained_rect.y = parent_content.y;
    }
    if (constrained_rect.x + constrained_rect.w > parent_content.x + parent_content.w) {
        constrained_rect.x = parent_content.x + parent_content.w - constrained_rect.w;
    }
    if (constrained_rect.y + constrained_rect.h > parent_content.y + parent_content.h) {
        constrained_rect.y = parent_content.y + parent_content.h - constrained_rect.h;
    }
    
    return constrained_rect;
}

TTF_Font* atomic_get_default_font(void) {
    // Return the default font from the optimum engine or create one
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Arial.ttf",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
    
    static TTF_Font* default_font = NULL;
    if (!default_font) {
        for (int i = 0; i < 4; i++) {
            default_font = TTF_OpenFont(font_paths[i], 16);
            if (default_font) break;
        }
    }
    
    return default_font;
}

void atomic_set_focus_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*)) {
    if (!element) return;
    element->events.on_focus = handler;
}

// === END OF IMPLEMENTATIONS ===