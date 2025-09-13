#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "src/core/core.h"
#include "src/window/window.h"

int main(int argc, char* argv[]) {
    // Éviter les avertissements pour les paramètres non utilisés
    (void)argc;
    (void)argv;
    
    // Initialiser SDL
    if (!window_init_sdl()) {
        printf("Erreur: Impossible d'initialiser SDL\n");
        return -1;
    }
    
    // Créer le core du jeu
    GameCore* core = game_core_create();
    if (!core) {
        printf("Erreur: Impossible de créer le core du jeu\n");
        window_quit_sdl();
        return -1;
    }
    
    // Initialiser les fenêtres globales
    window_initialize_global_windows();
    
    // Vérifier que la fenêtre par défaut (mini) a été créée
    if (!use_mini_window()) {
        printf("Erreur: Impossible de créer la fenêtre par défaut\n");
        window_cleanup_global_windows();
        game_core_destroy(core);
        window_quit_sdl();
        return -1;
    }
    
    printf("Fanorona - Jeu démarré avec succès\n");
    
    // Exemple d'utilisation : changer de fenêtre après 5 secondes (optionnel)
    // window_set_active_window(WINDOW_TYPE_MAIN); // Pour basculer vers la grande fenêtre
    // window_set_active_window(WINDOW_TYPE_BOTH);  // Pour ouvrir les deux fenêtres
    
    // Boucle principale du jeu
    while (game_core_is_running(core)) {
        // Gérer les événements
        game_core_handle_events(core);
        
        // Mettre à jour le jeu
        game_core_update(core);
        
        // Rendre le jeu
        game_core_render(core);
        
        // Délai pour contrôler le framerate
        SDL_Delay(16); // ~60 FPS
    }
    
    // Nettoyage
    printf("Fermeture du jeu...\n");
    window_cleanup_global_windows();
    game_core_destroy(core);
    window_quit_sdl();
    
    return 0;
}