#ifndef BUTTON_H
#define BUTTON_H

#include "native/atomic.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

// Énumérations pour les types de boutons
typedef enum {
    BUTTON_TYPE_PRIMARY,
    BUTTON_TYPE_SECONDARY,
    BUTTON_TYPE_SUCCESS,
    BUTTON_TYPE_WARNING,
    BUTTON_TYPE_DANGER
} ButtonType;

typedef enum {
    BUTTON_STATE_NORMAL,
    BUTTON_STATE_HOVERED,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_DISABLED
} ButtonState;

// Structure pour les données spécifiques au bouton
typedef struct {
    ButtonType type;
    ButtonState state;
    char* label;
    SDL_Texture* icon;
    
    // Couleurs pour les différents états
    SDL_Color normal_color;
    SDL_Color hover_color;
    SDL_Color pressed_color;
    SDL_Color disabled_color;
    
    // Animation
    float hover_animation;
    float press_animation;
    
    // Callback personnalisé
    void (*on_button_click)(void* button_data, void* user_data);
    void* callback_data;
} ButtonData;

// Structure principale du bouton
typedef struct Button {
    AtomicElement* element;  // Élément atomique de base
    ButtonData* data;        // Données spécifiques au bouton
} Button;

// Fonctions de création et destruction
Button* button_create(const char* id, const char* label);
void button_destroy(Button* button);

// Fonctions de configuration
void button_set_label(Button* button, const char* label);
void button_set_icon(Button* button, SDL_Texture* icon);
void button_set_type(Button* button, ButtonType type);
void button_set_position(Button* button, int x, int y);
void button_set_size(Button* button, int width, int height);
void button_set_enabled(Button* button, bool enabled);

// Fonctions d'événements
void button_set_click_callback(Button* button, void (*callback)(void*, void*), void* user_data);

// Fonctions de style
void button_set_colors(Button* button, SDL_Color normal, SDL_Color hover, SDL_Color pressed, SDL_Color disabled);
void button_apply_theme(Button* button, ButtonType type);

// Fonctions de rendu et mise à jour
void button_update(Button* button, float delta_time);
void button_render(Button* button, SDL_Renderer* renderer);

// Fonctions utilitaires
ButtonState button_get_state(Button* button);
AtomicElement* button_get_atomic_element(Button* button);

// Fonctions de gestion des événements
void button_register_events(Button* button, EventManager* manager);
void button_unregister_events(Button* button, EventManager* manager);

#endif // BUTTON_H