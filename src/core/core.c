#include "core.h"
#include "../utils/log_console.h"
#include <stdlib.h>
#include <stdio.h>

// === FONCTIONS DU CORE SIMPLIFIÉES ===

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
    
    // 🔧 FIX: Créer et INITIALISER la scène home immédiatement avec vérifications
    printf("🏠 Création de la scène home...\n");
    Scene* home_scene = create_home_scene();
    if (!home_scene) {
        printf("❌ Erreur: Impossible de créer la scène home\n");
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    // 🆕 Validate that scene has valid strings
    if (!home_scene->id || !home_scene->name) {
        printf("❌ Erreur: Scène home créée avec des chaînes invalides\n");
        if (home_scene->cleanup) home_scene->cleanup(home_scene);
        scene_destroy(home_scene);
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    printf("🔧 Définition de la scène home comme scène courante...\n");
    if (!scene_manager_set_scene(core->scene_manager, home_scene)) {
        printf("❌ Erreur: Impossible de définir la scène home\n");
        // Nettoyer la scène créée
        if (home_scene->cleanup) home_scene->cleanup(home_scene);
        free(home_scene);
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    // 🆕 Vérifier que la scène a été correctement définie
    Scene* verification_scene = scene_manager_get_current_scene(core->scene_manager);
    if (!verification_scene) {
        printf("❌ Erreur: Scène non définie après scene_manager_set_scene\n");
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    printf("✅ Scène home correctement définie (nom: '%s')\n", verification_scene->name);
    
    // 🆕 INITIALISER la scène immédiatement
    if (verification_scene->init) {
        printf("🔧 Initialisation de la scène home...\n");
        verification_scene->init(verification_scene);
        
        // Vérifier que l'initialisation a réussi (données créées)
        if (verification_scene->data) {
            printf("✅ Scène home initialisée avec succès\n");
        } else {
            printf("⚠️ Scène initialisée mais données manquantes\n");
        }
    } else {
        printf("⚠️ Pas de fonction d'initialisation pour la scène\n");
    }
    
    core->last_time = SDL_GetTicks();
    core->running = true;
    
    printf("✅ Core créé avec scène home initialisée et vérifiée\n");
    
    return core;
}

// 🆕 Fonction pour finaliser l'initialisation (SIMPLIFIÉE)
bool game_core_finalize_init(GameCore* core) {
    if (!core) {
        printf("❌ Core est NULL\n");
        return false;
    }
    
    if (!core->scene_manager) {
        printf("❌ SceneManager est NULL\n");
        return false;
    }
    
    // Récupération de la scène courante
    printf("🔍 Récupération de la scène courante...\n");
    Scene* current_scene = scene_manager_get_current_scene(core->scene_manager);
    
    if (!current_scene) {
        printf("❌ Aucune scène courante trouvée\n");
        return false;
    } else {
        printf("✅ Scène courante trouvée: '%s'\n", current_scene->name ? current_scene->name : "sans nom");
    }
    
    // Créer un EventManager dédié pour la scène si elle n'en a pas déjà un
    if (!current_scene->event_manager) {
        printf("🔧 Création d'un EventManager dédié pour la scène '%s'...\n", current_scene->name);
        current_scene->event_manager = event_manager_create();
        if (!current_scene->event_manager) {
            printf("❌ Impossible de créer l'EventManager pour la scène\n");
            return false;
        }
    }
    
    // Connexion des événements
    if (current_scene->data) {
        printf("🔗 Connexion des événements de la scène '%s'...\n", current_scene->name);
        
        // Connecter les événements en fonction du type de scène
        if (strcmp(current_scene->id, "home") == 0) {
            home_scene_connect_events(current_scene, core);
        } else if (strcmp(current_scene->id, "menu") == 0) {
            menu_scene_connect_events(current_scene, core);
        } else {
            printf("⚠️ Type de scène inconnu '%s', pas de connexion d'événements spécifique\n", current_scene->id);
        }
        
        printf("✅ Événements de la scène connectés\n");
        
        // Assigner la scène à sa fenêtre cible
        WindowType target_window = current_scene->target_window;
        scene_manager_set_scene_for_window(core->scene_manager, current_scene, target_window);
        printf("✅ Scène '%s' assignée à la fenêtre type %d\n", current_scene->name, target_window);
        
        // Debug: Vérifier combien d'éléments sont enregistrés
        printf("🔍 Debug: Vérification des éléments enregistrés...\n");
        log_console_debug_event_manager(current_scene->event_manager);
    } else {
        printf("❌ Scène non initialisée\n");
        return false;
    }
    
    // Activer le tracking souris
    log_console_set_mouse_tracking(true);
    printf("🖱️ Tracking souris activé\n");
    
    log_console_write("EventLoop", "SystemReady", "core.c", 
                     "[core.c] Event system ready with classic mono-thread approach");
    
    printf("✅ Core complètement initialisé avec gestion d'événements mono-thread\n");
    return true;
}

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

// 🔧 NOUVELLE FONCTION : Traitement simple mono-thread des événements
void game_core_handle_events(GameCore* core) {
    if (!core) return;
    
    SDL_Event event;
    int events_processed = 0;
    
    // 🔧 SIMPLE : Traiter TOUS les événements disponibles en mono-thread
    while (SDL_PollEvent(&event)) {
        events_processed++;
        
        // 🔧 GESTION DIRECTE des événements critiques
        if (event.type == SDL_QUIT) {
            // 🔧 SUPPRESSION: Log seulement pour QUIT
            printf("🚪 SDL_QUIT reçu - Arrêt du jeu\n");
            core->running = false;
            return;
        }
        
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
            // 🔧 SUPPRESSION: Log seulement pour fermeture de fenêtre
            printf("🚪 Fermeture de fenêtre - Arrêt du jeu\n");
            core->running = false;
            return;
        }
        
        // 🔧 ROUTAGE SIMPLE par fenêtre active
        WindowType active_type = window_get_active_window();
        Scene* active_scene = scene_manager_get_active_scene_for_window(core->scene_manager, active_type);
        
        if (active_scene && active_scene->event_manager) {
            // Transmettre directement à l'EventManager de la scène
            event_manager_handle_event(active_scene->event_manager, &event);
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

// 🆕 Fonction pour obtenir le SceneManager
SceneManager* game_core_get_scene_manager(GameCore* core) {
    return core ? core->scene_manager : NULL;
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

// Rendre le core (AVEC SYNCHRONISATION AMÉLIORÉE)
void game_core_render(GameCore* core) {
    if (!core || !core->scene_manager) return;
    
    WindowType active_type = window_get_active_window();
    
    // 🔧 FIX: Synchronisation stricte pour éviter le clignotement
    switch (active_type) {
        case WINDOW_TYPE_MAIN: {
            GameWindow* main_window = use_main_window();
            if (main_window && main_window->renderer) {
                // 🔧 Clear avec couleur de fond cohérente
                SDL_SetRenderDrawColor(main_window->renderer, 135, 206, 250, 255);
                SDL_RenderClear(main_window->renderer);
                
                // Rendu de la scène (sans clear/present)
                scene_manager_render_main(core->scene_manager);
                
                // 🔧 Present SEULEMENT à la fin
                SDL_RenderPresent(main_window->renderer);
            }
            break;
        }
        case WINDOW_TYPE_MINI: {
            GameWindow* mini_window = use_mini_window();
            if (mini_window && mini_window->renderer) {
                // 🔧 Clear avec couleur de fond cohérente
                SDL_SetRenderDrawColor(mini_window->renderer, 135, 206, 250, 255);
                SDL_RenderClear(mini_window->renderer);
                
                // Rendu de la scène (sans clear/present)
                scene_manager_render_mini(core->scene_manager);
                
                // 🔧 Present SEULEMENT à la fin
                SDL_RenderPresent(mini_window->renderer);
            }
            break;
        }
        case WINDOW_TYPE_BOTH: {
            GameWindow* main_window = use_main_window();
            GameWindow* mini_window = use_mini_window();
            
            // 🔧 Rendu séquentiel pour éviter les conflits
            if (main_window && main_window->renderer) {
                SDL_SetRenderDrawColor(main_window->renderer, 135, 206, 250, 255);
                SDL_RenderClear(main_window->renderer);
                scene_manager_render_main(core->scene_manager);
                SDL_RenderPresent(main_window->renderer);
            }
            
            if (mini_window && mini_window->renderer) {
                SDL_SetRenderDrawColor(mini_window->renderer, 135, 206, 250, 255);
                SDL_RenderClear(mini_window->renderer);
                scene_manager_render_mini(core->scene_manager);
                SDL_RenderPresent(mini_window->renderer);
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