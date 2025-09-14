#include "scene.h"
#include <stdlib.h>
#include <stdio.h>

// Créer un gestionnaire de scènes
SceneManager* scene_manager_create(void) {
    SceneManager* manager = (SceneManager*)malloc(sizeof(SceneManager));
    if (!manager) {
        printf("Erreur: Impossible d'allouer la mémoire pour le gestionnaire de scènes\n");
        return NULL;
    }
    
    manager->current_scene = NULL;
    manager->next_scene = NULL;
    manager->scene_change_requested = false;
    
    return manager;
}

// Détruire le gestionnaire de scènes
void scene_manager_destroy(SceneManager* manager) {
    if (!manager) return;
    
    if (manager->current_scene && manager->current_scene->cleanup) {
        manager->current_scene->cleanup(manager->current_scene);
    }
    
    free(manager);
}

// Changer de scène
bool scene_manager_set_scene(SceneManager* manager, Scene* scene) {
    if (!manager) {
        printf("❌ SceneManager est NULL dans scene_manager_set_scene\n");
        return false;
    }
    
    if (!scene) {
        printf("❌ Scene est NULL dans scene_manager_set_scene\n");
        return false;
    }
    
    printf("🔧 Définition de la scène '%s' comme scène courante...\n", scene->name ? scene->name : "sans nom");
    
    // Nettoyer la scène précédente si elle existe
    if (manager->current_scene && manager->current_scene != scene) {
        printf("🧹 Nettoyage de la scène précédente...\n");
        if (manager->current_scene->cleanup) {
            manager->current_scene->cleanup(manager->current_scene);
        }
        free(manager->current_scene);
    }
    
    manager->current_scene = scene;
    
    // Vérifier que la scène a été correctement assignée
    if (manager->current_scene == scene) {
        printf("✅ Scène '%s' correctement assignée\n", scene->name ? scene->name : "sans nom");
        return true;
    } else {
        printf("❌ Erreur lors de l'assignation de la scène\n");
        return false;
    }
}

// Mettre à jour le gestionnaire de scènes
void scene_manager_update(SceneManager* manager, float delta_time) {
    if (!manager) return;
    
    // Gérer le changement de scène
    if (manager->scene_change_requested) {
        if (manager->current_scene && manager->current_scene->cleanup) {
            manager->current_scene->cleanup(manager->current_scene);
        }
        
        manager->current_scene = manager->next_scene;
        manager->next_scene = NULL;
        manager->scene_change_requested = false;
        
        if (manager->current_scene && manager->current_scene->init) {
            manager->current_scene->init(manager->current_scene);
        }
    }
    
    // Mettre à jour la scène actuelle
    if (manager->current_scene && manager->current_scene->update) {
        manager->current_scene->update(manager->current_scene, delta_time);
    }
}

// Rendre sur la fenêtre principale
void scene_manager_render_main(SceneManager* manager) {
    if (!manager || !manager->current_scene) return;
    
    GameWindow* main_window = use_main_window();
    if (main_window && manager->current_scene->render) {
        manager->current_scene->render(manager->current_scene, main_window);
    }
}

// Rendre sur la mini fenêtre
void scene_manager_render_mini(SceneManager* manager) {
    if (!manager || !manager->current_scene) return;
    
    GameWindow* mini_window = use_mini_window();
    if (mini_window && manager->current_scene->render) {
        manager->current_scene->render(manager->current_scene, mini_window);
    }
}

// Obtenir la scène courante
Scene* scene_manager_get_current_scene(SceneManager* manager) {
    if (!manager) {
        printf("❌ SceneManager est NULL dans scene_manager_get_current_scene\n");
        return NULL;
    }
    
    Scene* current = manager->current_scene;
    if (current) {
        printf("🔍 Scène courante récupérée: '%s'\n", current->name ? current->name : "sans nom");
    } else {
        printf("⚠️ Aucune scène courante dans le scene manager\n");
    }
    
    return current;
}

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

// Fonctions pour la scène de jeu
static void game_init(Scene* scene) {
    printf("Initialisation de la scène de jeu\n");
    scene->data = NULL;
}

static void game_update(Scene* scene, float delta_time) {
    (void)scene;
    (void)delta_time;
    // Logique de mise à jour du jeu
}

static void game_render(Scene* scene, GameWindow* window) {
    (void)scene;
    if (!window) return;
    
    // Rendre un fond vert pour le jeu
    SDL_Renderer* renderer = window_get_renderer(window);
    SDL_SetRenderDrawColor(renderer, 0, 150, 50, 255);
    SDL_RenderClear(renderer);
}

static void game_cleanup(Scene* scene) {
    printf("Nettoyage de la scène de jeu\n");
    if (scene->data) {
        free(scene->data);
        scene->data = NULL;
    }
}

// Créer la scène de jeu
Scene* create_game_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->name = "Jeu Fanorona";
    scene->init = game_init;
    scene->update = game_update;
    scene->render = game_render;
    scene->cleanup = game_cleanup;
    scene->data = NULL;
    
    return scene;
}