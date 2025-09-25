#ifndef ATOMIC_H
#define ATOMIC_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../../event/event.h"
#include "../../utils/compat.h"

// Forward declaration for TTF_Font
typedef struct _TTF_Font TTF_Font;

// Énumérations pour les propriétés CSS-like
typedef enum {
    POSITION_STATIC,
    POSITION_RELATIVE,
    POSITION_ABSOLUTE,
    POSITION_FIXED
} PositionType;

// 🆕 AJOUT: Enum pour la gestion des débordements
typedef enum {
    OVERFLOW_VISIBLE,   // Les enfants peuvent déborder (comportement par défaut)
    OVERFLOW_HIDDEN,    // Les enfants sont contraints dans les limites du parent
    OVERFLOW_SCROLL,    // Débordement avec scroll (pour futures implémentations)
    OVERFLOW_AUTO       // Automatique selon le contenu
} OverflowType;

// 🆕 AJOUT: Enum pour align-self (centrage automatique par axe)
typedef enum {
    ALIGN_SELF_AUTO,        // Pas de centrage automatique
    ALIGN_SELF_CENTER_X,    // Centrage horizontal automatique
    ALIGN_SELF_CENTER_Y,    // Centrage vertical automatique
    ALIGN_SELF_CENTER_BOTH  // Centrage horizontal et vertical
} AlignSelf;

typedef enum {
    DISPLAY_BLOCK,
    DISPLAY_INLINE,
    DISPLAY_NONE,
    DISPLAY_FLEX
} DisplayType;

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT,
    ALIGN_TOP,
    ALIGN_MIDDLE,
    ALIGN_BOTTOM,
    ALIGN_STRETCH,
    ALIGN_START,    // Ajout pour compatibilité flexbox
    ALIGN_END       // Ajout pour compatibilité flexbox
} AlignType;

typedef enum {
    JUSTIFY_START,
    JUSTIFY_CENTER,
    JUSTIFY_END,
    JUSTIFY_SPACE_BETWEEN,
    JUSTIFY_SPACE_AROUND,
    JUSTIFY_SPACE_EVENLY
} JustifyType;

typedef enum {
    FLEX_DIRECTION_ROW,
    FLEX_DIRECTION_COLUMN,
    FLEX_DIRECTION_ROW_REVERSE,
    FLEX_DIRECTION_COLUMN_REVERSE
} FlexDirection;

typedef enum {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT,
    TEXT_ALIGN_JUSTIFY
} TextAlign;

// Énumérations pour les propriétés de background
typedef enum {
    BACKGROUND_SIZE_AUTO,
    BACKGROUND_SIZE_COVER,
    BACKGROUND_SIZE_CONTAIN,
    BACKGROUND_SIZE_STRETCH
} BackgroundSize;

typedef enum {
    BACKGROUND_REPEAT_NO_REPEAT,
    BACKGROUND_REPEAT_REPEAT,
    BACKGROUND_REPEAT_REPEAT_X,
    BACKGROUND_REPEAT_REPEAT_Y
} BackgroundRepeat;

// Structure pour les dimensions et espacements
typedef struct {
    int top, right, bottom, left;
} Spacing;

// Structure pour les propriétés flexbox
typedef struct {
    FlexDirection direction;
    JustifyType justify_content;
    AlignType align_items;
    AlignType align_content;
    bool wrap;
    int gap;
} FlexProperties;

// Structure pour les propriétés de texte
typedef struct {
    char* font_path;           // Chemin vers la police
    TTF_Font* font;            // 🔧 FIX: Ajouter le champ manquant
    TTF_Font* ttf_font;        // Police TTF chargée (pour compatibilité)
    int font_size;             // Taille de la police
    SDL_Color color;           // Couleur du texte
    TextAlign align;           // Alignement du texte
    bool bold;                 // Gras
    bool italic;               // Italique
} TextProperties;

// Structure pour les propriétés de positionnement
typedef struct {
    AlignType horizontal;      // left, center, right
    AlignType vertical;        // top, middle, bottom
    bool auto_center_x;        // Centrage automatique X
    bool auto_center_y;        // Centrage automatique Y
    AlignSelf align_self;      // 🆕 AJOUT: Centrage automatique par axe
} AlignmentProperties;

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
    
    // 🆕 AJOUT: Gestion des débordements
    OverflowType overflow;       // Contrôle du débordement des enfants
    
    // Couleurs
    SDL_Color background_color;
    SDL_Color border_color;
    int border_width;
    
    // Images de fond avec propriétés CSS
    SDL_Texture* background_image;
    char* background_image_path;
    BackgroundSize background_size;      // cover, contain, auto, stretch
    BackgroundRepeat background_repeat;  // no-repeat, repeat, repeat-x, repeat-y
    
    // Flexbox
    FlexProperties flex;
    
    // Alignement et positionnement
    AlignmentProperties alignment;
    
    // Texte
    TextProperties text;
    
    // Propriétés de texte étendues pour compatibilité
    int text_x, text_y;         // Position du texte
    TTF_Font* font;              // Police directe
    int font_size;               // Taille de police directe
    
    // Opacité
    Uint8 opacity;
} AtomicStyle;

// Structure pour les événements
typedef struct {
    void (*on_click)(void* element, SDL_Event* event);
    void (*on_hover)(void* element, SDL_Event* event);
    void (*on_unhover)(void* element, SDL_Event* event);  // 🆕 AJOUT: Handler pour unhover
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

// Fonctions d'images de fond
void atomic_set_background_image(AtomicElement* element, SDL_Texture* texture);
void atomic_set_background_image_path(AtomicElement* element, const char* path, SDL_Renderer* renderer);

// Fonctions pour les propriétés CSS background
void atomic_set_background_size(AtomicElement* element, BackgroundSize size);
void atomic_set_background_repeat(AtomicElement* element, BackgroundRepeat repeat);
void atomic_set_background_size_str(AtomicElement* element, const char* size);
void atomic_set_background_repeat_str(AtomicElement* element, const char* repeat);

// Fonctions de positionnement et alignement
void atomic_set_alignment(AtomicElement* element, AlignType horizontal, AlignType vertical);
void atomic_set_auto_center(AtomicElement* element, bool center_x, bool center_y);
void atomic_center_in_parent(AtomicElement* element);

// 🆕 NOUVELLES FONCTIONS pour align-self
void atomic_set_align_self(AtomicElement* element, AlignSelf align_self);
void atomic_set_align_self_center_x(AtomicElement* element);
void atomic_set_align_self_center_y(AtomicElement* element);
void atomic_set_align_self_center_both(AtomicElement* element);
void atomic_apply_align_self(AtomicElement* element);

// Fonctions flexbox
void atomic_set_flex_direction(AtomicElement* element, FlexDirection direction);
void atomic_set_justify_content(AtomicElement* element, JustifyType justify);
void atomic_set_align_items(AtomicElement* element, AlignType align);
void atomic_set_flex_wrap(AtomicElement* element, bool wrap);
void atomic_set_flex_gap(AtomicElement* element, int gap);
void atomic_apply_flex_layout(AtomicElement* element);

// Fonctions de texte et police
void atomic_set_font(AtomicElement* element, const char* font_path, int size);
void atomic_set_font_ttf(AtomicElement* element, TTF_Font* font);
void atomic_set_text_color(AtomicElement* element, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void atomic_set_text_align(AtomicElement* element, TextAlign align);
void atomic_set_text_style(AtomicElement* element, bool bold, bool italic);

// Fonctions de contenu
void atomic_set_text(AtomicElement* element, const char* text);
void atomic_set_texture(AtomicElement* element, SDL_Texture* texture);
void atomic_add_child(AtomicElement* parent, AtomicElement* child);
void atomic_remove_child(AtomicElement* parent, AtomicElement* child);

// Fonctions d'événements
void atomic_set_click_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*));
void atomic_set_hover_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*));
void atomic_set_unhover_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*));  // 🆕 AJOUT
void atomic_set_focus_handler(AtomicElement* element, void (*handler)(void*, SDL_Event*));

// Fonctions de rendu et mise à jour
void atomic_render(AtomicElement* element, SDL_Renderer* renderer);
void atomic_update(AtomicElement* element, float delta_time);

// 🆕 Police par défaut
TTF_Font* atomic_get_default_font(void);

// Fonctions utilitaires
bool atomic_is_point_inside(AtomicElement* element, int x, int y);
SDL_Rect atomic_get_render_rect(AtomicElement* element);
SDL_Rect atomic_get_content_rect(AtomicElement* element);

// Nouvelles fonctions pour le système de logs et z-index
bool atomic_has_explicit_z_index(AtomicElement* element);
int atomic_get_z_index(AtomicElement* element);
int atomic_get_width(AtomicElement* element);
int atomic_get_height(AtomicElement* element);

// Fonctions pour la gestion des positions de texte
void atomic_set_text_position(AtomicElement* element, int x, int y);
void atomic_set_text_font(AtomicElement* element, TTF_Font* font);
void atomic_set_text_size(AtomicElement* element, int size);

// Fonctions pour l'alignement
void atomic_set_align(AtomicElement* element, const char* horizontal, const char* vertical);

// Fonctions pour les classes CSS
void atomic_add_class(AtomicElement* element, const char* class_name);
bool atomic_has_class(AtomicElement* element, const char* class_name);

// Versions string pour la compatibilité UI
void atomic_set_text_align_str(AtomicElement* element, const char* align);
void atomic_set_text_color_rgba(AtomicElement* element, int r, int g, int b, int a);
void atomic_set_display_str(AtomicElement* element, const char* display);
void atomic_set_flex_direction_str(AtomicElement* element, const char* direction);
void atomic_set_justify_content_str(AtomicElement* element, const char* justify);
void atomic_set_align_items_str(AtomicElement* element, const char* align);

// Fonctions de debugging pour le texte
void atomic_debug_text_rendering(AtomicElement* element, const char* context);

// Fonctions d'événements système
void atomic_handle_event(AtomicElement* element, SDL_Event* event);
void atomic_register_with_event_manager(AtomicElement* element, EventManager* manager);
void atomic_unregister_from_event_manager(AtomicElement* element, EventManager* manager);

// 🆕 NOUVELLES FONCTIONS pour la gestion de l'overflow
void atomic_set_overflow(AtomicElement* element, OverflowType overflow);
void atomic_set_overflow_str(AtomicElement* element, const char* overflow);
SDL_Rect atomic_constrain_child_position(AtomicElement* parent, AtomicElement* child, int desired_x, int desired_y);
void atomic_apply_overflow_constraints(AtomicElement* parent);
bool atomic_is_child_overflowing(AtomicElement* parent, AtomicElement* child);

#endif // ATOMIC_H