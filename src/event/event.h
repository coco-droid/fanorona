#ifndef EVENT_H
#define EVENT_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Structure pour représenter un élément pouvant recevoir des événements
typedef struct EventElement {
    int x, y;           // Position
    int width, height;  // Dimensions
    int z_index;        // Z-index pour le tri
    bool display;       // Si l'élément est affiché
    void (*callback)(SDL_Event*, void*); // Fonction de callback
    void* user_data;    // Données utilisateur
    struct EventElement* next; // Liste chaînée
} EventElement;

// Structure pour l'event manager
typedef struct EventManager {
    EventElement* elements; // Liste des éléments
    bool running;          // État du gestionnaire
} EventManager;

// Fonctions de l'event manager
EventManager* event_manager_create(void);
void event_manager_destroy(EventManager* manager);
void event_manager_subscribe(EventManager* manager, int x, int y, int width, int height, 
                            int z_index, bool display, void (*callback)(SDL_Event*, void*), void* user_data);
void event_manager_unsubscribe(EventManager* manager, void (*callback)(SDL_Event*, void*), void* user_data);
void event_manager_clear_all(EventManager* manager);
void event_manager_handle_event(EventManager* manager, SDL_Event* event);
void event_manager_set_running(EventManager* manager, bool running);
bool event_manager_is_running(EventManager* manager);

// 🆕 HITBOX VISUALIZATION SUPPORT
// Fonction pour permettre le rendu des hitboxes de tous les éléments enregistrés
void event_manager_render_hitboxes(EventManager* manager, SDL_Renderer* renderer);

#endif // EVENT_H