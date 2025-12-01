#include "core.h"
#include "../utils/log_console.h"
#include "../sound/sound.h" // 🆕 AJOUT: Include sound
#include <stdlib.h>
#include <stdio.h>

GameCore* game_core_create(void) {
    GameCore* core = (GameCore*)malloc(sizeof(GameCore));
    if (!core) return NULL;
    
    // 🆕 AJOUT: Initialisation du système de son
    if (!sound_init()) {
        printf("⚠️ Attention: Impossible d'initialiser le système de son\n");
    }
    
    core->event_manager = event_manager_create();
    if (!core->event_manager) { free(core); return NULL; }
    
    core->scene_manager = scene_manager_create();
    if (!core->scene_manager) { event_manager_destroy(core->event_manager); free(core); return NULL; }
    
    Scene* home_scene = create_home_scene();
    if (!home_scene || !home_scene->id || !home_scene->name) {
        if (home_scene) {
            if (home_scene->cleanup) home_scene->cleanup(home_scene);
            scene_destroy(home_scene);
        }
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    if (!scene_manager_set_scene(core->scene_manager, home_scene)) {
        if (home_scene->cleanup) home_scene->cleanup(home_scene);
        free(home_scene);
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    Scene* verification_scene = scene_manager_get_current_scene(core->scene_manager);
    if (verification_scene && verification_scene->init) {
        verification_scene->init(verification_scene);
    }
    
    core->last_time = SDL_GetTicks();
    core->running = true;
    return core;
}

bool game_core_finalize_init(GameCore* core) {
    if (!core || !core->scene_manager) return false;
    
    scene_manager_set_core(core->scene_manager, core);
    Scene* current_scene = scene_manager_get_current_scene(core->scene_manager);
    if (!current_scene) return false;
    
    if (!current_scene->event_manager) {
        current_scene->event_manager = event_manager_create();
        if (!current_scene->event_manager) return false;
    }
    
    if (current_scene->data) {
        if (strcmp(current_scene->id, "home") == 0) home_scene_connect_events(current_scene, core);
        else if (strcmp(current_scene->id, "menu") == 0) menu_scene_connect_events(current_scene, core);
        
        scene_manager_set_scene_for_window(core->scene_manager, current_scene, current_scene->target_window);
    } else {
        return false;
    }
    
    log_console_set_mouse_tracking(true);
    return true;
}

void game_core_destroy(GameCore* core) {
    if (!core) return;
    
    // 🆕 AJOUT: Nettoyage du son
    sound_cleanup();
    
    if (core->scene_manager) scene_manager_destroy(core->scene_manager);
    if (core->event_manager) event_manager_destroy(core->event_manager);
    free(core);
}

void game_core_handle_events(GameCore* core) {
    if (!core) return;
    WindowEvent window_event;
    
    while (window_poll_events(&window_event)) {
        if (window_event.sdl_event.type == SDL_QUIT) {
            core->running = false;
            return;
        }
        
        if (window_event.sdl_event.type == SDL_WINDOWEVENT && 
            window_event.sdl_event.window.event == SDL_WINDOWEVENT_CLOSE) {
            
            // 🆕 FIX: Ignorer les événements de fermeture pour les fenêtres qui n'existent plus
            // (Cela arrive souvent lors d'une transition où la fenêtre source est détruite)
            if (!window_event.source_window) {
                continue;
            }
            
            if (game_core_get_active_window_type(core) == WINDOW_TYPE_BOTH) {
                if (window_event.window_type == WINDOW_TYPE_MINI) {
                    Scene* mini_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MINI);
                    if (mini_scene) mini_scene->active = false;
                    
                    Scene* main_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MAIN);
                    if (main_scene) {
                        main_scene->active = true;
                        if (core->scene_manager) core->scene_manager->current_scene = main_scene;
                    }
                    game_core_switch_to_main_window(core);
                    return;
                } 
                else if (window_event.window_type == WINDOW_TYPE_MAIN) {
                    Scene* main_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MAIN);
                    if (main_scene) main_scene->active = false;
                    
                    Scene* mini_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MINI);
                    if (mini_scene) {
                        mini_scene->active = true;
                        if (core->scene_manager) core->scene_manager->current_scene = mini_scene;
                    }
                    game_core_switch_to_mini_window(core);
                    return;
                }
            }
            core->running = false;
            return;
        }
        
        if (core->scene_manager) {
            scene_manager_dispatch_event(core->scene_manager, &window_event);
        }
    }
}

bool game_core_is_running(GameCore* core) { return core ? core->running : false; }
void game_core_set_running(GameCore* core, bool running) { if (core) core->running = running; }
EventManager* game_core_get_event_manager(GameCore* core) { return core ? core->event_manager : NULL; }
SceneManager* game_core_get_scene_manager(GameCore* core) { return core ? core->scene_manager : NULL; }

void game_core_update(GameCore* core) {
    if (!core || !core->scene_manager) return;
    Uint32 current_time = SDL_GetTicks();
    float delta_time = (current_time - core->last_time) / 1000.0f;
    core->last_time = current_time;
    scene_manager_update(core->scene_manager, delta_time);
}

// Rendre le core (AVEC SYNCHRONISATION AMÉLIORÉE)
void game_core_render(GameCore* core) {
    if (!core || !core->scene_manager) return;
    
    WindowType active_type = window_get_active_window();
    
    // FIX: Synchronisation stricte pour éviter le clignotement
    switch (active_type) {
        case WINDOW_TYPE_MAIN: {
            GameWindow* main_window = use_main_window();
            if (main_window && main_window->renderer) {
                // 🔧 FIX: Only clear and render if we have a scene, to avoid black flashes
                Scene* main_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MAIN);
                
                if (main_scene) {
                    SDL_SetRenderDrawColor(main_window->renderer, 0, 0, 0, 255);
                    SDL_RenderClear(main_window->renderer);
                    
                    scene_manager_render_main(core->scene_manager);
                    
                    SDL_RenderPresent(main_window->renderer);
                }
            }
            break;
        }
        case WINDOW_TYPE_MINI: {
            GameWindow* mini_window = use_mini_window();
            if (mini_window && mini_window->renderer) {
                Scene* mini_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MINI);
                
                if (mini_scene) {
                    SDL_SetRenderDrawColor(mini_window->renderer, 0, 0, 0, 255);
                    SDL_RenderClear(mini_window->renderer);
                    
                    scene_manager_render_mini(core->scene_manager);
                    
                    SDL_RenderPresent(mini_window->renderer);
                }
            }
            break;
        }
        case WINDOW_TYPE_BOTH: {
            GameWindow* main_window = use_main_window();
            GameWindow* mini_window = use_mini_window();
            
            // Rendu séquentiel pour éviter les conflits
            if (main_window && main_window->renderer) {
                Scene* main_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MAIN);
                
                if (main_scene) {
                    SDL_SetRenderDrawColor(main_window->renderer, 0, 0, 0, 255);
                    SDL_RenderClear(main_window->renderer);
                    scene_manager_render_main(core->scene_manager);
                    SDL_RenderPresent(main_window->renderer);
                }
            }
            
            if (mini_window && mini_window->renderer) {
                Scene* mini_scene = scene_manager_get_active_scene_for_window(core->scene_manager, WINDOW_TYPE_MINI);
                
                if (mini_scene) {
                    SDL_SetRenderDrawColor(mini_window->renderer, 0, 0, 0, 255);
                    SDL_RenderClear(mini_window->renderer);
                    scene_manager_render_mini(core->scene_manager);
                    SDL_RenderPresent(mini_window->renderer);
                }
            }
            break;
        }
    }
}

// Basculer vers la fenêtre principale
void game_core_switch_to_main_window(GameCore* core) {
    (void)core; // Éviter l'avertissement
    printf("Basculement vers la fenêtre principale\n");
    window_set_active_window(WINDOW_TYPE_MAIN);
}

// Basculer vers la mini fenêtre
void game_core_switch_to_mini_window(GameCore* core) {
    (void)core; // Éviter l'avertissement
    printf("Basculement vers la mini fenêtre\n");
    window_set_active_window(WINDOW_TYPE_MINI);
}

// Ouvrir les deux fenêtres
void game_core_open_both_windows(GameCore* core) {
    (void)core; // Éviter l'avertissement
    printf("Ouverture des deux fenêtres\n");
    window_set_active_window(WINDOW_TYPE_BOTH);
}

// Obtenir le type de fenêtre active
WindowType game_core_get_active_window_type(GameCore* core) {
    (void)core; // Éviter l'avertissement
    return window_get_active_window();
}