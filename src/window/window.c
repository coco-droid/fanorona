#include "window.h"
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>

// Pointeurs globaux pour les fenêtres
static GameWindow* g_main_window = NULL;
static GameWindow* g_mini_window = NULL;
static WindowType g_active_window_type = WINDOW_TYPE_MINI; // Par défaut mini fenêtre

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
    
    return true;
}

// Quitter SDL
void window_quit_sdl(void) {
    IMG_Quit();
    SDL_Quit();
}

// Créer une fenêtre générique
GameWindow* window_create(const char* title, int width, int height) {
    GameWindow* game_window = (GameWindow*)malloc(sizeof(GameWindow));
    if (!game_window) {
        printf("Erreur: Impossible d'allouer la mémoire pour la fenêtre\n");
        return NULL;
    }
    
    // Créer la fenêtre SDL
    game_window->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );
    
    if (!game_window->window) {
        printf("Erreur de création de la fenêtre: %s\n", SDL_GetError());
        free(game_window);
        return NULL;
    }
    
    // Créer le renderer
    game_window->renderer = SDL_CreateRenderer(
        game_window->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!game_window->renderer) {
        printf("Erreur de création du renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(game_window->window);
        free(game_window);
        return NULL;
    }
    
    game_window->width = width;
    game_window->height = height;
    game_window->title = title;
    
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
    return window_create("Fanorona - Mini Window", 700, 500);
}

// Créer la large fenêtre
GameWindow* create_large_window(void) {
    return window_create("Fanorona - Game Window", 800, 600);
}

// Initialiser les fenêtres globales
void window_initialize_global_windows(void) {
    // Par défaut, on ne crée que la mini fenêtre
    if (!g_mini_window) {
        g_mini_window = create_mini_window();
        if (!g_mini_window) {
            printf("Erreur: Impossible de créer la mini fenêtre\n");
        }
    }
    
    // La fenêtre principale n'est créée que si demandée explicitement
    printf("Fenêtre active par défaut: Mini (400x300)\n");
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