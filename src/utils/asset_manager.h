#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
// Fonctions utilitaires pour gérer les assets
char* asset_get_full_path(const char* filename);
SDL_Texture* asset_load_texture(SDL_Renderer* renderer, const char* filename);
bool asset_file_exists(const char* filename);

#endif // ASSET_MANAGER_H