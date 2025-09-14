#ifndef CORE_H
#define CORE_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../event/event.h"
#include "../scene/scene.h"
#include "../window/window.h"

// 🆕 Structure pour la boucle d'événements dédiée
typedef struct EventLoop {
    bool running;
    bool processing_events;
    SDL_Thread* event_thread;
    SDL_mutex* event_mutex;
    SDL_cond* event_condition;
    
    // Buffer circulaire pour les événements
    WindowEvent* event_buffer;
    int buffer_size;
    int buffer_head;
    int buffer_tail;
    int buffer_count;
    
    EventManager* event_manager;
} EventLoop;

// Structure principale du jeu
typedef struct GameCore {
    EventManager* event_manager;
    SceneManager* scene_manager;
    EventLoop* event_loop;  // 🆕 Boucle d'événements dédiée
    Uint32 last_time;
    bool running;
} GameCore;

// Fonctions du core
GameCore* game_core_create(void);
void game_core_destroy(GameCore* core);
bool game_core_finalize_init(GameCore* core); // 🆕 AJOUT
void game_core_handle_events(GameCore* core);  // 🆕 Maintenant traite le buffer
void game_core_update(GameCore* core);
void game_core_render(GameCore* core);
bool game_core_is_running(GameCore* core);
void game_core_set_running(GameCore* core, bool running);
EventManager* game_core_get_event_manager(GameCore* core);

// 🆕 Fonctions de la boucle d'événements
EventLoop* event_loop_create(EventManager* event_manager);
void event_loop_destroy(EventLoop* loop);
bool event_loop_start(EventLoop* loop);
void event_loop_stop(EventLoop* loop);
int event_loop_thread_function(void* data);
bool event_loop_push_event(EventLoop* loop, WindowEvent* event);
bool event_loop_pop_event(EventLoop* loop, WindowEvent* event);

// Fonctions de gestion des fenêtres
void game_core_switch_to_main_window(GameCore* core);
void game_core_switch_to_mini_window(GameCore* core);
void game_core_open_both_windows(GameCore* core);
WindowType game_core_get_active_window_type(GameCore* core);

#endif // CORE_H