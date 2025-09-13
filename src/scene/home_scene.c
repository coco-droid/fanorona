#include "./scene.h"
#include "../utils/asset_manager.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Données pour la scène home
typedef struct HomeSceneData {
    bool initialized;
    SDL_Texture* background_texture;
} HomeSceneData;

// Créer une texture de background par défaut
static SDL_Texture* create_default_background(SDL_Renderer* renderer, int width, int height) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        printf("Erreur lors de la création de la texture par défaut: %s\n", SDL_GetError());
        return NULL;
    }
    
    // Définir la texture comme cible de rendu
    SDL_SetRenderTarget(renderer, texture);
    
    // Créer un dégradé bleu-vert
    for (int y = 0; y < height; y++) {
        int blue = 100 + (y * 155) / height;   // De 100 à 255
        int green = 50 + (y * 100) / height;   // De 50 à 150
        SDL_SetRenderDrawColor(renderer, 30, green, blue, 255);
        SDL_RenderDrawLine(renderer, 0, y, width, y);
    }
    
    // Remettre le rendu sur la fenêtre
    SDL_SetRenderTarget(renderer, NULL);
    
    printf("Texture de background par défaut créée (%dx%d)\n", width, height);
    return texture;
}

// Initialisation de la scène home
static void home_scene_init(Scene* scene) {
    printf("Initialisation de la scène Home\n");
    
    HomeSceneData* data = (HomeSceneData*)malloc(sizeof(HomeSceneData));
    if (!data) {
        printf("Erreur: Impossible d'allouer la mémoire pour HomeSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->background_texture = NULL;
    
    // Charger l'image de fond
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            // Essayer de charger l'image avec l'asset manager
            data->background_texture = asset_load_texture(renderer, "fix_bg.png");
            
            // Si aucune texture n'a été créée, créer un background par défaut
            if (!data->background_texture) {
                printf("Création d'un background par défaut\n");
                data->background_texture = create_default_background(renderer, 600, 500);
            }
        }
    }
    
    scene->data = data;
}

// Mise à jour de la scène home
static void home_scene_update(Scene* scene, float delta_time) {
    (void)scene;
    (void)delta_time;
    // Pas de logique de mise à jour pour l'instant
}

// Rendu de la scène home
static void home_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    HomeSceneData* data = (HomeSceneData*)scene->data;
    
    // Afficher le background si disponible
    if (data && data->background_texture) {
        // Afficher le background en plein écran
        SDL_RenderCopy(renderer, data->background_texture, NULL, NULL);
    } else {
        // Fond jaune par défaut si pas d'image
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Jaune (R=255, G=255, B=0)
        SDL_RenderClear(renderer);
    }
    
    // Dessiner le texte "FANORONA" par-dessus le background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir pour le texte
    
    // Position centrée dans la mini fenêtre (600x500)
    int center_x = 300;
    int center_y = 250;
    int letter_width = 20;
    int letter_height = 30;
    int spacing = 5;
    
    // F
    SDL_Rect f_rects[] = {
        {center_x - 80, center_y - 15, letter_width, 5},  // ligne horizontale haut
        {center_x - 80, center_y - 15, 5, letter_height}, // ligne verticale gauche
        {center_x - 80, center_y - 5, 15, 5}              // ligne horizontale milieu
    };
    for (int i = 0; i < 3; i++) {
        SDL_RenderFillRect(renderer, &f_rects[i]);
    }
    
    // A
    int a_x = center_x - 80 + letter_width + spacing;
    SDL_Rect a_rects[] = {
        {a_x, center_y - 15, letter_width, 5},      // ligne horizontale haut
        {a_x, center_y - 15, 5, letter_height},     // ligne verticale gauche
        {a_x + 15, center_y - 15, 5, letter_height}, // ligne verticale droite
        {a_x, center_y - 5, letter_width, 5}        // ligne horizontale milieu
    };
    for (int i = 0; i < 4; i++) {
        SDL_RenderFillRect(renderer, &a_rects[i]);
    }
    
    // N
    int n_x = a_x + letter_width + spacing;
    SDL_Rect n_rects[] = {
        {n_x, center_y - 15, 5, letter_height},     // ligne verticale gauche
        {n_x + 15, center_y - 15, 5, letter_height}, // ligne verticale droite
        {n_x + 5, center_y - 10, 10, 5}             // ligne diagonale (approximative)
    };
    for (int i = 0; i < 3; i++) {
        SDL_RenderFillRect(renderer, &n_rects[i]);
    }
    
    // O
    int o_x = n_x + letter_width + spacing;
    SDL_Rect o_rects[] = {
        {o_x, center_y - 15, letter_width, 5},      // ligne horizontale haut
        {o_x, center_y + 10, letter_width, 5},      // ligne horizontale bas
        {o_x, center_y - 15, 5, letter_height},     // ligne verticale gauche
        {o_x + 15, center_y - 15, 5, letter_height} // ligne verticale droite
    };
    for (int i = 0; i < 4; i++) {
        SDL_RenderFillRect(renderer, &o_rects[i]);
    }
    
    // R
    int r_x = o_x + letter_width + spacing;
    SDL_Rect r_rects[] = {
        {r_x, center_y - 15, letter_width, 5},      // ligne horizontale haut
        {r_x, center_y - 15, 5, letter_height},     // ligne verticale gauche
        {r_x + 15, center_y - 15, 5, 15},           // ligne verticale droite (haut)
        {r_x, center_y - 5, 15, 5},                 // ligne horizontale milieu
        {r_x + 10, center_y, 10, 15}                // ligne diagonale bas
    };
    for (int i = 0; i < 5; i++) {
        SDL_RenderFillRect(renderer, &r_rects[i]);
    }
    
    // O (deuxième)
    int o2_x = r_x + letter_width + spacing;
    SDL_Rect o2_rects[] = {
        {o2_x, center_y - 15, letter_width, 5},      // ligne horizontale haut
        {o2_x, center_y + 10, letter_width, 5},      // ligne horizontale bas
        {o2_x, center_y - 15, 5, letter_height},     // ligne verticale gauche
        {o2_x + 15, center_y - 15, 5, letter_height} // ligne verticale droite
    };
    for (int i = 0; i < 4; i++) {
        SDL_RenderFillRect(renderer, &o2_rects[i]);
    }
    
    // N (deuxième)
    int n2_x = o2_x + letter_width + spacing;
    SDL_Rect n2_rects[] = {
        {n2_x, center_y - 15, 5, letter_height},     // ligne verticale gauche
        {n2_x + 15, center_y - 15, 5, letter_height}, // ligne verticale droite
        {n2_x + 5, center_y - 10, 10, 5}             // ligne diagonale (approximative)
    };
    for (int i = 0; i < 3; i++) {
        SDL_RenderFillRect(renderer, &n2_rects[i]);
    }
    
    // A (deuxième)
    int a2_x = n2_x + letter_width + spacing;
    SDL_Rect a2_rects[] = {
        {a2_x, center_y - 15, letter_width, 5},      // ligne horizontale haut
        {a2_x, center_y - 15, 5, letter_height},     // ligne verticale gauche
        {a2_x + 15, center_y - 15, 5, letter_height}, // ligne verticale droite
        {a2_x, center_y - 5, letter_width, 5}        // ligne horizontale milieu
    };
    for (int i = 0; i < 4; i++) {
        SDL_RenderFillRect(renderer, &a2_rects[i]);
    }
}

// Nettoyage de la scène home
static void home_scene_cleanup(Scene* scene) {
    printf("Nettoyage de la scène Home\n");
    if (scene->data) {
        HomeSceneData* data = (HomeSceneData*)scene->data;
        
        // Libérer la texture du background
        if (data->background_texture) {
            SDL_DestroyTexture(data->background_texture);
            data->background_texture = NULL;
        }
        
        free(scene->data);
        scene->data = NULL;
    }
}

// Créer la scène home
Scene* create_home_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("Erreur: Impossible d'allouer la mémoire pour la scène Home\n");
        return NULL;
    }
    
    scene->name = "Home";
    scene->init = home_scene_init;
    scene->update = home_scene_update;
    scene->render = home_scene_render;
    scene->cleanup = home_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}