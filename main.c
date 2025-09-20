#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include "src/core/core.h"
#include "src/window/window.h"
#include "src/utils/log_console.h"

// Gestionnaire de signal pour un nettoyage propre
void signal_handler(int sig) {
    printf("\n🛑 Signal %d reçu, fermeture propre...\n", sig);
#ifdef ENABLE_LOG_CONSOLE
    log_console_cleanup();
#endif
    exit(sig);
}

// Dans une fonction d'initialisation ou après la création du core
bool initialize_scenes(GameCore* core) {
    // 🔧 FIX: Test rapide pour vérifier que cnt_ui.c est compilé
    printf("🔧 Test de compilation des composants UI...\n");
    
    // Créer la scène menu
    Scene* menu_scene = create_menu_scene();
    if (!menu_scene) {
        printf("❌ Erreur: Impossible de créer la scène menu\n");
        return false;
    }
    
    printf("✅ Scène menu créée avec ID: '%s'\n", menu_scene->id);
    
    // Enregistrer la scène menu dans le SceneManager
    SceneManager* scene_manager = game_core_get_scene_manager(core);
    if (scene_manager) {
        bool registered = scene_manager_register_scene(scene_manager, menu_scene);
        if (registered) {
            printf("✅ Scène menu enregistrée dans le SceneManager\n");
            
            // Vérification
            Scene* found_scene = scene_manager_get_scene_by_id(scene_manager, "menu");
            if (found_scene) {
                printf("✅ VÉRIFICATION: Scène 'menu' trouvée par ID\n");
                
                // 🔧 FIX: Initialiser et connecter la scène menu IMMÉDIATEMENT
                printf("🔧 Pré-initialisation de la scène menu...\n");
                if (!menu_scene->initialized) {
                    if (menu_scene->init) {
                        menu_scene->init(menu_scene);
                        menu_scene->initialized = true;
                    }
                }
                
                // Connecter les événements de la scène menu
                menu_scene_connect_events(menu_scene, core);
                printf("✅ Scène menu pré-initialisée et connectée\n");
                
            } else {
                printf("❌ ERREUR: Scène 'menu' non trouvable après enregistrement!\n");
            }
        } else {
            printf("❌ Échec de l'enregistrement de la scène menu\n");
        }
    } else {
        printf("❌ SceneManager NULL - impossible d'enregistrer les scènes\n");
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    // Éviter les avertissements pour les paramètres non utilisés
    (void)argc;
    (void)argv;
    
    // Installer les gestionnaires de signaux
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("🎮 Démarrage de Fanorona...\n");
    
#ifdef ENABLE_LOG_CONSOLE
    // Initialiser la console de logs si le flag est activé
    printf("🖥️ Mode debug : Console de logs activée\n");
    if (!log_console_init()) {
        printf("⚠️ Impossible d'initialiser la console de logs\n");
        printf("   Le jeu continuera sans console de logs séparée\n");
    } else {
        printf("✅ Console de logs initialisée\n");
        printf("🎯 Deux fenêtres vont s'ouvrir :\n");
        printf("   1. Fenêtre de jeu (principale)\n");
        printf("   2. Console de logs (debug)\n");
    }
#else
    printf("🎮 Mode normal : Console de logs désactivée\n");
#endif
    
    // Initialiser SDL
    printf("⚙️ Initialisation de SDL...\n");
    if (!window_init_sdl()) {
        printf("❌ Erreur: Impossible d'initialiser SDL\n");
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("✅ SDL initialisé\n");
    
    // Initialiser les fenêtres globales EN PREMIER
    printf("🖼️ Initialisation des fenêtres...\n");
    window_initialize_global_windows();
    
    // Vérifier que la fenêtre par défaut (mini) a été créée
    GameWindow* mini_window = use_mini_window();
    if (!mini_window) {
        printf("❌ Erreur: Impossible de créer la fenêtre par défaut\n");
        window_cleanup_global_windows();
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("✅ Fenêtre de jeu créée (%dx%d)\n", mini_window->width, mini_window->height);
    
    // 🆕 ACTIVER LE TRACKING SOURIS IMMÉDIATEMENT
#ifdef ENABLE_LOG_CONSOLE
    if (log_console_is_enabled()) {
        log_console_set_mouse_tracking(true);
        printf("🖱️ Tracking souris activé dès maintenant\n");
        printf("   → Déplacez la souris dans la fenêtre pour voir les logs\n");
        
        // Test immédiat de la console de logs
        log_console_write("Main", "WindowReady", "main.c", 
                         "[main.c] 🖼️ Game window ready - mouse tracking active");
    }
#endif
    
    // Créer le core du jeu APRÈS que les fenêtres soient prêtes
    printf("🎯 Création du core du jeu...\n");
    GameCore* core = game_core_create();
    if (!core) {
        printf("❌ Erreur: Impossible de créer le core du jeu\n");
        window_cleanup_global_windows();
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    printf("✅ Core du jeu créé\n");
    
    // Initialiser les scènes
    printf("🎭 Initialisation des scènes...\n");
    if (!initialize_scenes(core)) {
        printf("❌ Erreur: Échec de l'initialisation des scènes\n");
        window_cleanup_global_windows();
        game_core_destroy(core);
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    
    // 🆕 Finaliser l'initialisation (connecter événements + démarrer boucle)
    printf("🔧 Finalisation de l'initialisation...\n");
    if (!game_core_finalize_init(core)) {
        printf("❌ Erreur lors de la finalisation du core\n");
        window_cleanup_global_windows();
        game_core_destroy(core);
        window_quit_sdl();
#ifdef ENABLE_LOG_CONSOLE
        log_console_cleanup();
#endif
        return -1;
    }
    
    printf("\n🚀 Fanorona - Jeu démarré avec succès !\n");
    
#ifdef ENABLE_LOG_CONSOLE
    log_console_ui_event("Main", "Start", "game", "Fanorona démarré avec console de logs");
    printf("🖥️ Console de logs active dans fenêtre séparée\n");
    printf("🖱️ Les événements souris seront trackés dès l'entrée dans la fenêtre de jeu\n");
    printf("⚡ Boucle d'événements dédiée active en arrière-plan\n");
#endif
    
    printf("🎮 Boucle principale démarrée (60 FPS) - Thread d'événements indépendant\n");
    printf("   📦 Événements capturés en continu dans un thread séparé\n");
    printf("   🔄 Traitement des événements depuis le buffer à chaque frame\n");
    printf("   🖼️ Rendu et logique du jeu dans la boucle principale\n\n");
    
    // 🆕 BOUCLE PRINCIPALE SIMPLIFIÉE ET OPTIMISÉE
    Uint32 target_frame_time = 16; // 16ms = ~60 FPS
    Uint32 frame_start;
    
    while (game_core_is_running(core)) {
        frame_start = SDL_GetTicks();
        
        // 🆕 Traiter les événements depuis le buffer (thread-safe)
        game_core_handle_events(core);
        
        // Mettre à jour le jeu
        game_core_update(core);
        
        // Rendre le jeu
        game_core_render(core);
        
        // 🆕 Contrôle de framerate plus précis
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < target_frame_time) {
            SDL_Delay(target_frame_time - frame_time);
        }
    }
    
    // Nettoyage
    printf("\n🧹 Fermeture du jeu...\n");
    
#ifdef ENABLE_LOG_CONSOLE
    log_console_ui_event("Main", "Shutdown", "game", "Arrêt de Fanorona");
    log_console_cleanup();
#endif
    
    window_cleanup_global_windows();
    game_core_destroy(core);
    window_quit_sdl();
    
    printf("✅ Fanorona fermé proprement\n");
    return 0;
}