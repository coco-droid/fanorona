#include "window_manager.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// Rayon par défaut pour les coins arrondis
#define DEFAULT_CORNER_RADIUS 12

// Variable globale pour stocker l'icône
static SDL_Surface *app_icon = NULL;

// Fonction pour dessiner un cercle rempli (pour les coins arrondis)
static void draw_filled_circle(SDL_Renderer *renderer, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

static SDL_Texture *create_corner_mask(SDL_Renderer *renderer, int radius, int w, int h) {
    SDL_Texture *mask = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                         SDL_TEXTUREACCESS_TARGET, w, h);
    if (!mask) return NULL;
    
    // Sauvegarder le target actuel
    SDL_Texture *old_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, mask);
    
    // Clear avec transparent (noir transparent)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    // Dessiner le masque en blanc opaque
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    // Rectangle principal (sans les coins)
    SDL_Rect center_rect = {radius, 0, w - 2*radius, h};
    SDL_RenderFillRect(renderer, &center_rect);
    
    // Rectangles verticaux pour les côtés
    SDL_Rect left_rect = {0, radius, radius, h - 2*radius};
    SDL_Rect right_rect = {w - radius, radius, radius, h - 2*radius};
    SDL_RenderFillRect(renderer, &left_rect);
    SDL_RenderFillRect(renderer, &right_rect);
    
    // Dessiner les coins arrondis avec des cercles
    draw_filled_circle(renderer, radius, radius, radius);           // Top-left
    draw_filled_circle(renderer, w - radius, radius, radius);       // Top-right
    draw_filled_circle(renderer, radius, h - radius, radius);       // Bottom-left
    draw_filled_circle(renderer, w - radius, h - radius, radius);   // Bottom-right
    
    // Restaurer le target
    SDL_SetRenderTarget(renderer, old_target);
    return mask;
}

// Fonction pour créer une fenêtre avec coins arrondis (inspirée de votre CreateRoundedWindow)
static SDL_Window *create_rounded_window(const char *title, int x, int y, int w, int h) {
    // Créer une fenêtre sans bordures pour pouvoir dessiner nos propres coins
    SDL_Window *window = SDL_CreateWindow(title, x, y, w, h, 
                                         SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
    
    if (!window) {
        return NULL;
    }
    
    return window;
}

static SDL_Surface *create_default_icon(void) {
    // Créer une surface 32x32 avec canal alpha
    SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormat(0, 32, 32, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!icon) return NULL;
    
    // Remplir avec une couleur (bleu foncé avec bordure blanche)
    SDL_FillRect(icon, NULL, SDL_MapRGBA(icon->format, 30, 60, 120, 255));
    
    // Ajouter une bordure blanche
    for (int i = 0; i < 2; i++) {
        Uint32 white = SDL_MapRGBA(icon->format, 255, 255, 255, 255);
        // Dessiner le contour
        SDL_Rect top = {i, i, 32-2*i, 1};
        SDL_Rect bottom = {i, 31-i, 32-2*i, 1};
        SDL_Rect left = {i, i, 1, 32-2*i};
        SDL_Rect right = {31-i, i, 1, 32-2*i};
        
        SDL_FillRect(icon, &top, white);
        SDL_FillRect(icon, &bottom, white);
        SDL_FillRect(icon, &left, white);
        SDL_FillRect(icon, &right, white);
    }
    
    return icon;
}

static void load_application_icon(void) {
    if (app_icon) return; // Déjà chargée
    
    // Initialiser SDL_image si pas déjà fait
    static bool img_initialized = false;
    if (!img_initialized) {
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            printf("Warning: Could not initialize SDL_image PNG support: %s\n", IMG_GetError());
            // Créer une icône par défaut si SDL_image n'est pas disponible
            app_icon = create_default_icon();
            if (app_icon) {
                printf("Default application icon created\n");
            }
            return;
        }
        img_initialized = true;
    }
    
    // Liste des chemins possibles pour l'icône
    const char *icon_paths[] = {
        "icone.png",                    // Dans le dossier courant
        "../icone.png",                 // Un niveau au-dessus (si on est dans build/)
        "assets/icone.png",             // Dans le dossier assets
        "../assets/icone.png",          // assets/ depuis build/
        "resources/icone.png",          // Dans le dossier resources
        "../resources/icone.png",       // resources/ depuis build/
        "/home/t4x/Desktop/fanorona/icone.png",  // Chemin absolu
        NULL
    };
    
    // Essayer chaque chemin
    for (int i = 0; icon_paths[i] != NULL; i++) {
        app_icon = IMG_Load(icon_paths[i]);
        if (app_icon) {
            printf("Application icon loaded successfully from: %s\n", icon_paths[i]);
            return;
        }
        printf("Tried: %s - %s\n", icon_paths[i], IMG_GetError());
    }
    
    // Si aucune icône trouvée, créer une icône par défaut
    printf("Warning: Could not load application icon from any location, creating default icon\n");
    app_icon = create_default_icon();
    if (app_icon) {
        printf("Default application icon created\n");
    }
}

void wm_load_icon(void) {
    load_application_icon();
}

bool wm_init(WindowManager *wm) {
    if (!wm) return false;
    
    memset(wm, 0, sizeof(WindowManager));
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }
    
    // Configuration pour rendu net et qualité
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Linear filtering pour des coins plus lisses
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "0");
    
    // Charger l'icône de l'application
    load_application_icon();
    
    wm->initialized = true;
    wm->active_window = NULL;
    
    return true;
}

void wm_quit(WindowManager *wm) {
    if (!wm || !wm->initialized) return;
    
    for (int i = 0; i < WINDOW_COUNT; i++) {
        wm_destroy_window(wm, (WindowType)i);
    }
    
    // Libérer l'icône
    if (app_icon) {
        SDL_FreeSurface(app_icon);
        app_icon = NULL;
    }
    
    // Quitter SDL_image
    IMG_Quit();
    SDL_Quit();
    wm->initialized = false;
}

GameWindow *wm_create_window(WindowManager *wm, WindowType type, const char *title, 
                            int w, int h, bool rounded_corners, int corner_radius) {
    if (!wm || !wm->initialized || type >= WINDOW_COUNT) return NULL;
    
    // Supprimer l'avertissement du paramètre inutilisé
    (void)rounded_corners; // Paramètre ignoré car toutes les fenêtres ont des coins arrondis
    
    GameWindow *win = &wm->windows[type];
    
    // Détruire fenêtre existante si nécessaire
    if (win->window) {
        wm_destroy_window(wm, type);
    }
    
    // Force les coins arrondis par défaut pour toutes les fenêtres
    bool use_rounded_corners = true;
    int use_corner_radius = (corner_radius > 0) ? corner_radius : DEFAULT_CORNER_RADIUS;
    
    // Créer la fenêtre avec coins arrondis (toujours borderless maintenant)
    win->window = create_rounded_window(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h);
    
    if (!win->window) return NULL;
    
    // Appliquer l'icône à la fenêtre si elle est disponible
    if (app_icon) {
        SDL_SetWindowIcon(win->window, app_icon);
    }
    
    // Créer le renderer avec configuration optimisée
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    win->renderer = SDL_CreateRenderer(win->window, -1, render_flags);
    if (!win->renderer) {
        SDL_DestroyWindow(win->window);
        win->window = NULL;
        return NULL;
    }
    
    // Configuration pour transparence et blend mode
    SDL_SetRenderDrawBlendMode(win->renderer, SDL_BLENDMODE_BLEND);
    
    // Initialiser les propriétés
    win->type = type;
    win->width = w;
    win->height = h;
    win->visible = true;
    win->has_rounded_corners = use_rounded_corners;  // Toujours true
    win->corner_radius = use_corner_radius;          // Utilise le rayon par défaut ou fourni
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';
    
    // Créer le masque pour coins arrondis (maintenant obligatoire)
    win->corner_mask = create_corner_mask(win->renderer, use_corner_radius, w, h);
    if (!win->corner_mask) {
        printf("Warning: Failed to create corner mask for window %s\n", title);
    }
    
    // Définir comme fenêtre active si c'est la première
    if (!wm->active_window) {
        wm->active_window = win;
    }
    
    return win;
}

void wm_destroy_window(WindowManager *wm, WindowType type) {
    if (!wm || type >= WINDOW_COUNT) return;
    
    GameWindow *win = &wm->windows[type];
    
    if (win->corner_mask) {
        SDL_DestroyTexture(win->corner_mask);
        win->corner_mask = NULL;
    }
    
    if (win->renderer) {
        SDL_DestroyRenderer(win->renderer);
        win->renderer = NULL;
    }
    
    if (win->window) {
        SDL_DestroyWindow(win->window);
        win->window = NULL;
    }
    
    if (wm->active_window == win) {
        wm->active_window = NULL;
    }
    
    memset(win, 0, sizeof(GameWindow));
}

void wm_show_window(WindowManager *wm, WindowType type) {
    GameWindow *win = wm_get_window(wm, type);
    if (win && win->window) {
        SDL_ShowWindow(win->window);
        win->visible = true;
    }
}

void wm_hide_window(WindowManager *wm, WindowType type) {
    GameWindow *win = wm_get_window(wm, type);
    if (win && win->window) {
        SDL_HideWindow(win->window);
        win->visible = false;
    }
}

void wm_set_active_window(WindowManager *wm, WindowType type) {
    if (!wm) return;
    GameWindow *win = wm_get_window(wm, type);
    if (win && win->window) {
        wm->active_window = win;
        SDL_RaiseWindow(win->window);
    }
}

GameWindow *wm_get_window(WindowManager *wm, WindowType type) {
    if (!wm || type >= WINDOW_COUNT) return NULL;
    GameWindow *win = &wm->windows[type];
    return win->window ? win : NULL;
}

void wm_render_window(GameWindow *win, void (*render_callback)(SDL_Renderer *)) {
    // 1. Crée une texture temporaire
    // 2. Rend le contenu sur cette texture
    // 3. Applique le masque de coins arrondis
    // 4. Affiche le résultat final
    if (!win || !win->renderer || !render_callback) return;
    
    // Maintenant toutes les fenêtres utilisent les coins arrondis
    if (win->corner_mask) {
        // Créer une texture temporaire pour le rendu
        SDL_Texture *temp_texture = SDL_CreateTexture(win->renderer, 
                                                     SDL_PIXELFORMAT_RGBA8888,
                                                     SDL_TEXTUREACCESS_TARGET, 
                                                     win->width, win->height);
        if (!temp_texture) {
            // Fallback: rendu normal
            render_callback(win->renderer);
            SDL_RenderPresent(win->renderer);
            return;
        }
        
        // Sauvegarder le target actuel
        SDL_Texture *old_target = SDL_GetRenderTarget(win->renderer);
        
        // Rendre sur la texture temporaire
        SDL_SetRenderTarget(win->renderer, temp_texture);
        SDL_SetRenderDrawColor(win->renderer, 0, 0, 0, 0); // Transparent
        SDL_RenderClear(win->renderer);
        
        // Callback de rendu sur la texture
        render_callback(win->renderer);
        
        // Restaurer le target principal
        SDL_SetRenderTarget(win->renderer, old_target);
        
        // Clear du renderer principal
        SDL_SetRenderDrawColor(win->renderer, 0, 0, 0, 0);
        SDL_RenderClear(win->renderer);
        
        // Appliquer le masque à la texture
        SDL_SetTextureBlendMode(temp_texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(win->corner_mask, SDL_BLENDMODE_MOD);
        
        // D'abord dessiner le masque
        SDL_RenderCopy(win->renderer, win->corner_mask, NULL, NULL);
        
        // Puis dessiner la texture avec le masque appliqué
        SDL_SetTextureBlendMode(temp_texture, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(win->renderer, temp_texture, NULL, NULL);
        
        // Nettoyer
        SDL_DestroyTexture(temp_texture);
    } else {
        // Fallback si le masque n'a pas pu être créé
        SDL_SetRenderDrawColor(win->renderer, 0, 0, 0, 255);
        SDL_RenderClear(win->renderer);
        render_callback(win->renderer);
    }
    
    SDL_RenderPresent(win->renderer);
}

bool wm_handle_window_events(WindowManager *wm, SDL_Event *e) {
    if (!wm || !e) return false;
    
    if (e->type == SDL_WINDOWEVENT) {
        Uint32 windowID = e->window.windowID;
        
        for (int i = 0; i < WINDOW_COUNT; i++) {
            GameWindow *win = &wm->windows[i];
            if (win->window && SDL_GetWindowID(win->window) == windowID) {
                switch (e->window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        wm_hide_window(wm, (WindowType)i);
                        return true;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        wm->active_window = win;
                        return true;
                }
                break;
            }
        }
    }
    
    return false;
}
