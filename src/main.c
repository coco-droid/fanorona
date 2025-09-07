#include "core/sdl_init.h"
#include "core/timer.h"
#include "audio/audio.h"
#include "scenes/scene.h"
#include "layer/layer_manager.h"
#include <stdio.h>

static void render_menu_scene(SDL_Renderer *ren) {
    // Rendu spécifique au menu
    SDL_SetRenderDrawColor(ren, 40, 40, 60, 255);
    SDL_RenderClear(ren);
    
    // TODO: Rendre les éléments du menu
}

static void render_game_scene(SDL_Renderer *ren) {
    // Rendu spécifique au jeu
    SDL_SetRenderDrawColor(ren, 20, 40, 20, 255);
    SDL_RenderClear(ren);
    
    // TODO: Rendre les éléments du jeu
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    
    CoreState core;
    if (!core_init(&core)) {
        printf("Failed to initialize core\n");
        return 1;
    }
    
    if (!audio_init()) {
        printf("Warning: Audio initialization failed\n");
    }
    
    // Créer et afficher la fenêtre de menu
    core_switch_to_menu(&core);
    
    bool running = true;
    SDL_Event e;
    bool show_game = false; // Pour tester le changement de fenêtre
    
    while (running) {
        double frame_start = timer_now();
        
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                // Changer de fenêtre avec ESPACE (pour test)
                show_game = !show_game;
                if (show_game) {
                    core_switch_to_game(&core);
                } else {
                    core_switch_to_menu(&core);
                }
            }
            
            // Gérer les événements de fenêtres
            if (!wm_handle_window_events(&core.wm, &e)) {
                // Dispatcher les événements à la scène active
                // TODO: Intégrer avec le layer manager
            }
        }
        
        // Rendre les fenêtres visibles
        if (core.menu_window && core.menu_window->visible) {
            wm_render_window(core.menu_window, render_menu_scene);
        }
        
        if (core.game_window && core.game_window->visible) {
            wm_render_window(core.game_window, render_game_scene);
        }
        
        // Cap at 60 FPS
        double frame_time = timer_now() - frame_start;
        if (frame_time < 16.666) {
            timer_delay(16.666 - frame_time);
        }
    }
    
    audio_quit();
    core_quit(&core);
    return 0;
}
