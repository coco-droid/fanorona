#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

typedef enum {
    WINDOW_MENU,
    WINDOW_GAME,
    WINDOW_SETTINGS,
    WINDOW_COUNT
} WindowType;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *corner_mask;  // Pour les coins arrondis
    WindowType type;
    int width, height;
    bool visible;
    bool has_rounded_corners;
    int corner_radius;
    char title[64];
} GameWindow;

typedef struct {
    GameWindow windows[WINDOW_COUNT];
    GameWindow *active_window;
    bool initialized;
} WindowManager;

bool wm_init(WindowManager *wm);
void wm_quit(WindowManager *wm);
void wm_load_icon(void);  // Nouvelle fonction pour charger l'icône
GameWindow *wm_create_window(WindowManager *wm, WindowType type, const char *title, 
                            int w, int h, bool rounded_corners, int corner_radius);
// Note: rounded_corners est ignoré - toutes les fenêtres ont des coins arrondis
// corner_radius: rayon personnalisé ou 0 pour utiliser DEFAULT_CORNER_RADIUS
void wm_destroy_window(WindowManager *wm, WindowType type);
void wm_show_window(WindowManager *wm, WindowType type);
void wm_hide_window(WindowManager *wm, WindowType type);
void wm_set_active_window(WindowManager *wm, WindowType type);
GameWindow *wm_get_window(WindowManager *wm, WindowType type);
void wm_render_window(GameWindow *win, void (*render_callback)(SDL_Renderer *));
bool wm_handle_window_events(WindowManager *wm, SDL_Event *e);
