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
    
    // 🔧 FIX CRITIQUE: Vérifier qu'on ne traite QU'UNE FOIS par événement
    static SDL_Event last_event = {0};
    if (event->type == last_event.type && 
        event->type == SDL_TEXTINPUT &&
        strcmp(event->text.text, last_event.text.text) == 0) {
        // Même événement déjà traité
        return;
    }
    last_event = *event;
    
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

// 🆕 HANDLE FOCUS - Re-add missing function
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
    
    // 🔧 FIX: Rendu du texte AVEC TTF (priorité au texte saisi)
    const char* display_text = (data->text[0] != '\0') ? data->text : data->placeholder;
    
    if (display_text && display_text[0] != '\0') {
        extern TTF_Font* atomic_get_default_font(void);
        TTF_Font* font = atomic_get_default_font();
        
        if (font) {
            // 🔧 FIX: Noir pour texte saisi, gris pour placeholder
            SDL_Color text_color = (data->text[0] != '\0') ? 
                (SDL_Color){0, 0, 0, 255} : (SDL_Color){128, 128, 128, 255};
            
            SDL_Surface* surface = TTF_RenderText_Blended(font, display_text, text_color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect text_rect = {
                        rect.x + 10, 
                        rect.y + (rect.h - surface->h) / 2, 
                        surface->w, 
                        surface->h
                    };
                    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
    
    // Curseur jaune clignotant
    if (data->cursor_visible && data->is_focused) {
        extern TTF_Font* atomic_get_default_font(void);
        TTF_Font* font = atomic_get_default_font();
        int cursor_x = rect.x + 10;
        
        if (font && data->text[0] != '\0') {
            int text_w = 0;
            TTF_SizeText(font, data->text, &text_w, NULL);
            cursor_x += text_w;
        }
        
        SDL_Rect cursor = {cursor_x, rect.y + 8, 2, rect.h - 16};
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
    data->is_focused = false;
    
    input->component_data = data;
    input->component_update = text_input_update;
    atomic_set_custom_data(input->element, "input_data", data);
    atomic_set_custom_render(input->element, text_input_render);
    
    // 🆕 CONNEXION DU HANDLER CLAVIER via atomic
    input->element->user_data = input;
    atomic_set_click_handler(input->element, text_input_handle_focus);
    
    // 🔧 FIX CRITIQUE: Enregistrer la hitbox pour recevoir les événements
    if (tree->event_manager) {
        atomic_register_with_event_manager(input->element, tree->event_manager);
        printf("🔗 Text input hitbox registered with EventManager\n");
    }
    
    input->element->events.on_key_down = text_input_handle_text;
    
    return input;
}

void ui_text_input_set_max_length(UINode* input, int max_length) {
    TextInputData* data = (TextInputData*)input->component_data;
    if (data) data->max_length = max_length;
}
