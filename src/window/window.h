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
} GameWindow;

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

#endif // WINDOW_H