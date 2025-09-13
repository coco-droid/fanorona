#ifndef ATOMIC_H
#define ATOMIC_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../../event/event.h"

// Énumérations pour les propriétés CSS-like
typedef enum {
    POSITION_STATIC,
    POSITION_RELATIVE,
    POSITION_ABSOLUTE,
    POSITION_FIXED
} PositionType;

typedef enum {
    DISPLAY_BLOCK,
    DISPLAY_INLINE,
    DISPLAY_NONE,
    DISPLAY_FLEX
} DisplayType;

// Structure pour les dimensions et espacements
typedef struct {
    int top, right, bottom, left;
} Spacing;

// Structure pour les propriétés de style CSS-like
typedef struct {
    // Positionnement
    PositionType position;
    int x, y;                    // Position
    int width, height;           // Dimensions
    
    // Marges et padding
    Spacing margin;
    Spacing padding;
    
    // Affichage
    DisplayType display;
    int z_index;
    bool visible;
    
    // Couleurs
    SDL_Color background_color;
    SDL_Color border_color;
    int border_width;
    
    // Opacité
    Uint8 opacity;
} AtomicStyle;

// Structure pour les événements
typedef struct {
    void (*on_click)(void* element, SDL_Event* event);
    void (*on_hover)(void* element, SDL_Event* event);
    void (*on_focus)(void* element, SDL_Event* event);
    void (*on_blur)(void* element, SDL_Event* event);
    void (*on_key_down)(void* element, SDL_Event* event);
    void (*on_key_up)(void* element, SDL_Event* event);
} AtomicEventHandlers;

// Structure pour le contenu de l'élément
typedef struct AtomicContent {
    char* text;                          // Texte à afficher
    SDL_Texture* texture;                // Texture/image à afficher
    struct AtomicElement** children;     // Éléments enfants
    int children_count;
    int children_capacity;
} AtomicContent;

// Structure principale de l'élément atomique
typedef struct AtomicElement {
    char* id;                           // Identifiant unique
    char* class_name;                   // Classe CSS-like
    
    AtomicStyle style;                  // Propriétés de style
    AtomicContent content;              // Contenu de l'élément
    AtomicEventHandlers events;         // Gestionnaires d'événements
    
    // État interne
    bool is_hovered;
    bool is_focused;
    bool is_pressed;
    
    // Données utilisateur
    void* user_data;
    
    // Référence à l'élément parent
    struct AtomicElement* parent;
    
    // Fonctions de rendu personnalisées
    void (*custom_render)(struct AtomicElement* element, SDL_Renderer* renderer);
    void (*custom_update)(struct AtomicElement* element, float delta_time);
} AtomicElement;

// Fonctions de création et destruction
AtomicElement* atomic_create(const char* id);
void atomic_destroy(AtomicElement* element);

// Fonctions de style
void atomic_set_position(AtomicElement* element, int x, int y);
void atomic_set_size(AtomicElement* element, int width, int height);
void atomic_set_margin(AtomicElement* element, int top, int right, int bottom, int left);
void atomic_set_padding(AtomicElement* element, int top, int right, int bottom, int left);
void atomic_set_background_color(AtomicElement* element, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void atomic_set_border(AtomicElement* element, int width, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void atomic_set_z_index(AtomicElement* element, int z_index);
void atomic_set_display(AtomicElement* element, DisplayType display);
void atomic_set_visibility(AtomicElement* element, bool visible);
void atomic_set_opacity(AtomicElement* element, Uint8 opacity);

// Fonctions de contenu
void atomic_set_text(AtomicElement* element, const char* text);
void atomic_set_texture(AtomicElement* element, SDL_Texture* texture);
void atomic_add_child(AtomicElement* parent, AtomicElement* child);
void atomic_remove_child(AtomicElement* parent, AtomicElement* child);

// Fonctions d'événements
void atomic_set_click_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*));
void atomic_set_hover_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*));
void atomic_set_focus_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*));

// Fonctions de rendu et mise à jour
void atomic_render(AtomicElement* element, SDL_Renderer* renderer);
void atomic_update(AtomicElement* element, float delta_time);

// Fonctions utilitaires
bool atomic_is_point_inside(AtomicElement* element, int x, int y);
SDL_Rect atomic_get_render_rect(AtomicElement* element);
SDL_Rect atomic_get_content_rect(AtomicElement* element);

// Fonctions d'événements système
void atomic_handle_event(AtomicElement* element, SDL_Event* event);
void atomic_register_with_event_manager(AtomicElement* element, EventManager* manager);
void atomic_unregister_from_event_manager(AtomicElement* element, EventManager* manager);

#endif // ATOMIC_H