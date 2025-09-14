#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Énumération pour les types de fenêtres
typedef enum WindowType {
    WINDOW_TYPE_MAIN,
    WINDOW_TYPE_MINI,
    WINDOW_TYPE_BOTH
} WindowType;

// Structure pour une fenêtre
typedef struct GameWindow {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width;
    int height;
    const char* title;
    Uint32 window_id; // 🆕 ID unique de la fenêtre SDL
    bool has_focus;   // 🆕 Focus de la fenêtre
} GameWindow;

// 🆕 Structure pour un événement avec contexte de fenêtre
typedef struct WindowEvent {
    SDL_Event sdl_event;
    GameWindow* source_window;
    WindowType window_type;
    bool is_valid;
} WindowEvent;

// Fonctions du window manager
bool window_init_sdl(void);
void window_quit_sdl(void);
GameWindow* window_create(const char* title, int width, int height);
void window_destroy(GameWindow* window);
void window_clear(GameWindow* window);
void window_present(GameWindow* window);
SDL_Renderer* window_get_renderer(GameWindow* window);

// Fonctions pour les fenêtres spécifiques
GameWindow* create_mini_window(void);
GameWindow* create_large_window(void);

// Système de fenêtres globales pour les scènes
void window_initialize_global_windows(void);
void window_cleanup_global_windows(void);
void window_set_active_window(WindowType type);
WindowType window_get_active_window(void);
GameWindow* use_mini_window(void);
GameWindow* use_main_window(void);
GameWindow* use_active_window(void);
bool window_is_window_active(WindowType type);

// 🆕 Gestion des événements par fenêtre
bool window_poll_events(WindowEvent* window_event);
GameWindow* window_get_by_id(Uint32 window_id);
void window_update_focus(void);
GameWindow* window_get_focused_window(void);
bool window_has_events_pending(void);

#endif // WINDOW_H