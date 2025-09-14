#define _POSIX_C_SOURCE 200809L
#include "asset_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Vérifier si un fichier existe
bool asset_file_exists(const char* filename) {
    if (!filename) return false;
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

// Obtenir le chemin complet vers un asset
char* asset_get_full_path(const char* filename) {
    static char full_path[2048]; // Augmenter la taille pour éviter la troncature
    
    if (!filename) return NULL;
    
    // Vérifier la longueur du nom de fichier
    if (strlen(filename) > 1000) {
        printf("Nom de fichier trop long: %s\n", filename);
        return NULL;
    }
    
    // Essayer d'abord le chemin relatif simple
    snprintf(full_path, sizeof(full_path), "assets/%s", filename);
    if (asset_file_exists(full_path)) {
        printf("Asset trouvé: %s\n", full_path);
        return full_path;
    }
    
    // Essayer depuis un niveau au-dessus (si on est dans build/)
    snprintf(full_path, sizeof(full_path), "../assets/%s", filename);
    if (asset_file_exists(full_path)) {
        printf("Asset trouvé: %s\n", full_path);
        return full_path;
    }
    
    // Essayer avec le chemin base de SDL
    char* base_path = SDL_GetBasePath();
    if (base_path) {
        int result = snprintf(full_path, sizeof(full_path), "%sassets/%s", base_path, filename);
        SDL_free(base_path);
        
        // Vérifier si le chemin a été tronqué
        if (result >= 0 && result < (int)sizeof(full_path)) {
            if (asset_file_exists(full_path)) {
                printf("Asset trouvé: %s\n", full_path);
                return full_path;
            }
        }
    }
    
    // Essayer le chemin absolu depuis le répertoire de travail
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        // Vérifier que nous avons assez de place pour le chemin complet
        size_t cwd_len = strlen(cwd);
        size_t filename_len = strlen(filename);
        size_t total_len = cwd_len + 9 + filename_len; // "/assets/" = 9 caractères
        
        if (total_len < sizeof(full_path)) {
            int result = snprintf(full_path, sizeof(full_path), "%s/assets/%s", cwd, filename);
            
            // Vérifier si le chemin a été tronqué
            if (result >= 0 && result < (int)sizeof(full_path)) {
                if (asset_file_exists(full_path)) {
                    printf("Asset trouvé: %s\n", full_path);
                    return full_path;
                }
            }
        } else {
            printf("Chemin trop long pour %s depuis %s\n", filename, cwd);
        }
    }
    
    // Retourner le chemin simple par défaut
    snprintf(full_path, sizeof(full_path), "assets/%s", filename);
    printf("Asset non trouvé, utilisation du chemin par défaut: %s\n", full_path);
    return full_path;
}

// Charger une texture depuis un fichier
SDL_Texture* asset_load_texture(SDL_Renderer* renderer, const char* filename) {
    if (!renderer || !filename) return NULL;
    
    char* full_path = asset_get_full_path(filename);
    if (!full_path) return NULL;
    
    SDL_Surface* surface = IMG_Load(full_path);
    if (!surface) {
        printf("Erreur lors du chargement de %s: %s\n", full_path, IMG_GetError());
        return NULL;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        printf("Erreur lors de la création de la texture: %s\n", SDL_GetError());
        return NULL;
    }
    
    printf("Texture chargée avec succès: %s\n", full_path);
    return texture;
}