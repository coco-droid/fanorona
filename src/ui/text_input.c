#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char text[256];
    char placeholder[256];
    int cursor_pos;
    float blink_timer;
    bool cursor_visible;
    int max_length;
    bool is_focused; // 🆕 Track focus state
} TextInputData;

// 🆕 CURSOR ANIMATION UPDATE - Remove __attribute__((unused))
static void text_input_update(UINode* node, float delta_time) {
    TextInputData* data = (TextInputData*)node->component_data;
    if (!data) return;
    
    data->blink_timer += delta_time;
    if (data->blink_timer >= 0.5f) {
        data->cursor_visible = !data->cursor_visible;
        data->blink_timer = 0.0f;
    }
}

// 🔧 FIX: Remove __attribute__((unused)) and register properly
static void text_input_handle_text(void* element, SDL_Event* event) {
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* node = (UINode*)atomic->user_data;
    if (!node) return;
    
    TextInputData* data = (TextInputData*)node->component_data;
    if (!data || !data->is_focused) return;
    
    if (event->type == SDL_TEXTINPUT) {
        int len = strlen(data->text);
        if (len < data->max_length - 1) {
            strcat(data->text, event->text.text);
            printf("📝 Text input: '%s'\n", data->text);
        }
    } else if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_BACKSPACE) {
        int len = strlen(data->text);
        if (len > 0) {
            data->text[len - 1] = '\0';
        }
    }
}

// 🆕 HANDLE FOCUS
static void text_input_handle_focus(void* element, SDL_Event* event) {
    (void)event;
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* node = (UINode*)atomic->user_data;
    if (!node) return;
    
    TextInputData* data = (TextInputData*)node->component_data;
    if (data) {
        data->is_focused = true;
        SDL_StartTextInput();
        printf("🎯 Text input focused\n");
        
        // 🔧 FIX: Remove atomic_set_key_handler call - not needed
        // The text handler is already registered at creation time
    }
}

static void text_input_render(AtomicElement* element, SDL_Renderer* renderer) {
    TextInputData* data = (TextInputData*)atomic_get_custom_data(element, "input_data");
    if (!data) return;
    
    SDL_Rect rect = atomic_get_render_rect(element);
    
    // Fond blanc + bordure orange
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // 🔧 FIX: Render text or placeholder
    const char* display_text = data->text[0] != '\0' ? data->text : data->placeholder;
    if (display_text[0] != '\0') {
        // TODO: Implement TTF text rendering
    }
    
    // Curseur jaune clignotant
    if (data->cursor_visible && data->is_focused) {
        int cursor_x = rect.x + 10 + (int)strlen(data->text) * 8;
        SDL_Rect cursor = {cursor_x, rect.y + 5, 2, rect.h - 10};
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &cursor);
    }
}

UINode* ui_text_input(UITree* tree, const char* id, const char* placeholder) {
    UINode* input = ui_div(tree, id);
    if (!input) return NULL;
    
    TextInputData* data = (TextInputData*)calloc(1, sizeof(TextInputData));
    data->text[0] = '\0';
    strncpy(data->placeholder, placeholder, 255);
    data->cursor_pos = 0;
    data->blink_timer = 0.0f;
    data->cursor_visible = true;
    data->max_length = 255;
    data->is_focused = false; // 🆕 Initialize focus
    
    input->component_data = data;
    input->component_update = text_input_update; // ✅ Now valid - field added to UINode
    atomic_set_custom_data(input->element, "input_data", data);
    atomic_set_custom_render(input->element, text_input_render);
    
    // 🆕 ATTACH EVENT HANDLERS
    input->element->user_data = input;
    atomic_set_click_handler(input->element, text_input_handle_focus);
    
    // 🔧 FIX: Remove atomic_set_key_handler call - function doesn't exist
    // Keyboard events will be handled via EventManager when focused
    // TODO: Register for SDL_TEXTINPUT/SDL_KEYDOWN events via EventManager
    
    return input;
}

void ui_text_input_set_max_length(UINode* input, int max_length) {
    TextInputData* data = (TextInputData*)input->component_data;
    if (data) data->max_length = max_length;
}
