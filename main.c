#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include "src/core/core.h"
#include "src/window/window.h"
#include "src/utils/log_console.h"
#include "src/scene/scene_registry.h"  // 🆕 AJOUT: Include du registre de scènes

// Gestionnaire de signal pour un nettoyage propres
void signal_handler(int sig) {
    printf("\nSignal %d recu, fermeture propre...\n", sig);
#ifdef ENABLE_LOG_CONSOLE
    log_console_cleanup();
#endif
    exit(sig);
}

// 🔧 SIMPLIFICATION MAJEURE: Remplacement de initialize_scenes par le registre
bool initialize_scenes_with_registry(GameCore* core) {
    printf("Initialisation des scenes via le registre automatique...\n");
    
    SceneManager* scene_manager = game_core_get_scene_manager(core);
    if (!scene_manager) {
        printf("SceneManager NULL - impossible d'enregistrer les scenes\n");
        return false;
    }
    
    // 🆕 ENREGISTREMENT AUTOMATIQUE via le registre
    if (!scene_registry_register_all(scene_manager)) {
        printf("Impossible d'enregistrer les scenes via scene_registry\n");
        return false;
    }
    
    // 🆕 CONNEXION AUTOMATIQUE des événements pour toutes les scènes
    if (!scene_registry_connect_all_events(scene_manager, core)) {
        printf("Impossible de connecter les evenements via scene_registry\n");
        return false;
    }
    
    printf("Toutes les scenes initialisees et connectees via le registre\n");
    return true;
}

int main(int argc, char* argv[]) {
    // Éviter les avertissements pour les paramètres non utilisés
    (void)argc;
    (void)argv;
    
    // Installer les gestionnaires de signaux
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Demarrage de Fanorona...\n");
    
#ifdef ENABLE_LOG_CONSOLE
    // Initialiser la console de logs si le flag est activé
    printf("Mode debug : Console de logs activee\n");
    if (!log_console_init()) {
        printf("Impossible d'initialiser la console de logs\n");
        printf("   Le jeu continuera sans console de logs separee\n");
    } else {
        printf("Console de logs initialisee\n");
        printf("Deux fenetres vont s'ouvrir :\n");
        printf("   1. Fenetre de jeu (principale)\n");
        printf("   2. Console de logs (debug)\n");
    }
#else
    printf("Mode normal : Console de logs desactivee\n");
#endif
    
    // Initialiser SDL
    printf("Initialisation de SDL...\n");
    if (!window_init_sdl()) {
        printf("Erreur: Impossible d'initialiser SDL\n");
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("SDL initialise\n");
    
    // Initialiser les fenêtres globales EN PREMIER
    printf("Initialisation des fenetres...\n");
    window_initialize_global_windows();
    
    // Vérifier que la fenêtre par défaut (mini) a été créée
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
    
    // 🆕 ACTIVER LE TRACKING SOURIS IMMÉDIATEMENT
#ifdef ENABLE_LOG_CONSOLE
    if (log_console_is_enabled()) {
        log_console_set_mouse_tracking(true);
        printf("Tracking souris activé dès maintenant\n");
        printf("   → Déplacez la souris dans la fenêtre pour voir les logs\n");
        
        // Test immédiat de la console de logs
        log_console_write("Main", "WindowReady", "main.c", 
                         "[main.c] 🖼️ Game window ready - mouse tracking active");
    }
#endif
    
    // Créer le core du jeu APRÈS que les fenêtres soient prêtes
    printf("Création du core du jeu...\n");
    GameCore* core = game_core_create();
    if (!core) {
        printf("Erreur: Impossible de créer le core du jeu\n");
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("Core du jeu créé\n");
    
    // Initialiser les scènes VIA LE REGISTRE
    printf("Initialisation des scènes...\n");
    if (!initialize_scenes_with_registry(core)) {
        printf("Erreur: Échec de l'initialisation des scènes\n");
        game_core_destroy(core);
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    
    // 🆕 Finaliser l'initialisation (connecter événements)
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
    
    printf("\nFanorona - Jeu démarré avec succès !\n");
    
#ifdef ENABLE_LOG_CONSOLE
    log_console_ui_event("Main", "Start", "game", "Fanorona démarré avec console de logs");
    printf("Console de logs active dans fenêtre séparée\n");
    printf("Les événements souris seront trackés dès l'entrée dans la fenêtre de jeu\n");
#endif
    
    printf("Boucle principale démarrée (60 FPS) - Approche classique mono-thread\n");
    printf("   Événements traités directement dans la boucle principale\n");
    printf("   SDL utilisé de manière standard et stable\n\n");
    
    // 🔧 BOUCLE PRINCIPALE ULTRA-SIMPLE (mono-thread classique)
    while (game_core_is_running(core)) {
        // 1. 🔧 Traiter les événements (mono-thread, simple)
        game_core_handle_events(core);
        
        // 2. Mettre à jour le jeu
        game_core_update(core);
        
        // 3. Rendre le jeu
        game_core_render(core);
        
        // 4. 🔧 Contrôle de framerate simple
        SDL_Delay(16); // ~60 FPS
    }
    
    // Nettoyage
    printf("\nFermeture du jeu...\n");
    
#ifdef ENABLE_LOG_CONSOLE
    log_console_ui_event("Main", "Shutdown", "game", "Arrêt de Fanorona");
    log_console_cleanup();
#endif
    
    game_core_destroy(core);
    window_quit_sdl();
    
    printf("Fanorona fermé proprement\n");
    return 0;
}