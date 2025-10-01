#include "scene.h"
#include <stdlib.h>
#include <stdio.h>

// === EXEMPLES DE SCÈNES ===

// Données pour le menu principal
typedef struct MainMenuData {
    int selected_option;
} MainMenuData;

// Fonctions pour le menu principal
static void main_menu_init(Scene* scene) {
    printf("Initialisation du menu principal\n");
    MainMenuData* data = (MainMenuData*)malloc(sizeof(MainMenuData));
    data->selected_option = 0;
    scene->data = data;
}

static void main_menu_update(Scene* scene, float delta_time) {
    (void)scene;
    (void)delta_time;
    // Logique de mise à jour du menu
}

static void main_menu_render(Scene* scene, GameWindow* window) {
    (void)scene;
    if (!window) return;
    
    // Rendre un fond bleu pour le menu
    SDL_Renderer* renderer = window_get_renderer(window);
    SDL_SetRenderDrawColor(renderer, 0, 100, 200, 255);
    SDL_RenderClear(renderer);
}

static void main_menu_cleanup(Scene* scene) {
    printf("Nettoyage du menu principal\n");
    if (scene->data) {
        free(scene->data);
        scene->data = NULL;
    }
}

// Créer la scène du menu principal
Scene* create_main_menu_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->name = "Menu Principal";
    scene->init = main_menu_init;
    scene->update = main_menu_update;
    scene->render = main_menu_render;
    scene->cleanup = main_menu_cleanup;
    scene->data = NULL;
    
    return scene;
}