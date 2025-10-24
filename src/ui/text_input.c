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
    bool is_focused;
} TextInputData;

// 🆕 REGISTRE GLOBAL - MOVED BEFORE USAGE
typedef struct TextInputRegistry {
    char* scene_input_id;  // 🔧 CHANGED: ID de scène (ex: "input_name")
    char text[256];
    struct TextInputRegistry* next;
} TextInputRegistry;

static TextInputRegistry* g_text_registry = NULL;
static SDL_mutex* g_registry_mutex = NULL;

static void text_registry_init(void) {
    if (!g_registry_mutex) {
        g_registry_mutex = SDL_CreateMutex();
    }
}

static void text_registry_set(const char* scene_input_id, const char* text) {
    if (!scene_input_id) return;
    
    text_registry_init();
    SDL_LockMutex(g_registry_mutex);
    
    TextInputRegistry* current = g_text_registry;
    while (current) {
        if (strcmp(current->scene_input_id, scene_input_id) == 0) {
            strncpy(current->text, text ? text : "", sizeof(current->text) - 1);
            current->text[sizeof(current->text) - 1] = '\0';
            SDL_UnlockMutex(g_registry_mutex);
            printf("📝 [TEXT_REGISTRY] Updated '%s': '%s'\n", scene_input_id, current->text);
            return;
        }
        current = current->next;
    }
    
    TextInputRegistry* entry = (TextInputRegistry*)malloc(sizeof(TextInputRegistry));
    if (entry) {
        entry->scene_input_id = strdup(scene_input_id);
        strncpy(entry->text, text ? text : "", sizeof(entry->text) - 1);
        entry->text[sizeof(entry->text) - 1] = '\0';
        entry->next = g_text_registry;
        g_text_registry = entry;
        printf("📝 [TEXT_REGISTRY] Created '%s': '%s'\n", scene_input_id, entry->text);
    }
    
    SDL_UnlockMutex(g_registry_mutex);
}

static const char* text_registry_get(const char* scene_input_id) {
    if (!scene_input_id) return "";
    
    text_registry_init();
    SDL_LockMutex(g_registry_mutex);
    
    TextInputRegistry* current = g_text_registry;
    while (current) {
        if (strcmp(current->scene_input_id, scene_input_id) == 0) {
            SDL_UnlockMutex(g_registry_mutex);
            return current->text;
        }
        current = current->next;
    }
    
    SDL_UnlockMutex(g_registry_mutex);
    return "";
}

// NOW WE CAN USE text_registry_set BELOW

static void text_input_update(UINode* node, float delta_time) {
    TextInputData* data = (TextInputData*)node->component_data;
    if (!data) return;
    
    data->blink_timer += delta_time;
    if (data->blink_timer >= 0.5f) {
        data->cursor_visible = !data->cursor_visible;
        data->blink_timer = 0.0f;
    }
}

static void text_input_handle_text(void* element, SDL_Event* event) {
    AtomicElement* atomic = (AtomicElement*)element;
    UINode* node = (UINode*)atomic->user_data;
    if (!node) return;
    
    TextInputData* data = (TextInputData*)node->component_data;
    if (!data || !data->is_focused) return;
    
    // 🔧 FIX: Get scene_input_id from custom_data
    const char* scene_input_id = (const char*)atomic_get_custom_data(atomic, "scene_input_id");
    if (!scene_input_id) {
        printf("⚠️ No scene_input_id for this text input\n");
        return;
    }
    
    // 🔧 FIX: Deduplicate events using timestamp
    static Uint32 last_event_timestamp = 0;
    static char last_event_text[32] = {0};
    
    if (event->type == SDL_TEXTINPUT) {
        // Check if this is a duplicate of the previous event
        if (event->common.timestamp == last_event_timestamp && 
            strcmp(event->text.text, last_event_text) == 0) {
            return; // Skip duplicate
        }
        
        // Record this event
        last_event_timestamp = event->common.timestamp;
        strncpy(last_event_text, event->text.text, sizeof(last_event_text) - 1);
        
        int len = strlen(data->text);
        if (len < data->max_length - 1) {
            strcat(data->text, event->text.text);
            text_registry_set(scene_input_id, data->text);
            printf("📝 Text input '%s': '%s'\n", scene_input_id, data->text);
        }
    } else if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_BACKSPACE) {
        // Deduplicate backspace too
        if (event->common.timestamp == last_event_timestamp) {
            return;
        }
        last_event_timestamp = event->common.timestamp;
        
        int len = strlen(data->text);
        if (len > 0) {
            data->text[len - 1] = '\0';
            text_registry_set(scene_input_id, data->text);
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

// 🆕 NETTOYER LE REGISTRE
void ui_text_input_cleanup_registry(void) {
    if (!g_registry_mutex) return;
    
    SDL_LockMutex(g_registry_mutex);
    
    TextInputRegistry* current = g_text_registry;
    while (current) {
        TextInputRegistry* next = current->next;
        free(current->scene_input_id);
        free(current);
        current = next;
    }
    g_text_registry = NULL;
    
    SDL_UnlockMutex(g_registry_mutex);
    SDL_DestroyMutex(g_registry_mutex);
    g_registry_mutex = NULL;
    
    printf("🧹 [TEXT_REGISTRY] Cleanup complete\n");
}

// 🆕 NEW: Get text by scene_input_id (global registry)
const char* ui_text_input_get_text_by_id(const char* scene_input_id) {
    return text_registry_get(scene_input_id);
}

// 🔧 CHANGED: ui_text_input_get_text now uses scene_input_id from custom_data
const char* ui_text_input_get_text(UINode* input) {
    if (!input || !input->element) return "";
    
    const char* scene_input_id = (const char*)atomic_get_custom_data(input->element, "scene_input_id");
    if (!scene_input_id) return "";
    
    return text_registry_get(scene_input_id);
}

// 🆕 NEW: Set scene_input_id for a text input
void ui_text_input_set_scene_id(UINode* input, const char* scene_input_id) {
    if (!input || !input->element || !scene_input_id) return;
    
    // Store scene_input_id in custom_data
    atomic_set_custom_data(input->element, "scene_input_id", (void*)strdup(scene_input_id));
    
    // Initialize in registry with empty text
    text_registry_set(scene_input_id, "");
    
    printf("🔗 Text input linked to scene ID: '%s'\n", scene_input_id);
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
    
    input->element->user_data = input;
    atomic_set_click_handler(input->element, text_input_handle_focus);
    
    if (tree->event_manager) {
        atomic_register_with_event_manager(input->element, tree->event_manager);
        printf("🔗 Text input '%s' registered with EventManager\n", id ? id : "NoID");
    }
    
    input->element->events.on_key_down = text_input_handle_text;
    
    return input;
}

void ui_text_input_set_max_length(UINode* input, int max_length) {
    TextInputData* data = (TextInputData*)input->component_data;
    if (data) data->max_length = max_length;
}

// 🆕 NEW: Set text programmatically
void ui_text_input_set_text(UINode* input, const char* text) {
    // 🔧 FIX: Triple safety check
    if (!input) {
        printf("⚠️ [TEXT_INPUT] ui_text_input_set_text: input is NULL\n");
        return;
    }
    if (!input->element) {
        printf("⚠️ [TEXT_INPUT] ui_text_input_set_text: input->element is NULL\n");
        return;
    }
    
    TextInputData* data = (TextInputData*)input->component_data;
    if (!data) {
        printf("⚠️ [TEXT_INPUT] ui_text_input_set_text: component_data is NULL\n");
        return;
    }
    
    const char* scene_input_id = (const char*)atomic_get_custom_data(input->element, "scene_input_id");
    
    // Update local buffer
    if (text) {
        strncpy(data->text, text, sizeof(data->text) - 1);
        data->text[sizeof(data->text) - 1] = '\0';
    } else {
        data->text[0] = '\0';
    }
    
    // Update global registry
    if (scene_input_id) {
        text_registry_set(scene_input_id, data->text);
    }
}

// 🆕 NEW: Set placeholder dynamically
void ui_text_input_set_placeholder(UINode* input, const char* placeholder) {
    if (!input) return;
    
    TextInputData* data = (TextInputData*)input->component_data;
    if (!data || !placeholder) return;
    
    strncpy(data->placeholder, placeholder, sizeof(data->placeholder) - 1);
    data->placeholder[sizeof(data->placeholder) - 1] = '\0';
}
