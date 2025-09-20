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
        printf("Erreur: Impossible d'allouer la mémoire pour l'event manager\n");
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
        printf("Erreur: Impossible d'allouer la mémoire pour l'élément\n");
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
    
    static int event_counter = 0;
    event_counter++;
    
    // 🔧 FERMETURE DE FENÊTRE (garder)
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_CLOSE) {
        log_console_write_event("EventManager", "WindowClose", "event.c", 
                               "[event.c] Window close detected", event->type);
        manager->running = false;
        return;
    }
    
    // 🔧 QUIT (garder)
    if (event->type == SDL_QUIT) {
        log_console_write_event("EventManager", "Quit", "event.c", 
                               "[event.c] SDL_QUIT received", event->type);
        manager->running = false;
        return;
    }
    
    // Traiter SEULEMENT SDL_MOUSEBUTTONDOWN comme des clics
    if (event->type == SDL_MOUSEBUTTONDOWN) { // 1024 SEULEMENT
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        
        log_console_write("EventManager", "Click", "event.c", 
                         "[event.c] Mouse click DOWN detected (code=1024=SDL_MOUSEBUTTONDOWN)");
        
        // 🆕 DEBUG: Compter les éléments avant hit testing
        int element_count = 0;
        EventElement* count_current = manager->elements;
        while (count_current) {
            element_count++;
            count_current = count_current->next;
        }
        
        char debug_message[256];
        snprintf(debug_message, sizeof(debug_message), 
                "[event.c] Hit testing against %d registered elements at (%d,%d)", 
                element_count, mouse_x, mouse_y);
        log_console_write("EventManager", "HitTesting", "event.c", debug_message);
        
        EventElement* current = manager->elements;
        int element_index = 0;
        
        while (current) {
            element_index++;
            
            // 🆕 DEBUG: Log chaque test avec coordonnées détaillées
            char test_message[512];
            snprintf(test_message, sizeof(test_message), 
                    "[event.c] Testing element #%d: bounds(%d,%d,%dx%d) display=%s z=%d", 
                    element_index, current->x, current->y, current->width, current->height,
                    current->display ? "true" : "false", current->z_index);
            log_console_write("EventManager", "ElementTest", "event.c", test_message);
            
            if (current->display && is_point_in_element(current, mouse_x, mouse_y)) {
                log_console_write("EventManager", "HitDetected", "event.c", 
                                 "[event.c] 🎯 Element hit - calling callback");
                
                current->callback(event, current->user_data);
                
                log_console_write("EventManager", "CallbackDone", "event.c", 
                                 "[event.c] ✅ Callback executed");
                return;
            }
            current = current->next;
        }
        
        // LOG si aucun hit
        log_console_write("EventManager", "NoHit", "event.c", 
                         "[event.c] ❌ No elements hit");
        return;
    }
    
    // 🔧 GESTION SILENCIEUSE DES AUTRES ÉVÉNEMENTS
    // Transmettre TOUS les événements aux éléments (y compris mousemotion, mouseup, etc.)
    EventElement* current = manager->elements;
    while (current) {
        if (current->display) {
            current->callback(event, current->user_data);
        }
        current = current->next;
    }
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