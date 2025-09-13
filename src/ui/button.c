#include "button.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// Couleurs de thème par défaut
static const SDL_Color BUTTON_THEMES[][4] = {
    // PRIMARY: normal, hover, pressed, disabled
    {{70, 130, 180, 255}, {100, 149, 237, 255}, {65, 105, 225, 255}, {169, 169, 169, 255}},
    // SECONDARY: normal, hover, pressed, disabled
    {{108, 117, 125, 255}, {134, 142, 150, 255}, {86, 95, 103, 255}, {169, 169, 169, 255}},
    // SUCCESS: normal, hover, pressed, disabled
    {{40, 167, 69, 255}, {57, 181, 85, 255}, {32, 134, 55, 255}, {169, 169, 169, 255}},
    // WARNING: normal, hover, pressed, disabled
    {{255, 193, 7, 255}, {255, 205, 57, 255}, {204, 154, 5, 255}, {169, 169, 169, 255}},
    // DANGER: normal, hover, pressed, disabled
    {{220, 53, 69, 255}, {231, 76, 60, 255}, {176, 42, 55, 255}, {169, 169, 169, 255}}
};

// Fonctions callback internes
static void button_on_click(void* element, SDL_Event* event);
static void button_on_hover(void* element, SDL_Event* event);
static void button_on_blur(void* element, SDL_Event* event);
static void button_custom_render(AtomicElement* element, SDL_Renderer* renderer);
static void button_custom_update(AtomicElement* element, float delta_time);

// Créer un nouveau bouton
Button* button_create(const char* id, const char* label) {
    Button* button = (Button*)calloc(1, sizeof(Button));
    if (!button) {
        printf("Erreur: Impossible d'allouer la mémoire pour le bouton\n");
        return NULL;
    }
    
    // Créer l'élément atomique
    button->element = atomic_create(id);
    if (!button->element) {
        free(button);
        return NULL;
    }
    
    // Créer les données du bouton
    button->data = (ButtonData*)calloc(1, sizeof(ButtonData));
    if (!button->data) {
        atomic_destroy(button->element);
        free(button);
        return NULL;
    }
    
    // Initialiser les données
    button->data->type = BUTTON_TYPE_PRIMARY;
    button->data->state = BUTTON_STATE_NORMAL;
    button->data->label = label ? strdup(label) : NULL;
    button->data->hover_animation = 0.0f;
    button->data->press_animation = 0.0f;
    
    // Appliquer le thème par défaut
    button_apply_theme(button, BUTTON_TYPE_PRIMARY);
    
    // Configurer l'élément atomique
    atomic_set_size(button->element, 120, 40);
    atomic_set_padding(button->element, 8, 16, 8, 16);
    atomic_set_border(button->element, 1, 0, 0, 0, 0);
    
    // Associer les données utilisateur
    button->element->user_data = button;
    
    // Configurer les gestionnaires d'événements
    atomic_set_click_handler(button->element, button_on_click);
    atomic_set_hover_handler(button->element, button_on_hover);
    button->element->events.on_blur = button_on_blur;
    
    // Configurer le rendu personnalisé
    button->element->custom_render = button_custom_render;
    button->element->custom_update = button_custom_update;
    
    return button;
}

// Détruire un bouton
void button_destroy(Button* button) {
    if (!button) return;
    
    if (button->data) {
        free(button->data->label);
        free(button->data);
    }
    
    atomic_destroy(button->element);
    free(button);
}

// === FONCTIONS DE CONFIGURATION ===

void button_set_label(Button* button, const char* label) {
    if (!button || !button->data) return;
    
    free(button->data->label);
    button->data->label = label ? strdup(label) : NULL;
}

void button_set_icon(Button* button, SDL_Texture* icon) {
    if (!button || !button->data) return;
    button->data->icon = icon;
}

void button_set_type(Button* button, ButtonType type) {
    if (!button || !button->data) return;
    button->data->type = type;
    button_apply_theme(button, type);
}

void button_set_position(Button* button, int x, int y) {
    if (!button || !button->element) return;
    atomic_set_position(button->element, x, y);
}

void button_set_size(Button* button, int width, int height) {
    if (!button || !button->element) return;
    atomic_set_size(button->element, width, height);
}

void button_set_enabled(Button* button, bool enabled) {
    if (!button || !button->data) return;
    button->data->state = enabled ? BUTTON_STATE_NORMAL : BUTTON_STATE_DISABLED;
    atomic_set_visibility(button->element, enabled);
}

// === FONCTIONS D'ÉVÉNEMENTS ===

void button_set_click_callback(Button* button, void (*callback)(void*, void*), void* user_data) {
    if (!button || !button->data) return;
    button->data->on_button_click = callback;
    button->data->callback_data = user_data;
}

// === FONCTIONS DE STYLE ===

void button_set_colors(Button* button, SDL_Color normal, SDL_Color hover, SDL_Color pressed, SDL_Color disabled) {
    if (!button || !button->data) return;
    
    button->data->normal_color = normal;
    button->data->hover_color = hover;
    button->data->pressed_color = pressed;
    button->data->disabled_color = disabled;
}

void button_apply_theme(Button* button, ButtonType type) {
    if (!button || !button->data || type < 0 || type >= 5) return;
    
    const SDL_Color* colors = BUTTON_THEMES[type];
    button_set_colors(button, colors[0], colors[1], colors[2], colors[3]);
}

// === FONCTIONS DE RENDU ET MISE À JOUR ===

void button_update(Button* button, float delta_time) {
    if (!button) return;
    atomic_update(button->element, delta_time);
}

void button_render(Button* button, SDL_Renderer* renderer) {
    if (!button) return;
    atomic_render(button->element, renderer);
}

// === FONCTIONS UTILITAIRES ===

ButtonState button_get_state(Button* button) {
    return button && button->data ? button->data->state : BUTTON_STATE_DISABLED;
}

AtomicElement* button_get_atomic_element(Button* button) {
    return button ? button->element : NULL;
}

// === GESTION DES ÉVÉNEMENTS ===

void button_register_events(Button* button, EventManager* manager) {
    if (!button || !button->element || !manager) return;
    atomic_register_with_event_manager(button->element, manager);
}

void button_unregister_events(Button* button, EventManager* manager) {
    if (!button || !button->element || !manager) return;
    atomic_unregister_from_event_manager(button->element, manager);
}

// === CALLBACKS INTERNES ===

static void button_on_click(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    if (!atomic_element || !atomic_element->user_data) return;
    
    Button* button = (Button*)atomic_element->user_data;
    if (button->data->state == BUTTON_STATE_DISABLED) return;
    
    button->data->state = BUTTON_STATE_PRESSED;
    button->data->press_animation = 1.0f;
    
    // Appeler le callback personnalisé
    if (button->data->on_button_click) {
        button->data->on_button_click(button, button->data->callback_data);
    }
    
    (void)event; // Éviter l'avertissement
}

static void button_on_hover(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    if (!atomic_element || !atomic_element->user_data) return;
    
    Button* button = (Button*)atomic_element->user_data;
    if (button->data->state == BUTTON_STATE_DISABLED) return;
    
    if (button->data->state != BUTTON_STATE_PRESSED) {
        button->data->state = BUTTON_STATE_HOVERED;
    }
    
    (void)event; // Éviter l'avertissement
}

static void button_on_blur(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    if (!atomic_element || !atomic_element->user_data) return;
    
    Button* button = (Button*)atomic_element->user_data;
    if (button->data->state == BUTTON_STATE_DISABLED) return;
    
    button->data->state = BUTTON_STATE_NORMAL;
    
    (void)event; // Éviter l'avertissement
}

// === RENDU PERSONNALISÉ ===

static void button_custom_render(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !element->user_data || !renderer) return;
    
    Button* button = (Button*)element->user_data;
    SDL_Rect content_rect = atomic_get_content_rect(element);
    
    // Choisir la couleur selon l'état
    SDL_Color current_color;
    switch (button->data->state) {
        case BUTTON_STATE_HOVERED:
            current_color = button->data->hover_color;
            break;
        case BUTTON_STATE_PRESSED:
            current_color = button->data->pressed_color;
            break;
        case BUTTON_STATE_DISABLED:
            current_color = button->data->disabled_color;
            break;
        default:
            current_color = button->data->normal_color;
            break;
    }
    
    // Appliquer l'animation
    if (button->data->hover_animation > 0.0f) {
        float factor = button->data->hover_animation;
        current_color.r = (Uint8)(current_color.r * (1.0f - factor) + button->data->hover_color.r * factor);
        current_color.g = (Uint8)(current_color.g * (1.0f - factor) + button->data->hover_color.g * factor);
        current_color.b = (Uint8)(current_color.b * (1.0f - factor) + button->data->hover_color.b * factor);
    }
    
    // Mettre à jour la couleur de fond de l'élément atomique
    atomic_set_background_color(element, current_color.r, current_color.g, current_color.b, current_color.a);
    
    // Dessiner l'icône si présente
    if (button->data->icon) {
        SDL_Rect icon_rect = {
            content_rect.x,
            content_rect.y + (content_rect.h - 20) / 2,
            20, 20
        };
        SDL_RenderCopy(renderer, button->data->icon, NULL, &icon_rect);
    }
    
    // Dessiner le label (représenté par un rectangle pour l'instant)
    if (button->data->label) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanc pour le texte
        
        int text_width = (int)strlen(button->data->label) * 8; // Approximation
        int text_height = 12;
        
        SDL_Rect text_rect = {
            content_rect.x + (content_rect.w - text_width) / 2,
            content_rect.y + (content_rect.h - text_height) / 2,
            text_width,
            text_height
        };
        
        // Dessiner des rectangles pour simuler les lettres
        for (int i = 0; i < (int)strlen(button->data->label); i++) {
            SDL_Rect letter_rect = {
                text_rect.x + i * 8,
                text_rect.y,
                6,
                text_height
            };
            SDL_RenderFillRect(renderer, &letter_rect);
        }
    }
}

// === MISE À JOUR PERSONNALISÉE ===

static void button_custom_update(AtomicElement* element, float delta_time) {
    if (!element || !element->user_data) return;
    
    Button* button = (Button*)element->user_data;
    
    // Animation de hover
    if (button->data->state == BUTTON_STATE_HOVERED) {
        button->data->hover_animation += delta_time * 3.0f;
        if (button->data->hover_animation > 1.0f) {
            button->data->hover_animation = 1.0f;
        }
    } else {
        button->data->hover_animation -= delta_time * 3.0f;
        if (button->data->hover_animation < 0.0f) {
            button->data->hover_animation = 0.0f;
        }
    }
    
    // Animation de press
    if (button->data->press_animation > 0.0f) {
        button->data->press_animation -= delta_time * 4.0f;
        if (button->data->press_animation < 0.0f) {
            button->data->press_animation = 0.0f;
        }
    }
}