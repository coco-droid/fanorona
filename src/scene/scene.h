#ifndef SCENE_H
#define SCENE_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "../window/window.h"

// Forward declarations pour éviter les dépendances circulaires
typedef struct GameCore GameCore;
typedef struct Scene Scene;  // 🔧 FIX: Forward declaration de Scene

// Structure pour une scène
struct Scene {
    const char* name;
    void (*init)(struct Scene* scene);
    void (*update)(struct Scene* scene, float delta_time);
    void (*render)(struct Scene* scene, GameWindow* window);
    void (*cleanup)(struct Scene* scene);
    void* data; // Données spécifiques à la scène
};

// Structure pour gérer les transitions de scène
typedef struct SceneTransition {
    Scene* new_scene;  // 🔧 FIX: Maintenant Scene est défini
    Scene* old_scene;
} SceneTransition;

// Gestionnaire de scènes
typedef struct SceneManager {
    Scene* current_scene;
    Scene* next_scene;
    SceneTransition* transitions;      // 🔧 FIX: Ajout du champ manquant
    size_t transition_count;           // 🔧 FIX: Ajout du champ manquant
    size_t transition_capacity;        // 🔧 FIX: Ajout du champ manquant
    bool scene_change_requested;
} SceneManager;

// Fonctions du gestionnaire de scènes
SceneManager* scene_manager_create(void);
void scene_manager_destroy(SceneManager* manager);
bool scene_manager_set_scene(SceneManager* manager, Scene* scene); // 🔧 FIX: Retourne bool pour indiquer le succès
Scene* scene_manager_get_current_scene(SceneManager* manager);
void scene_manager_update(SceneManager* manager, float delta_time);
void scene_manager_render_main(SceneManager* manager);
void scene_manager_render_mini(SceneManager* manager);

// Exemple de scènes
Scene* create_main_menu_scene(void);
Scene* create_game_scene(void);
Scene* create_home_scene(void);

// Fonction spéciale pour connecter les événements après création du core
void home_scene_connect_events(Scene* scene, GameCore* core);

#endif // SCENE_H