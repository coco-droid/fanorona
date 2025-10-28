#include "event.h"
#include "../utils/log_console.h"
#include <stdlib.h>
#include <stdio.h>

// === FONCTIONS STATIQUES (DÉCLARATIONS AVANT UTILISATION) ===

// Vérifier si un point est dans les limites d'un élément
static bool is_point_in_element(EventElement* element, int x, int y) {
    return (x >= element->x && x < element->x + element->width &&
            y >= element->y && y < element->y + element->height);
}

// === FONCTIONS PUBLIQUES ===

// Créer un nouvel event manager
EventManager* event_manager_create(void) {
    EventManager* manager = (EventManager*)malloc(sizeof(EventManager));
    if (!manager) {
        //printf("Erreur: Impossible d'allouer la mémoire pour l'event manager\n");
        return NULL;
    }
    
    manager->elements = NULL;
    manager->running = true;
    
    return manager;
}

// Détruire l'event manager et libérer la mémoire
void event_manager_destroy(EventManager* manager) {
    if (!manager) return;
    
    event_manager_clear_all(manager);
    free(manager);
}

// Souscrire un élément aux événements
void event_manager_subscribe(EventManager* manager, int x, int y, int width, int height, 
                            int z_index, bool display, void (*callback)(SDL_Event*, void*), void* user_data) {
    if (!manager || !callback) return;
    
    EventElement* element = (EventElement*)malloc(sizeof(EventElement));
    if (!element) {
        //printf("Erreur: Impossible d'allouer la mémoire pour l'élément\n");
        return;
    }
    
    element->x = x;
    element->y = y;
    element->width = width;
    element->height = height;
    element->z_index = z_index;
    element->display = display;
    element->callback = callback;
    element->user_data = user_data;
    element->next = NULL;
    
    // Insérer dans la liste triée par z_index (plus élevé en premier)
    if (!manager->elements || manager->elements->z_index < z_index) {
        element->next = manager->elements;
        manager->elements = element;
    } else {
        EventElement* current = manager->elements;
        while (current->next && current->next->z_index >= z_index) {
            current = current->next;
        }
        element->next = current->next;
        current->next = element;
    }
}

// Désinscrire un élément spécifique
void event_manager_unsubscribe(EventManager* manager, void (*callback)(SDL_Event*, void*), void* user_data) {
    if (!manager || !callback) return;
    
    EventElement* current = manager->elements;
    EventElement* prev = NULL;
    
    while (current) {
        if (current->callback == callback && current->user_data == user_data) {
            if (prev) {
                prev->next = current->next;
            } else {
                manager->elements = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Effacer tous les éléments
void event_manager_clear_all(EventManager* manager) {
    if (!manager) return;
    
    EventElement* current = manager->elements;
    while (current) {
        EventElement* next = current->next;
        free(current);
        current = next;
    }
    manager->elements = NULL;
}

// Gérer un événement (CORRIGÉ)
void event_manager_handle_event(EventManager* manager, SDL_Event* event) {
    if (!manager || !event) return;
    
    // FERMETURE DE FENÊTRE (garder)
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_CLOSE) {
        log_console_write_event("EventManager", "WindowClose", "event.c", 
                               "[event.c] Window close detected", event->type);
        manager->running = false;
        return;
    }
    
    // QUIT (garder)
    if (event->type == SDL_QUIT) {
        log_console_write_event("EventManager", "Quit", "event.c", 
                               "[event.c] SDL_QUIT received", event->type);
        manager->running = false;
        return;
    }
    
    // NOUVEAU: Transmettre TOUS les événements (y compris clavier) aux éléments
    if (event->type == SDL_TEXTINPUT || event->type == SDL_KEYDOWN) {
        // Transmettre les événements clavier à TOUS les éléments
        EventElement* current = manager->elements;
        while (current) {
            if (current->display) {
                current->callback(event, current->user_data);
            }
            current = current->next;
        }
        return;
    }
    
    // Traiter SDL_MOUSEBUTTONDOWN comme des clics avec hit-testing
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        
        EventElement* current = manager->elements;
        
        while (current) {
            if (current->display && is_point_in_element(current, mouse_x, mouse_y)) {
                current->callback(event, current->user_data);
                return;
            }
            current = current->next;
        }
        return;
    }
    
    // Transmettre autres événements souris (motion, etc.)
    EventElement* current = manager->elements;
    while (current) {
        if (current->display) {
            current->callback(event, current->user_data);
        }
        current = current->next;
    }
}

// HITBOX VISUALIZATION SUPPORT
void event_manager_render_hitboxes(EventManager* manager, SDL_Renderer* renderer) {
    if (!manager || !renderer) return;
    
    // Déclarer la fonction externe pour la visualisation
    extern bool atomic_is_hitbox_visualization_enabled(void);
    
    if (!atomic_is_hitbox_visualization_enabled()) {
        return; // Visualisation désactivée
    }
    
    // Sauvegarder l'état du renderer
    SDL_BlendMode old_blend_mode;
    SDL_GetRenderDrawBlendMode(renderer, &old_blend_mode);
    Uint8 old_r, old_g, old_b, old_a;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
    
    // Activer le blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Parcourir tous les éléments enregistrés
    EventElement* current = manager->elements;
    
    while (current) {
        if (current->display) {
            SDL_Rect hitbox_rect = {current->x, current->y, current->width, current->height};
            
            // FOND ROUGE TRANSPARENT
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 30);
            SDL_RenderFillRect(renderer, &hitbox_rect);
            
            // BORDURE BLEUE OPAQUE (2px)
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 200);
            for (int i = 0; i < 2; i++) {
                SDL_Rect border_rect = {
                    hitbox_rect.x - i,
                    hitbox_rect.y - i,
                    hitbox_rect.w + 2 * i,
                    hitbox_rect.h + 2 * i
                };
                SDL_RenderDrawRect(renderer, &border_rect);
            }
        }
        current = current->next;
    }
    
    // Restaurer l'état du renderer
    SDL_SetRenderDrawBlendMode(renderer, old_blend_mode);
    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

// Définir l'état de fonctionnement
void event_manager_set_running(EventManager* manager, bool running) {
    if (manager) {
        manager->running = running;
    }
}

// Vérifier si le gestionnaire est en cours d'exécution
bool event_manager_is_running(EventManager* manager) {
    return manager ? manager->running : false;
}