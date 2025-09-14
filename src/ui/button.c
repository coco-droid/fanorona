#include "button.h"
#include "./native/atomic.h"
#include "ui_components.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

// Simple button implementation - click only
Button* button_create(const char* text) {
    Button* button = (Button*)malloc(sizeof(Button));
    if (!button) return NULL;
    
    button->data = (ButtonData*)malloc(sizeof(ButtonData));
    if (!button->data) {
        free(button);
        return NULL;
    }
    
    // Initialize with defaults based on existing structure
    // Don't use text field since it doesn't exist in ButtonData
    button->data->type = 0; // Use 0 for default type
    button->data->state = 0; // Use 0 for normal state
    
    (void)text; // Ignore text parameter for now
    
    return button;
}

void button_destroy(Button* button) {
    if (!button) return;
    
    if (button->data) {
        // Don't try to free text field since it doesn't exist
        free(button->data);
    }
    free(button);
}

void button_set_type(Button* button, ButtonType type) {
    if (button && button->data) {
        button->data->type = type;
    }
}

// Simple render function
static void button_custom_render(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer) return;
    
    Button* button = (Button*)element->user_data;
    if (!button || !button->data) return;
    
    // Get button color based on type
    SDL_Color bg_color;
    switch (button->data->type) {
        case 1: // SUCCESS
            bg_color = (SDL_Color){40, 167, 69, 255};
            break;
        case 2: // DANGER
            bg_color = (SDL_Color){220, 53, 69, 255};
            break;
        default:
            bg_color = (SDL_Color){108, 117, 125, 255};
            break;
    }
    
    // Use atomic element's position and size properties
    SDL_Rect rect = {
        atomic_get_x(element), 
        atomic_get_y(element), 
        atomic_get_width(element), 
        atomic_get_height(element)
    };
    
    // Draw button rectangle
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

// Simple update function
static void button_custom_update(AtomicElement* element, float delta_time) {
    (void)element;
    (void)delta_time;
    // No animations - just empty
}

// Register button events - simplified
void button_register_events(Button* button, EventManager* event_manager) {
    (void)button;
    (void)event_manager;
    // Simplified - no event handling for now
}

// Stub functions to match header
void button_set_colors(Button* button, SDL_Color normal, SDL_Color hover, SDL_Color pressed, SDL_Color disabled) {
    (void)button; (void)normal; (void)hover; (void)pressed; (void)disabled;
}

void button_set_border_radius(Button* button, int radius) {
    (void)button; (void)radius;
}

void button_set_font(Button* button, const char* font_path, int font_size) {
    (void)button; (void)font_path; (void)font_size;
}

void button_set_text_color(Button* button, SDL_Color color) {
    (void)button; (void)color;
}

void button_enable(Button* button) {
    (void)button;
}

void button_disable(Button* button) {
    (void)button;
}

bool button_is_enabled(const Button* button) {
    (void)button;
    return true;
}

void button_simulate_click(Button* button) {
    (void)button;
    // Simplified - no click simulation for now
}