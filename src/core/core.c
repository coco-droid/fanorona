#include "core.h"
#include "../utils/log_console.h"
#include <stdlib.h>
#include <stdio.h>

// === FONCTIONS DE LA BOUCLE D'ÉVÉNEMENTS ===

EventLoop* event_loop_create(EventManager* event_manager) {
    EventLoop* loop = (EventLoop*)calloc(1, sizeof(EventLoop));
    if (!loop) {
        printf("❌ Impossible d'allouer la mémoire pour EventLoop\n");
        return NULL;
    }
    
    // Initialiser le buffer circulaire
    loop->buffer_size = 256; // Buffer pour 256 événements
    loop->event_buffer = (WindowEvent*)calloc(loop->buffer_size, sizeof(WindowEvent));
    if (!loop->event_buffer) {
        free(loop);
        return NULL;
    }
    
    // Créer les primitives de synchronisation
    loop->event_mutex = SDL_CreateMutex();
    loop->event_condition = SDL_CreateCond();
    
    if (!loop->event_mutex || !loop->event_condition) {
        if (loop->event_mutex) SDL_DestroyMutex(loop->event_mutex);
        if (loop->event_condition) SDL_DestroyCond(loop->event_condition);
        free(loop->event_buffer);
        free(loop);
        return NULL;
    }
    
    loop->event_manager = event_manager;
    loop->running = false;
    loop->processing_events = false;
    
    log_console_write("EventLoop", "Created", "core.c", 
                     "[core.c] Event loop created with 256-event buffer");
    
    return loop;
}

void event_loop_destroy(EventLoop* loop) {
    if (!loop) return;
    
    // Arrêter la boucle si elle tourne
    event_loop_stop(loop);
    
    // Libérer les ressources
    if (loop->event_mutex) SDL_DestroyMutex(loop->event_mutex);
    if (loop->event_condition) SDL_DestroyCond(loop->event_condition);
    free(loop->event_buffer);
    free(loop);
    
    log_console_write("EventLoop", "Destroyed", "core.c", 
                     "[core.c] Event loop destroyed");
}

bool event_loop_start(EventLoop* loop) {
    if (!loop || loop->running) return false;
    
    loop->running = true;
    loop->processing_events = true;
    
    // Créer le thread d'événements
    loop->event_thread = SDL_CreateThread(event_loop_thread_function, "EventLoop", loop);
    
    if (!loop->event_thread) {
        printf("❌ Impossible de créer le thread d'événements: %s\n", SDL_GetError());
        loop->running = false;
        loop->processing_events = false;
        return false;
    }
    
    log_console_write("EventLoop", "Started", "core.c", 
                     "[core.c] Event loop thread started - dedicated event processing");
    
    return true;
}

void event_loop_stop(EventLoop* loop) {
    if (!loop || !loop->running) return;
    
    log_console_write("EventLoop", "Stopping", "core.c", 
                     "[core.c] Stopping event loop thread...");
    
    // Signaler l'arrêt
    SDL_LockMutex(loop->event_mutex);
    loop->running = false;
    loop->processing_events = false;
    SDL_CondSignal(loop->event_condition);
    SDL_UnlockMutex(loop->event_mutex);
    
    // Attendre la fin du thread
    if (loop->event_thread) {
        int thread_result;
        SDL_WaitThread(loop->event_thread, &thread_result);
        loop->event_thread = NULL;
        
        log_console_write("EventLoop", "Stopped", "core.c", 
                         "[core.c] Event loop thread stopped cleanly");
    }
}

// 🆕 THREAD DÉDIÉ POUR LA CAPTURE D'ÉVÉNEMENTS (CORRIGÉ)
int event_loop_thread_function(void* data) {
    EventLoop* loop = (EventLoop*)data;
    
    log_console_write("EventLoop", "ThreadStarted", "core.c", 
                     "[core.c] Event capture thread started");
    
    while (loop->running) {
        WindowEvent window_event;
        
        if (window_poll_events(&window_event)) {
            if (window_event.is_valid) {
                
                // 🔧 FIX: Classification correcte des événements
                bool should_log = false;
                
                switch (window_event.sdl_event.type) {
                    case SDL_MOUSEBUTTONDOWN: // 1024 - VRAI CLIC
                        should_log = true;
                        break;
                    case SDL_MOUSEBUTTONUP: // 1025 - PAS UN CLIC, mais important pour UI
                        should_log = false; // 🔧 Ne plus logger les mouseup comme critiques
                        break;
                    case SDL_WINDOWEVENT: // 512
                        // Seulement logger les fermetures de fenêtre
                        if (window_event.sdl_event.window.event == SDL_WINDOWEVENT_CLOSE) {
                            should_log = true;
                        }
                        break;
                    case SDL_QUIT: // 256
                        should_log = true;
                        break;
                    case SDL_MOUSEMOTION: // 1026 - NE PAS LOGGER
                        should_log = false;
                        break;
                    default: 
                        should_log = false;
                        break;
                }
                
                // 🔧 LOG SEULEMENT SI CRITIQUE
                if (should_log) {
                    log_console_write_event("EventLoop", "EventCaptured", "core.c", 
                                           "[core.c] Critical event captured", 
                                           window_event.sdl_event.type);
                }
                
                // Ajouter TOUS les événements au buffer
                event_loop_push_event(loop, &window_event);
            }
        } else {
            SDL_Delay(1);
        }
        
        window_update_focus();
    }
    
    log_console_write("EventLoop", "ThreadExiting", "core.c", 
                     "[core.c] Event thread stopped");
    
    return 0;
}

// Buffer thread-safe pour les événements
bool event_loop_push_event(EventLoop* loop, WindowEvent* event) {
    if (!loop || !event) return false;
    
    SDL_LockMutex(loop->event_mutex);
    
    // Vérifier si le buffer est plein
    if (loop->buffer_count >= loop->buffer_size) {
        SDL_UnlockMutex(loop->event_mutex);
        return false; // Buffer plein
    }
    
    // Ajouter l'événement au buffer
    loop->event_buffer[loop->buffer_head] = *event;
    loop->buffer_head = (loop->buffer_head + 1) % loop->buffer_size;
    loop->buffer_count++;
    
    // Signaler qu'un événement est disponible
    SDL_CondSignal(loop->event_condition);
    
    SDL_UnlockMutex(loop->event_mutex);
    return true;
}

bool event_loop_pop_event(EventLoop* loop, WindowEvent* event) {
    if (!loop || !event) return false;
    
    SDL_LockMutex(loop->event_mutex);
    
    // Vérifier s'il y a des événements
    if (loop->buffer_count == 0) {
        SDL_UnlockMutex(loop->event_mutex);
        return false; // Pas d'événement
    }
    
    // Récupérer l'événement du buffer
    *event = loop->event_buffer[loop->buffer_tail];
    loop->buffer_tail = (loop->buffer_tail + 1) % loop->buffer_size;
    loop->buffer_count--;
    
    SDL_UnlockMutex(loop->event_mutex);
    return true;
}

// === FONCTIONS DU CORE MODIFIÉES ===

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
    
    // 🆕 Créer la boucle d'événements MAIS ne pas la démarrer encore
    core->event_loop = event_loop_create(core->event_manager);
    if (!core->event_loop) {
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    // 🔧 FIX: Créer et INITIALISER la scène home immédiatement avec vérifications
    printf("🏠 Création de la scène home...\n");
    Scene* home_scene = create_home_scene();
    if (!home_scene) {
        printf("❌ Erreur: Impossible de créer la scène home\n");
        event_loop_destroy(core->event_loop);
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
        event_loop_destroy(core->event_loop);
        scene_manager_destroy(core->scene_manager);
        event_manager_destroy(core->event_manager);
        free(core);
        return NULL;
    }
    
    // 🆕 Vérifier que la scène a été correctement définie
    Scene* verification_scene = scene_manager_get_current_scene(core->scene_manager);
    if (!verification_scene) {
        printf("❌ Erreur: Scène non définie après scene_manager_set_scene\n");
        event_loop_destroy(core->event_loop);
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

// 🆕 Nouvelle fonction pour finaliser l'initialisation
bool game_core_finalize_init(GameCore* core) {
    if (!core) {
        printf("❌ Core est NULL\n");
        return false;
    }
    
    if (!core->scene_manager) {
        printf("❌ Scene manager est NULL\n");
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
    
    // Connexion des événements
    if (current_scene->data) {
        printf("🔗 Connexion des événements de la scène '%s'...\n", current_scene->name);
        home_scene_connect_events(current_scene, core);
        printf("✅ Événements de la scène connectés\n");
        
        // 🆕 DEBUG: Vérifier combien d'éléments sont enregistrés
        printf("🔍 Debug: Vérification des éléments enregistrés...\n");
        log_console_debug_event_manager(core->event_manager);
        
    } else {
        printf("❌ Scène non initialisée\n");
        return false;
    }
    
    // Activer le tracking souris
    log_console_set_mouse_tracking(true);
    printf("🖱️ Tracking souris activé\n");
    
    // Démarrer la boucle d'événements
    if (!event_loop_start(core->event_loop)) {
        printf("❌ Impossible de démarrer la boucle d'événements\n");
        return false;
    }
    
    log_console_write("EventLoop", "SystemReady", "core.c", 
                     "[core.c] Event system fully operational");
    
    printf("✅ Core complètement initialisé avec boucle d'événements active\n");
    return true;
}

void game_core_destroy(GameCore* core) {
    if (!core) return;
    
    // 🆕 Arrêter la boucle d'événements en premier
    if (core->event_loop) {
        event_loop_destroy(core->event_loop);
    }
    
    if (core->scene_manager) {
        scene_manager_destroy(core->scene_manager);
    }
    
    if (core->event_manager) {
        event_manager_destroy(core->event_manager);
    }
    
    free(core);
}

// 🆕 NOUVELLE FONCTION : Traitement des événements depuis le buffer (LOGS RÉDUITS)
void game_core_handle_events(GameCore* core) {
    if (!core || !core->event_loop || !core->event_manager) return;
    
    WindowEvent window_event;
    int critical_events = 0;
    
    while (event_loop_pop_event(core->event_loop, &window_event)) {
        SDL_Event* event = &window_event.sdl_event;
        
        // 🔧 FIX: Classification correcte des événements
        bool is_critical = false;
        
        switch (event->type) {
            case SDL_MOUSEBUTTONDOWN: // 1024 - VRAI CLIC
                is_critical = true;
                break;
            case SDL_MOUSEBUTTONUP: // 1025 - Important pour UI mais pas critique
                is_critical = false;
                break;
            case SDL_WINDOWEVENT: // 512
                if (event->window.event == SDL_WINDOWEVENT_CLOSE) {
                    is_critical = true;
                }
                break;
            case SDL_QUIT: // 256
                is_critical = true;
                break;
            default:
                is_critical = false;
                break;
        }
        
        if (is_critical) {
            critical_events++;
            log_console_write_event("CoreEvents", "Processing", "core", 
                                   "[core.c] Processing critical event", event->type);
        }
        
        // Transmission vers Event Manager de TOUS les événements
        event_manager_handle_event(core->event_manager, event);
        
        if (!event_manager_is_running(core->event_manager)) {
            core->running = false;
            break;
        }
    }
    
    // LOG RÉSUMÉ SEULEMENT SI ÉVÉNEMENTS CRITIQUES
    if (critical_events > 0) {
        char message[128];
        snprintf(message, sizeof(message), 
                "[core.c] Processed %d critical events", critical_events);
        log_console_write("CoreEvents", "Summary", "core", message);
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