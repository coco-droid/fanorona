#ifndef CORE_H
#define CORE_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../event/event.h"
#include "../scene/scene.h"
#include "../window/window.h"

// Structure principale du jeu (SIMPLIFIÉE)
typedef struct GameCore {
    EventManager* event_manager;
    SceneManager* scene_manager;
    Uint32 last_time;
    bool running;
} GameCore;

// Fonctions du core (SIMPLIFIÉES)
GameCore* game_core_create(void);
void game_core_destroy(GameCore* core);
bool game_core_finalize_init(GameCore* core);
void game_core_handle_events(GameCore* core);  // FIX: Maintenant mono-thread simple
void game_core_update(GameCore* core);
void game_core_render(GameCore* core);
bool game_core_is_running(GameCore* core);
void game_core_set_running(GameCore* core, bool running);
EventManager* game_core_get_event_manager(GameCore* core);
SceneManager* game_core_get_scene_manager(GameCore* core);

// Fonctions de gestion des fenêtres
void game_core_switch_to_main_window(GameCore* core);
void game_core_switch_to_mini_window(GameCore* core);
void game_core_open_both_windows(GameCore* core);
WindowType game_core_get_active_window_type(GameCore* core);

// Fonctions de debug
void debug_current_state(GameCore* core);
void game_core_debug_event_system(GameCore* core);
void game_core_force_scene_event_registration(GameCore* core);

#endif // CORE_H