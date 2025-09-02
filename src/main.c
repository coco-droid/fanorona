#include "core/sdl_init.h"
#include "core/timer.h"
#include "audio/audio.h"
#include "scenes/scene.h"
#include "layer/layer_manager.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    (void)argc; (void)argv; // Suppress unused warnings
    
    CoreState core;
    if (!core_init("Fanorona", 1024, 768, &core)) {
        printf("Failed to initialize SDL\n");
        return 1;
    }
    
    if (!audio_init()) {
        printf("Warning: Audio initialization failed\n");
    }
    
    Scene *current_scene = menu_scene_create();
    if (current_scene->init) {
        current_scene->init(current_scene);
    }
    current_scene->layout(current_scene, core.w, core.h);
    
    bool running = true;
    SDL_Event e;
    
    while (running) {
        double frame_start = timer_now();
        
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else {
                lm_dispatch(current_scene->lm, &e);
            }
        }
        
        SDL_SetRenderDrawColor(core.ren, 30, 30, 30, 255);
        SDL_RenderClear(core.ren);
        
        lm_render(current_scene->lm, core.ren);
        
        SDL_RenderPresent(core.ren);
        
        // Cap at 60 FPS
        double frame_time = timer_now() - frame_start;
        if (frame_time < 16.666) {
            timer_delay(16.666 - frame_time);
        }
    }
    
    if (current_scene->cleanup) {
        current_scene->cleanup(current_scene);
    }
    
    audio_quit();
    core_quit(&core);
    return 0;
}
