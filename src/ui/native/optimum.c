#define _POSIX_C_SOURCE 200809L
#include "optimum.h"
#include "atomic.h"
#include "../ui_tree.h"
#include "../../utils/asset_manager.h"
#include "../../utils/log_console.h"
#include "../../event/event.h"  // 🔧 FIX: Ajouter l'include pour EventManager
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// === OPTIMUM RENDER ENGINE ===

// Police par défaut globale pour le moteur
static TTF_Font* optimum_default_font = NULL;

// === FONCTIONS STATIQUES INTERNES ===

// Calculer le rectangle de destination pour l'image de fond selon background-size
static SDL_Rect calculate_background_dest_rect(AtomicElement* element, SDL_Texture* texture) {
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

// Obtenir la police par défaut du moteur Optimum
static TTF_Font* optimum_get_default_font(void) {
    if (!optimum_default_font) {
        const char* font_paths[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/System/Library/Fonts/Arial.ttf",
            "C:\\Windows\\Fonts\\arial.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf"
        };
        
        for (int i = 0; i < 6; i++) {
            optimum_default_font = TTF_OpenFont(font_paths[i], 16);
            if (optimum_default_font) {
                ////printf("✅ [OPTIMUM] Police par défaut chargée: %s\n", font_paths[i]);
                break;
            }
        }
        
        if (!optimum_default_font) {
            ////printf("❌ [OPTIMUM] ERREUR: Aucune police système trouvée!\n");
        }
    }
    
    return optimum_default_font;
}

// === MOTEUR DE RENDU PRINCIPAL ===

void optimum_render_element(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer || !element->style.visible || element->style.display == DISPLAY_NONE) {
        return;
    }
    
    // 🔧 FIX: Recalculer le layout AVANT le rendu pour synchroniser positions
    atomic_calculate_layout(element);
    
    // Calculer les rectangles de rendu
    SDL_Rect render_rect = atomic_get_render_rect(element);
    SDL_Rect content_rect = atomic_get_content_rect(element);
    
    // Sauvegarder l'état du renderer
    SDL_BlendMode old_blend_mode;
    SDL_GetRenderDrawBlendMode(renderer, &old_blend_mode);
    
    Uint8 old_r, old_g, old_b, old_a;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
    
    // Activer le blending pour les transparences
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // === RENDU DU BACKGROUND ===
    if (element->style.background_color.a > 0) {
        SDL_SetRenderDrawColor(renderer, 
                             element->style.background_color.r,
                             element->style.background_color.g,
                             element->style.background_color.b,
                             (Uint8)((element->style.background_color.a * element->style.opacity) / 255));
        SDL_RenderFillRect(renderer, &render_rect);
    }
    
    // === RENDU DE L'IMAGE DE FOND ===
    if (element->style.background_image) {
        Uint8 texture_alpha = element->style.opacity;
        SDL_SetTextureAlphaMod(element->style.background_image, texture_alpha);
        
        SDL_Rect bg_dest = calculate_background_dest_rect(element, element->style.background_image);
        
        // Gérer background-repeat
        if (element->style.background_repeat == BACKGROUND_REPEAT_NO_REPEAT) {
            SDL_RenderCopy(renderer, element->style.background_image, NULL, &bg_dest);
        } else {
            // TODO: Implémenter les autres modes de repeat
            SDL_RenderCopy(renderer, element->style.background_image, NULL, &bg_dest);
        }
        
        SDL_SetTextureAlphaMod(element->style.background_image, 255);
    }
    
    // === RENDU DE LA BORDURE ===
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
    
    // === RENDU DE LA TEXTURE (IMAGES) ===
    if (element->content.texture) {
        SDL_SetTextureAlphaMod(element->content.texture, element->style.opacity);
        SDL_RenderCopy(renderer, element->content.texture, NULL, &content_rect);
        SDL_SetTextureAlphaMod(element->content.texture, 255);
    }
    
    // === RENDU DU TEXTE ===
    if (element->content.text && strlen(element->content.text) > 0) {
        TTF_Font* font = element->style.font;
        if (!font) {
            font = optimum_get_default_font();
            if (!font) {
                /*printf("⚠️ [OPTIMUM] No font available for text rendering of '%s'\n", 
                       element->id ? element->id : "NoID");*/
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
            //printf("⚠️ [OPTIMUM] Failed to create text surface: %s\n", TTF_GetError());
            goto skip_text_rendering;
        }
        
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        if (!text_texture) {
            SDL_FreeSurface(text_surface);
            //printf("⚠️ [OPTIMUM] Failed to create text texture: %s\n", SDL_GetError());
            goto skip_text_rendering;
        }
        
        // Calculer la position du texte dans le content_rect
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
    
    // Rendre les enfants récursivement
    for (int i = 0; i < element->content.children_count; i++) {
        AtomicElement* child = element->content.children[i];
        if (child) {
            optimum_render_element(child, renderer);
        }
    }
    
    // Restaurer l'état du renderer
    SDL_SetRenderDrawBlendMode(renderer, old_blend_mode);
    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

// === API PUBLIQUE DU MOTEUR OPTIMUM ===

void optimum_render_ui_tree(UITree* tree, SDL_Renderer* renderer) {
    if (!tree || !tree->root || !renderer) {
        return;
    }
    
    // Rendre l'arbre UI complet en commençant par la racine
    optimum_render_element(tree->root->element, renderer);
    
    // 🆕 RENDU DES HITBOXES après le rendu normal
    if (tree->event_manager) {
        event_manager_render_hitboxes(tree->event_manager, renderer);
    }
}

void optimum_cleanup(void) {
    if (optimum_default_font) {
        TTF_CloseFont(optimum_default_font);
        optimum_default_font = NULL;
        //printf("🧹 [OPTIMUM] Default font cleaned up\n");
    }
}

// === FONCTIONS DE DEBUG ET PROFILING ===

void optimum_debug_render_bounds(AtomicElement* element, SDL_Renderer* renderer, bool show_content_rect) {
    if (!element || !renderer) return;
    
    SDL_Rect render_rect = atomic_get_render_rect(element);
    SDL_Rect content_rect = atomic_get_content_rect(element);
    
    // Sauvegarder l'état
    Uint8 old_r, old_g, old_b, old_a;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
    
    // Dessiner le rectangle de rendu en rouge
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
    SDL_RenderDrawRect(renderer, &render_rect);
    
    if (show_content_rect) {
        // Dessiner le rectangle de contenu en bleu
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
        SDL_RenderDrawRect(renderer, &content_rect);
    }
    
    // Restaurer l'état
    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

void optimum_render_performance_info(SDL_Renderer* renderer, int elements_rendered, float render_time_ms) {
    // TODO: Implémenter l'affichage des informations de performance
    (void)renderer; // Éviter le warning unused parameter
    (void)elements_rendered; // Éviter le warning unused parameter
    (void)render_time_ms; // Éviter le warning unused parameter
    //printf("🎯 [OPTIMUM] Rendered %d elements in %.2fms\n", elements_rendered, render_time_ms);
}

// === SYSTÈME DE SYNCHRONISATION POST-CALCULS ===

/**
 * Synchroniser récursivement les hitboxes d'un élément et ses enfants
 * APPELÉ APRÈS tous les calculs de layout pour avoir les positions finales
 */
void optimum_sync_element_hitbox_recursive(UINode* node, EventManager* manager) {
    if (!node || !node->element || !manager) return;
    
    // Vérifier si l'élément a des handlers d'événements
    bool has_handlers = (node->element->events.on_click != NULL ||
                        node->element->events.on_hover != NULL ||
                        node->element->events.on_unhover != NULL);
    
    if (has_handlers) {
        // Synchroniser la position finale calculée avec l'EventManager
        atomic_sync_event_manager_position(node->element, manager);
    }
    
    // Parcourir récursivement tous les enfants
    for (int i = 0; i < node->children_count; i++) {
        optimum_sync_element_hitbox_recursive(node->children[i], manager);
    }
}

/**
 * Synchroniser toutes les hitboxes après les calculs de layout complets
 * POINT D'ENTRÉE PRINCIPAL pour la synchronisation post-calculs
 */
void optimum_sync_all_hitboxes_post_layout(UITree* tree) {
    if (!tree || !tree->root || !tree->event_manager) {
        
        return;
    }
    
   
    
    // Parcourir récursivement l'arbre entier et synchroniser toutes les hitboxes
    optimum_sync_element_hitbox_recursive(tree->root, tree->event_manager);
    
   
}
