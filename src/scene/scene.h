#ifndef SCENE_H
#define SCENE_H

#include "../window/window.h"
#include "../event/event.h"

// Structure pour une scène
typedef struct Scene {
    const char* name;
    void (*init)(struct Scene* scene);
    void (*update)(struct Scene* scene, float delta_time);
    void (*render)(struct Scene* scene, GameWindow* window);
    void (*cleanup)(struct Scene* scene);
    void* data; // Données spécifiques à la scène
} Scene;

// Gestionnaire de scènes
typedef struct SceneManager {
    Scene* current_scene;
    Scene* next_scene;
    bool scene_change_requested;
} SceneManager;

// Fonctions du gestionnaire de scènes
SceneManager* scene_manager_create(void);
void scene_manager_destroy(SceneManager* manager);
void scene_manager_set_scene(SceneManager* manager, Scene* scene);
void scene_manager_update(SceneManager* manager, float delta_time);
void scene_manager_render_main(SceneManager* manager);
void scene_manager_render_mini(SceneManager* manager);

// Exemple de scènes
Scene* create_main_menu_scene(void);
Scene* create_game_scene(void);
Scene* create_home_scene(void);

#endif // SCENE_H