#include "event.h"
#include <stdlib.h>
#include <stdio.h>

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

// Vérifier si un point est dans les limites d'un élément
static bool is_point_in_element(EventElement* element, int x, int y) {
    return (x >= element->x && x < element->x + element->width &&
            y >= element->y && y < element->y + element->height);
}

// Gérer un événement
void event_manager_handle_event(EventManager* manager, SDL_Event* event) {
    if (!manager || !event) return;
    
    // Gérer l'événement de fermeture
    if (event->type == SDL_QUIT) {
        manager->running = false;
        return;
    }
    
    // Pour les événements de souris, vérifier les collisions
    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP || 
        event->type == SDL_MOUSEMOTION) {
        
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        
        // Parcourir les éléments par ordre de z_index (plus élevé en premier)
        EventElement* current = manager->elements;
        while (current) {
            if (current->display && is_point_in_element(current, mouse_x, mouse_y)) {
                // Appeler le callback et arrêter la propagation
                current->callback(event, current->user_data);
                return;
            }
            current = current->next;
        }
    }
    
    // Pour les autres événements (clavier, etc.), appeler tous les callbacks
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