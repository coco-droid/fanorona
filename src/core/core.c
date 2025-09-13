#include "core.h"
#include <stdlib.h>
#include <stdio.h>

// Créer le core du jeu
GameCore* game_core_create(void) {
    GameCore* core = (GameCore*)malloc(sizeof(GameCore));
    if (!core) {
        printf("Erreur: Impossible d'allouer la mémoire pour le core\n");
        return NULL;
    }
    
    core->event_manager = event_manager_create();
    if (!core->event_manager) {
        free(core);
        return NULL;
    }
    
    core->scene_manager = scene_manager_create();
    if (!core->scene_manager) {
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    // Créer et définir la scène home par défaut
    Scene* home_scene = create_home_scene();
    if (home_scene) {
        scene_manager_set_scene(core->scene_manager, home_scene);
    }
    
    core->last_time = SDL_GetTicks();
    core->running = true;
    
    return core;
}

// Détruire le core du jeu
void game_core_destroy(GameCore* core) {
    if (!core) return;
    
    if (core->scene_manager) {
        scene_manager_destroy(core->scene_manager);
    }
    
    if (core->event_manager) {
        event_manager_destroy(core->event_manager);
    }
    
    free(core);
}

// Gérer les événements
void game_core_handle_events(GameCore* core) {
    if (!core || !core->event_manager) return;
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Transmettre l'événement à l'event manager
        event_manager_handle_event(core->event_manager, &event);
        
        // Mettre à jour l'état du core si nécessaire
        if (!event_manager_is_running(core->event_manager)) {
            core->running = false;
        }
    }
}

// Vérifier si le jeu est en cours d'exécution
bool game_core_is_running(GameCore* core) {
    return core ? core->running : false;
}

// Définir l'état d'exécution
void game_core_set_running(GameCore* core, bool running) {
    if (core) {
        core->running = running;
        if (core->event_manager) {
            event_manager_set_running(core->event_manager, running);
        }
    }
}

// Obtenir l'event manager
EventManager* game_core_get_event_manager(GameCore* core) {
    return core ? core->event_manager : NULL;
}

// Mettre à jour le core
void game_core_update(GameCore* core) {
    if (!core || !core->scene_manager) return;
    
    // Calculer le delta time
    Uint32 current_time = SDL_GetTicks();
    float delta_time = (current_time - core->last_time) / 1000.0f;
    core->last_time = current_time;
    
    // Mettre à jour le gestionnaire de scènes
    scene_manager_update(core->scene_manager, delta_time);
}

// Rendre le core
void game_core_render(GameCore* core) {
    if (!core || !core->scene_manager) return;
    
    WindowType active_type = window_get_active_window();
    
    // Rendre selon le type de fenêtre active
    switch (active_type) {
        case WINDOW_TYPE_MAIN: {
            GameWindow* main_window = use_main_window();
            if (main_window) {
                window_clear(main_window);
                scene_manager_render_main(core->scene_manager);
                window_present(main_window);
            }
            break;
        }
        case WINDOW_TYPE_MINI: {
            GameWindow* mini_window = use_mini_window();
            if (mini_window) {
                window_clear(mini_window);
                scene_manager_render_mini(core->scene_manager);
                window_present(mini_window);
            }
            break;
        }
        case WINDOW_TYPE_BOTH: {
            GameWindow* main_window = use_main_window();
            GameWindow* mini_window = use_mini_window();
            
            if (main_window) {
                window_clear(main_window);
                scene_manager_render_main(core->scene_manager);
                window_present(main_window);
            }
            
            if (mini_window) {
                window_clear(mini_window);
                scene_manager_render_mini(core->scene_manager);
                window_present(mini_window);
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