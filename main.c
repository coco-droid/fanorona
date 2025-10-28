#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include "src/core/core.h"
#include "src/window/window.h"
#include "src/utils/log_console.h"
#include "src/scene/scene_registry.h"

// Signal handler for clean shutdown
void signal_handler(int sig) {
    printf("\nSignal %d recu, fermeture propre...\n", sig);
#ifdef ENABLE_LOG_CONSOLE
    log_console_cleanup();
#endif
    exit(sig);
}

// Initialize scenes using the registry system
bool initialize_scenes_with_registry(GameCore* core) {
    printf("Initialisation des scenes via le registre automatique...\n");
    
    SceneManager* scene_manager = game_core_get_scene_manager(core);
    if (!scene_manager) {
        printf("SceneManager NULL - impossible d'enregistrer les scenes\n");
        return false;
    }
    
    // Register all scenes automatically via registry
    if (!scene_registry_register_all(scene_manager)) {
        printf("Impossible d'enregistrer les scenes via scene_registry\n");
        return false;
    }
    
    // Connect events for all scenes automatically
    if (!scene_registry_connect_all_events(scene_manager, core)) {
        printf("Impossible de connecter les evenements via scene_registry\n");
        return false;
    }
    
    printf("Toutes les scenes initialisees et connectees via le registre\n");
    return true;
}

int main(int argc, char* argv[]) {
    // Avoid warnings for unused parameters
    (void)argc;
    (void)argv;
    
    // Install signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Demarrage de Fanorona...\n");
    printf("PRESENTED BY BOULDIMAS\n");
    printf("Directed by ROBENS,SHAMMA,GERARDO\n");
#ifdef ENABLE_LOG_CONSOLE
    // Initialize log console if flag is enabled
    printf("Mode debug : Console de logs activee\n");
    if (!log_console_init()) {
        printf("Impossible d'initialiser la console de logs\n");
        printf("   Le jeu continuera sans console de logs separee\n");
    } else {
        printf("Console de logs initialisee\n");
    }
#else
    printf("Mode normal : Console de logs desactivee\n");
#endif
    
    // Initialize SDL
    printf("Initialisation de SDL...\n");
    if (!window_init_sdl()) {
        printf("Erreur: Impossible d'initialiser SDL\n");
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("SDL initialise\n");
    
    // Initialize global windows first
    printf("Initialisation des fenetres...\n");
    window_initialize_global_windows();
    
    // Verify that default window (mini) was created
    GameWindow* mini_window = use_mini_window();
    if (!mini_window) {
        printf("Erreur: Impossible de creer la fenetre par defaut\n");
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("Fenetre de jeu creee (%dx%d)\n", mini_window->width, mini_window->height);
    
    // Activate mouse tracking immediately
#ifdef ENABLE_LOG_CONSOLE
    if (log_console_is_enabled()) {
        log_console_set_mouse_tracking(true);
        printf("Tracking souris active\n");
        
        // Immediate test of log console
        log_console_write("Main", "WindowReady", "main.c", 
                         "[main.c]  Game window ready - mouse tracking active");
    }
#endif
    
    // Create game core after windows are ready
    printf("Creation du core du jeu...\n");
    GameCore* core = game_core_create();
    if (!core) {
        printf("Erreur: Impossible de creer le core du jeu\n");
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("Core du jeu cree\n");
    
    // Initialize scenes via registry
    printf("Initialisation des scenes...\n");
    if (!initialize_scenes_with_registry(core)) {
        printf("Erreur: Echec de l'initialisation des scenes\n");
        game_core_destroy(core);
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    
    // Finalize initialization (connect events)
    printf("Finalisation de l'initialisation...\n");
    if (!game_core_finalize_init(core)) {
        printf("Erreur lors de la finalisation du core\n");
        game_core_destroy(core);
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    
    printf("\nFanorona - Jeu demarre avec succes !\n");
    
#ifdef ENABLE_LOG_CONSOLE
    log_console_ui_event("Main", "Start", "game", "Fanorona demarre avec console de logs");
#endif
    
    printf("Boucle principale demarree (60 FPS)\n\n");
    
    // Main game loop - simple mono-thread approach
    while (game_core_is_running(core)) {
        // Handle events
        game_core_handle_events(core);
        
        // Update game state
        game_core_update(core);
        
        // Render game
        game_core_render(core);
        
        // Simple framerate control
        SDL_Delay(16); // ~60 FPS
    }
    
    // Cleanup
    printf("\nFermeture du jeu...\n");
    
#ifdef ENABLE_LOG_CONSOLE
    log_console_ui_event("Main", "Shutdown", "game", "Arret de Fanorona");
    log_console_cleanup();
#endif
    
    game_core_destroy(core);
    window_quit_sdl();
    
    printf("Fanorona ferme proprement\n");
    return 0;
}