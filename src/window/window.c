#include "window.h"
#include "../utils/log_console.h"
#include "../config.h" // <-- ajout
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h> // 🔧 Inclure SDL_ttf.h pour TTF
#include <stdlib.h>
#include <stdio.h>

// Pointeurs globaux pour les fenêtres
static GameWindow* g_main_window = NULL;
static GameWindow* g_mini_window = NULL;
static WindowType g_active_window_type = WINDOW_TYPE_MINI; // Par défaut mini fenêtre
static GameWindow* g_focused_window = NULL; // 🆕 Fenêtre ayant le focus

// Initialiser SDL
bool window_init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur d'initialisation SDL: %s\n", SDL_GetError());
        return false;
    }
    
    // Initialiser SDL_image
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        printf("Erreur d'initialisation SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }
    
    // 🔧 FIX PRINCIPAL: Initialiser SDL_TTF pour le rendu du texte
    if (TTF_Init() == -1) {
        printf("Erreur d'initialisation SDL_TTF: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    printf("✅ SDL, SDL_image et SDL_TTF initialisés avec succès\n");
    return true;
}

// Quitter SDL
void window_quit_sdl(void) {
    TTF_Quit();  // 🔧 Ajouter le nettoyage TTF
    IMG_Quit();
    SDL_Quit();
    printf("✅ SDL, SDL_image et SDL_TTF fermés proprement\n");
}

// Créer une fenêtre générique (AVEC VSYNC FORCÉ)
GameWindow* window_create(const char* title, int width, int height) {
    // 🔧 FIX: Vérifier si SDL est déjà initialisé
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        printf("⚠️ SDL n'est pas initialisé, initialisation tardive en cours...\n");
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            printf("❌ Erreur d'initialisation SDL tardive: %s\n", SDL_GetError());
            return NULL;
        }
    }
    
    // 🔧 FIX: Définir des hints SDL pour éviter les problèmes d'événements X11
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    
    GameWindow* game_window = (GameWindow*)malloc(sizeof(GameWindow));
    if (!game_window) {
        printf("Erreur: Impossible d'allouer la mémoire pour la fenêtre\n");
        return NULL;
    }
    
    // Créer la fenêtre SDL
    game_window->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,  // 🔧 FIX: Utiliser UNDEFINED au lieu de CENTERED
        SDL_WINDOWPOS_UNDEFINED,  // pour éviter les problèmes de positionnement X11
        width,
        height,
        SDL_WINDOW_SHOWN
    );
    
    if (!game_window->window) {
        printf("Erreur de création de la fenêtre: %s\n", SDL_GetError());
        free(game_window);
        return NULL;
    }
    
    // 🔧 FIX: Traiter immédiatement tous les événements en attente
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Vider simplement la file d'attente des événements
    }
    
    // 🔧 FIX: Permettre au système de fenêtrage de traiter les événements
    SDL_PumpEvents();
    
    // 🆕 Récupérer l'ID unique de la fenêtre
    game_window->window_id = SDL_GetWindowID(game_window->window);
    game_window->has_focus = false;
    
    // 🔧 FIX: Créer le renderer avec VSync OBLIGATOIRE pour éviter le clignotement
    game_window->renderer = SDL_CreateRenderer(
        game_window->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!game_window->renderer) {
        printf("⚠️ Tentative sans VSync...\n");
        // Fallback sans VSync si échec
        game_window->renderer = SDL_CreateRenderer(
            game_window->window,
            -1,
            SDL_RENDERER_ACCELERATED
        );
    }
    
    if (!game_window->renderer) {
        printf("Erreur de création du renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(game_window->window);
        free(game_window);
        return NULL;
    }
    
    // 🔧 FIX: Forcer le blend mode pour les transparences
    SDL_SetRenderDrawBlendMode(game_window->renderer, SDL_BLENDMODE_BLEND);
    
    game_window->width = width;
    game_window->height = height;
    game_window->title = title;
    
    // 🆕 Logs de création de fenêtre
    char message[256];
    snprintf(message, sizeof(message), 
            "[window.c] Window created with VSync: '%s' (%dx%d) ID=%u", 
            title, width, height, game_window->window_id);
    log_console_write("WindowManager", "WindowCreated", "window.c", message);
    
    return game_window;
}

// Détruire une fenêtre
void window_destroy(GameWindow* window) {
    if (!window) return;
    
    if (window->renderer) {
        SDL_DestroyRenderer(window->renderer);
    }
    
    if (window->window) {
        SDL_DestroyWindow(window->window);
    }
    
    free(window);
}

// Effacer la fenêtre
void window_clear(GameWindow* window) {
    if (!window || !window->renderer) return;
    
    SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 255);
    SDL_RenderClear(window->renderer);
}

// Présenter le contenu rendu
void window_present(GameWindow* window) {
    if (!window || !window->renderer) return;
    
    SDL_RenderPresent(window->renderer);
}

// Obtenir le renderer
SDL_Renderer* window_get_renderer(GameWindow* window) {
    return window ? window->renderer : NULL;
}

// Créer la mini fenêtre
GameWindow* create_mini_window(void) {
    GameWindow* window = window_create("Fanorona - Mini Window", DEFAULT_MINI_WINDOW_WIDTH, DEFAULT_MINI_WINDOW_HEIGHT);
    if (window) {
        printf("🖼️ Mini window créée: %dx%d (depuis config.h)\n", DEFAULT_MINI_WINDOW_WIDTH, DEFAULT_MINI_WINDOW_HEIGHT);
    }
    return window;
}

// Créer la large fenêtre
GameWindow* create_large_window(void) {
    GameWindow* window = window_create("Fanorona - Game Window", DEFAULT_MAIN_WINDOW_WIDTH, DEFAULT_MAIN_WINDOW_HEIGHT);
    if (window) {
        printf("🖼️ Main window créée: %dx%d (depuis config.h)\n", DEFAULT_MAIN_WINDOW_WIDTH, DEFAULT_MAIN_WINDOW_HEIGHT);
    }
    return window;
}

// Initialiser les fenêtres globales
void window_initialize_global_windows(void) {
    // Créer immédiatement la mini fenêtre
    if (!g_mini_window) {
        printf("🔧 Création de la mini fenêtre...\n");
        g_mini_window = create_mini_window();
        if (!g_mini_window) {
            printf("❌ Erreur: Impossible de créer la mini fenêtre\n");
            return;
        }
        printf("✅ Mini fenêtre créée : '%s' (%dx%d)\n", 
               g_mini_window->title, g_mini_window->width, g_mini_window->height);
    }
    
    // Définir la mini fenêtre comme active par défaut
    g_active_window_type = WINDOW_TYPE_MINI;
    printf("🎯 Fenêtre active par défaut: Mini (%dx%d)\n", 
           g_mini_window->width, g_mini_window->height);
    
    // 🆕 Logs de création de fenêtre
    char message[256];
    snprintf(message, sizeof(message), 
            "[window.c] Global windows initialized - Mini window active");
    log_console_write("WindowManager", "GlobalInit", "window.c", message);
}

// Nettoyer les fenêtres globales
void window_cleanup_global_windows(void) {
    if (g_main_window) {
        window_destroy(g_main_window);
        g_main_window = NULL;
    }
    
    if (g_mini_window) {
        window_destroy(g_mini_window);
        g_mini_window = NULL;
    }
}

// Obtenir la fenêtre principale pour les scènes
GameWindow* use_main_window(void) {
    if (!g_main_window) {
        // Créer la fenêtre principale seulement si demandée
        g_main_window = create_large_window();
        if (!g_main_window) {
            printf("Attention: Impossible de créer la fenêtre principale\n");
        }
    }
    return g_main_window;
}

// Obtenir la mini fenêtre pour les scènes
GameWindow* use_mini_window(void) {
    if (!g_mini_window) {
        g_mini_window = create_mini_window();
        if (!g_mini_window) {
            printf("Attention: Impossible de créer la mini fenêtre\n");
        }
    }
    return g_mini_window;
}

// Définir la fenêtre active
void window_set_active_window(WindowType type) {
    WindowType old_type = g_active_window_type;
    g_active_window_type = type;
    
    // Fermer les fenêtres inactives si ce n'est pas BOTH
    if (type != WINDOW_TYPE_BOTH) {
        if (old_type != type) {
            if (type == WINDOW_TYPE_MINI && g_main_window) {
                printf("Fermeture de la fenêtre principale\n");
                window_destroy(g_main_window);
                g_main_window = NULL;
            } else if (type == WINDOW_TYPE_MAIN && g_mini_window) {
                printf("Fermeture de la mini fenêtre\n");
                window_destroy(g_mini_window);
                g_mini_window = NULL;
            }
        }
    }
    
    // Créer la nouvelle fenêtre active si nécessaire
    switch (type) {
        case WINDOW_TYPE_MAIN:
            if (!g_main_window) {
                printf("Ouverture de la fenêtre principale\n");
                use_main_window();
            }
            break;
        case WINDOW_TYPE_MINI:
            if (!g_mini_window) {
                printf("Ouverture de la mini fenêtre\n");
                use_mini_window();
            }
            break;
        case WINDOW_TYPE_BOTH:
            printf("Ouverture des deux fenêtres\n");
            use_main_window();
            use_mini_window();
            break;
    }
}

// Obtenir le type de fenêtre active
WindowType window_get_active_window(void) {
    return g_active_window_type;
}

// Obtenir la fenêtre actuellement active
GameWindow* use_active_window(void) {
    switch (g_active_window_type) {
        case WINDOW_TYPE_MAIN:
            return use_main_window();
        case WINDOW_TYPE_MINI:
            return use_mini_window();
        case WINDOW_TYPE_BOTH:
            // Retourner la fenêtre principale par défaut si les deux sont actives
            return use_main_window();
        default:
            return use_mini_window();
    }
}

// Vérifier si une fenêtre spécifique est active
bool window_is_window_active(WindowType type) {
    if (g_active_window_type == WINDOW_TYPE_BOTH) {
        return true; // Les deux fenêtres sont actives
    }
    return g_active_window_type == type;
}

// 🆕 Déterminer le type d'une fenêtre (main ou mini)
WindowType window_get_window_type(GameWindow* window) {
    if (!window) return WINDOW_TYPE_MINI;  // Default to mini as fallback
    
    if (window == g_main_window) {
        return WINDOW_TYPE_MAIN;
    } else if (window == g_mini_window) {
        return WINDOW_TYPE_MINI;
    }
    
    // If we can't identify it, default to mini window type
    return WINDOW_TYPE_MINI;
}

// 🆕 Obtenir une fenêtre par son ID SDL
GameWindow* window_get_by_id(Uint32 window_id) {
    if (g_main_window && g_main_window->window_id == window_id) {
        return g_main_window;
    }
    if (g_mini_window && g_mini_window->window_id == window_id) {
        return g_mini_window;
    }
    return NULL;
}

// 🆕 Mettre à jour le focus des fenêtres
void window_update_focus(void) {
    SDL_Window* focused_sdl_window = SDL_GetKeyboardFocus();
    
    if (focused_sdl_window) {
        Uint32 focused_id = SDL_GetWindowID(focused_sdl_window);
        GameWindow* new_focused = window_get_by_id(focused_id);
        
        if (new_focused != g_focused_window) {
            // Changer le focus
            if (g_focused_window) {
                g_focused_window->has_focus = false;
                
                char message[256];
                snprintf(message, sizeof(message), 
                        "[window.c] Window lost focus: '%s' ID=%u", 
                        g_focused_window->title, g_focused_window->window_id);
                log_console_write("WindowManager", "FocusLost", "window.c", message);
            }
            
            g_focused_window = new_focused;
            if (g_focused_window) {
                g_focused_window->has_focus = true;
                
                char message[256];
                snprintf(message, sizeof(message), 
                        "[window.c] Window gained focus: '%s' ID=%u", 
                        g_focused_window->title, g_focused_window->window_id);
                log_console_write("WindowManager", "FocusGained", "window.c", message);
            }
        }
    } else {
        // Aucune fenêtre n'a le focus
        if (g_focused_window) {
            g_focused_window->has_focus = false;
            g_focused_window = NULL;
            log_console_write("WindowManager", "AllFocusLost", "window.c", 
                            "[window.c] All windows lost focus");
        }
    }
}

// 🆕 Obtenir la fenêtre ayant le focus
GameWindow* window_get_focused_window(void) {
    return g_focused_window;
}

// 🆕 Vérifier s'il y a des événements en attente
bool window_has_events_pending(void) {
    return SDL_PollEvent(NULL) == 1;
}

// 🆕 FONCTION PRINCIPALE : Capturer les événements avec contexte de fenêtre
bool window_poll_events(WindowEvent* window_event) {
    if (!window_event) return false;
    
    // Initialiser la structure
    window_event->source_window = NULL;
    window_event->window_type = g_active_window_type;
    window_event->is_valid = false;
    
    // Capturer l'événement SDL
    if (!SDL_PollEvent(&window_event->sdl_event)) {
        return false; // Pas d'événement
    }
    
    // 🔍 ANALYSER L'ÉVÉNEMENT ET IDENTIFIER LA FENÊTRE SOURCE
    GameWindow* source_window = NULL;
    WindowType event_window_type = g_active_window_type;
    
    switch (window_event->sdl_event.type) {
        case SDL_WINDOWEVENT:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_TEXTINPUT:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEWHEEL: {
            // Ces événements ont un window ID
            Uint32 event_window_id = 0;
            
            if (window_event->sdl_event.type == SDL_WINDOWEVENT) {
                event_window_id = window_event->sdl_event.window.windowID;
            } else {
                // Pour les autres événements, utiliser la fenêtre qui a le focus
                SDL_Window* focused = SDL_GetKeyboardFocus();
                if (focused) {
                    event_window_id = SDL_GetWindowID(focused);
                }
            }
            
            source_window = window_get_by_id(event_window_id);
            
            if (source_window) {
                // Déterminer le type de fenêtre
                if (source_window == g_main_window) {
                    event_window_type = WINDOW_TYPE_MAIN;
                } else if (source_window == g_mini_window) {
                    event_window_type = WINDOW_TYPE_MINI;
                }
                
                // 🆕 LOGS détaillés par type d'événement
                char message[512];
                switch (window_event->sdl_event.type) {
                    case SDL_MOUSEMOTION:
                        if (log_console_is_mouse_tracking_enabled()) {
                            snprintf(message, sizeof(message), 
                                    "[window.c] Mouse motion in window '%s' at (%d,%d)", 
                                    source_window->title,
                                    window_event->sdl_event.motion.x, 
                                    window_event->sdl_event.motion.y);
                            log_console_write("WindowEvents", "MouseMotion", "window.c", message);
                        }
                        break;
                        
                    case SDL_MOUSEBUTTONDOWN:
                        snprintf(message, sizeof(message), 
                                "[window.c] Mouse button %d down in window '%s' at (%d,%d)", 
                                window_event->sdl_event.button.button,
                                source_window->title,
                                window_event->sdl_event.button.x, 
                                window_event->sdl_event.button.y);
                        log_console_write("WindowEvents", "MouseDown", "window.c", message);
                        break;
                        
                    case SDL_MOUSEBUTTONUP:
                        snprintf(message, sizeof(message), 
                                "[window.c] Mouse button %d up in window '%s' at (%d,%d)", 
                                window_event->sdl_event.button.button,
                                source_window->title,
                                window_event->sdl_event.button.x, 
                                window_event->sdl_event.button.y);
                        log_console_write("WindowEvents", "MouseUp", "window.c", message);
                        break;
                        
                    case SDL_KEYDOWN:
                        snprintf(message, sizeof(message), 
                                "[window.c] Key '%s' down in window '%s'", 
                                SDL_GetKeyName(window_event->sdl_event.key.keysym.sym),
                                source_window->title);
                        log_console_write("WindowEvents", "KeyDown", "window.c", message);
                        break;
                        
                    case SDL_WINDOWEVENT:
                        // 🆕 LOGS DÉTAILLÉS POUR LES ÉVÉNEMENTS DE FENÊTRE
                        const char* window_event_name = "Unknown";
                        switch (window_event->sdl_event.window.event) {
                            case SDL_WINDOWEVENT_CLOSE: window_event_name = "CLOSE"; break;
                            case SDL_WINDOWEVENT_MINIMIZED: window_event_name = "MINIMIZED"; break;
                            case SDL_WINDOWEVENT_MAXIMIZED: window_event_name = "MAXIMIZED"; break;
                            case SDL_WINDOWEVENT_RESTORED: window_event_name = "RESTORED"; break;
                            case SDL_WINDOWEVENT_FOCUS_GAINED: window_event_name = "FOCUS_GAINED"; break;
                            case SDL_WINDOWEVENT_FOCUS_LOST: window_event_name = "FOCUS_LOST"; break;
                            case SDL_WINDOWEVENT_RESIZED: window_event_name = "RESIZED"; break;
                            case SDL_WINDOWEVENT_MOVED: window_event_name = "MOVED"; break;
                            default: window_event_name = "OTHER"; break;
                        }
                        
                        snprintf(message, sizeof(message), 
                                "[window.c] 🚪 Window event %s (%d) in window '%s' ID=%u", 
                                window_event_name,
                                window_event->sdl_event.window.event,
                                source_window->title,
                                source_window->window_id);
                        log_console_write("WindowEvents", "WindowEvent", "window.c", message);
                        
                        // 🆕 LOG SPÉCIAL POUR LA FERMETURE
                        if (window_event->sdl_event.window.event == SDL_WINDOWEVENT_CLOSE) {
                            char close_message[512];
                            snprintf(close_message, sizeof(close_message), 
                                    "[window.c] 🚪🔴 CRITICAL: Window '%s' close button clicked - this will close the application!", 
                                    source_window->title);
                            log_console_write("WindowEvents", "WindowCloseRequest", "window.c", close_message);
                        }
                        break;
                }
            }
            break;
        }
        
        case SDL_QUIT: {
            // 🆕 LOG DÉTAILLÉ POUR SDL_QUIT
            log_console_write("WindowEvents", "Quit", "window.c", 
                            "[window.c] 🚪🔴 SDL_QUIT event received - application shutdown requested");
            // Événement global, pas de fenêtre spécifique
            break;
        }
        
        default:
            // Autres événements globaux
            char message[256];
            snprintf(message, sizeof(message), 
                    "[window.c] Other SDL event received: type=%d", 
                    window_event->sdl_event.type);
            log_console_write("WindowEvents", "OtherEvent", "window.c", message);
            break;
    }
    
    // Remplir la structure de retour
    window_event->source_window = source_window;
    window_event->window_type = event_window_type;
    window_event->is_valid = true;
    
    // 🆕 LOG de résumé si l'événement a une source
    if (source_window) {
        char message[256];
        snprintf(message, sizeof(message), 
                "[window.c] Event from window '%s' (ID=%u) type=%d", 
                source_window->title, source_window->window_id, event_window_type);
        log_console_write("WindowEvents", "EventSourceIdentified", "window.c", message);
    }
    
    return true;
}

// 🆕 NOUVELLES FONCTIONS pour la transmission automatique des dimensions
WindowDimensions window_get_active_dimensions(void) {
    WindowDimensions dims = {700, 500, WINDOW_TYPE_MINI}; // Par défaut
    
    switch (g_active_window_type) {
        case WINDOW_TYPE_MAIN:
            if (g_main_window) {
                dims.width = g_main_window->width;
                dims.height = g_main_window->height;
                dims.type = WINDOW_TYPE_MAIN;
            }
            break;
            
        case WINDOW_TYPE_MINI:
            if (g_mini_window) {
                dims.width = g_mini_window->width;
                dims.height = g_mini_window->height;
                dims.type = WINDOW_TYPE_MINI;
            }
            break;
            
        case WINDOW_TYPE_BOTH:
            // Retourner les dimensions de la fenêtre principale par défaut
            if (g_main_window) {
                dims.width = g_main_window->width;
                dims.height = g_main_window->height;
                dims.type = WINDOW_TYPE_MAIN;
            } else if (g_mini_window) {
                dims.width = g_mini_window->width;
                dims.height = g_mini_window->height;
                dims.type = WINDOW_TYPE_MINI;
            }
            break;
    }
    
    return dims;
}

WindowDimensions window_get_dimensions_for_type(WindowType type) {
    WindowDimensions dims = {DEFAULT_MINI_WINDOW_WIDTH, DEFAULT_MINI_WINDOW_HEIGHT, type}; // Utiliser config.h
    
    switch (type) {
        case WINDOW_TYPE_MAIN:
            if (g_main_window) {
                dims.width = g_main_window->width;
                dims.height = g_main_window->height;
            } else {
                dims.width = DEFAULT_MAIN_WINDOW_WIDTH;  // Utiliser config.h
                dims.height = DEFAULT_MAIN_WINDOW_HEIGHT;
            }
            break;
            
        case WINDOW_TYPE_MINI:
            if (g_mini_window) {
                dims.width = g_mini_window->width;
                dims.height = g_mini_window->height;
            } else {
                dims.width = DEFAULT_MINI_WINDOW_WIDTH;  // Utiliser config.h
                dims.height = DEFAULT_MINI_WINDOW_HEIGHT;
            }
            break;
            
        case WINDOW_TYPE_BOTH:
            // Retourner les dimensions de la mini par défaut
            dims = window_get_dimensions_for_type(WINDOW_TYPE_MINI);
            break;
    }
    
    return dims;
}

// 🆕 Fonction sécurisée pour transition entre fenêtres
// Ouvre d'abord la destination avant de fermer la source
void window_transition_safely(WindowType from_type, WindowType to_type) {
    if (from_type == to_type) {
        return; // Rien à faire si même type
    }
    
    printf("🚀 TRANSITION SÉCURISÉE: %s → %s\n",
           from_type == WINDOW_TYPE_MINI ? "MINI" : "MAIN",
           to_type == WINDOW_TYPE_MINI ? "MINI" : "MAIN");
    
    // 1. D'ABORD créer la fenêtre de destination si elle n'existe pas
    if (to_type == WINDOW_TYPE_MAIN && !g_main_window) {
        printf("🔍 ÉTAPE 1: Création de la fenêtre principale...\n");
        use_main_window();
        if (g_main_window) {
            printf("✅ Fenêtre principale créée avec succès (%dx%d)\n", 
                   g_main_window->width, g_main_window->height);
        } else {
            printf("❌ Échec de la création de la fenêtre principale\n");
            return;
        }
    } else if (to_type == WINDOW_TYPE_MINI && !g_mini_window) {
        printf("🔍 ÉTAPE 1: Création de la mini fenêtre...\n");
        use_mini_window();
        if (g_mini_window) {
            printf("✅ Mini fenêtre créée avec succès (%dx%d)\n", 
                   g_mini_window->width, g_mini_window->height);
        } else {
            printf("❌ Échec de la création de la mini fenêtre\n");
            return;
        }
    }
    
    // 2. Mettre temporairement les deux fenêtres comme actives
    g_active_window_type = WINDOW_TYPE_BOTH;
    printf("🔍 ÉTAPE 2: Les deux fenêtres sont temporairement actives\n");
    
    // 3. Actualiser l'état du rendu pour la nouvelle fenêtre
    printf("🔍 ÉTAPE 3: Mise à jour du contexte de rendu...\n");
    if (to_type == WINDOW_TYPE_MAIN && g_main_window) {
        SDL_SetRenderDrawColor(g_main_window->renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_main_window->renderer);
        SDL_RenderPresent(g_main_window->renderer);
        printf("✅ Contexte de rendu de la fenêtre principale initialisé\n");
    } else if (to_type == WINDOW_TYPE_MINI && g_mini_window) {
        SDL_SetRenderDrawColor(g_mini_window->renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_mini_window->renderer);
        SDL_RenderPresent(g_mini_window->renderer);
        printf("✅ Contexte de rendu de la mini fenêtre initialisé\n");
    }
    
    // 4. SEULEMENT MAINTENANT fermer l'ancienne fenêtre
    printf("🔍 ÉTAPE 4: Fermeture de l'ancienne fenêtre...\n");
    if (from_type == WINDOW_TYPE_MAIN && g_main_window) {
        printf("👋 Fermeture de la fenêtre principale\n");
        window_destroy(g_main_window);
        g_main_window = NULL;
    } else if (from_type == WINDOW_TYPE_MINI && g_mini_window) {
        printf("👋 Fermeture de la mini fenêtre\n");
        window_destroy(g_mini_window);
        g_mini_window = NULL;
    }
    
    // 5. Définir le type final
    g_active_window_type = to_type;
    printf("✅ TRANSITION TERMINÉE: Fenêtre active = %s\n", 
           to_type == WINDOW_TYPE_MINI ? "MINI" : "MAIN");
}