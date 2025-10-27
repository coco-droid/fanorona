#ifndef SCENE_H
#define SCENE_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "../window/window.h"
#include "../event/event.h"
#include "../ui/ui_tree.h"

// Forward declarations pour éviter les dépendances circulaires
typedef struct GameCore GameCore;
typedef struct Scene Scene;
typedef struct SceneManager SceneManager;  // 🔧 ADD: Missing forward declaration

// Options de changement de scène
typedef enum SceneTransitionOption {
    SCENE_TRANSITION_REPLACE,          // Remplacer la scène actuelle
    SCENE_TRANSITION_OPEN_NEW_WINDOW,  // Ouvrir une nouvelle fenêtre
    SCENE_TRANSITION_CLOSE_AND_OPEN,   // Fermer la fenêtre actuelle et en ouvrir une autre
    SCENE_TRANSITION_SWAP_WINDOWS      // Échanger les fenêtres des scènes
} SceneTransitionOption;

// Structure pour une scène
struct Scene {
    char* id;                           // 🔧 FIX: Changed from const char* to char*
    char* name;                         // 🔧 FIX: Changed from const char* to char*
    WindowType target_window;           // Type de fenêtre cible
    EventManager* event_manager;        // Gestionnaire d'événements propre à la scène
    UITree* ui_tree;                    // Arbre UI propre à la scène
    
    void (*init)(struct Scene* scene);
    void (*update)(struct Scene* scene, float delta_time);
    void (*render)(struct Scene* scene, GameWindow* window);
    void (*cleanup)(struct Scene* scene);
    void* data;                         // Données spécifiques à la scène
    
    bool initialized;                   // Flag indiquant si la scène a été initialisée
    bool active;                        // Flag indiquant si la scène est active
};

// Structure pour gérer les transitions de scène
typedef struct SceneTransition {
    Scene* new_scene;
    Scene* old_scene;
    SceneTransitionOption option;       // Option de transition
    WindowType target_window;           // Fenêtre cible pour la transition
} SceneTransition;

// Gestionnaire de scènes avec structure corrigée
typedef struct SceneManager {
    Scene* scenes[16];                  // Tableau des scènes disponibles
    int scene_count;                    // Nombre de scènes
    
    // 🔧 CORRECTION: Ajouter les champs manquants pour l'ancien API
    Scene* current_scene;               // Scène actuellement active
    Scene* next_scene;                  // Prochaine scène (pour les transitions)
    bool scene_change_requested;       // Flag de changement de scène requis
    
    Scene* active_scenes[WINDOW_TYPE_BOTH + 1]; // Scènes actives par fenêtre
    
    SceneTransition* transitions;       // Transitions en cours
    size_t transition_count;
    size_t transition_capacity;
    
    GameCore* core;                     // Référence au core pour les callbacks
} SceneManager;

// Fonctions du gestionnaire de scènes - SIGNATURES CORRIGÉES
SceneManager* scene_manager_create(void);  // 🔧 CORRECTION: Pas de paramètre GameCore
void scene_manager_destroy(SceneManager* manager);

// Gestion des scènes
Scene* scene_create(const char* id, const char* name, WindowType target_window);
void scene_destroy(Scene* scene);
void scene_initialize(Scene* scene);

// Enregistrement et activation des scènes
bool scene_manager_register_scene(SceneManager* manager, Scene* scene);
bool scene_manager_set_scene(SceneManager* manager, Scene* scene);
bool scene_manager_set_scene_for_window(SceneManager* manager, Scene* scene, WindowType window_type);
bool scene_manager_transition_to_scene(SceneManager* manager, const char* scene_id, 
                                     SceneTransitionOption option);

// Récupération des scènes
Scene* scene_manager_get_scene_by_id(SceneManager* manager, const char* id);
Scene* scene_manager_get_active_scene(SceneManager* manager);
Scene* scene_manager_get_active_scene_for_window(SceneManager* manager, WindowType window_type);

// 🔧 CORRECTION: Ajouter la fonction manquante
Scene* scene_manager_get_current_scene(SceneManager* manager);

// Mise à jour et rendu
void scene_manager_update(SceneManager* manager, float delta_time);
void scene_manager_render_main(SceneManager* manager);
void scene_manager_render_mini(SceneManager* manager);

// Gestion des événements
void scene_manager_dispatch_event(SceneManager* manager, WindowEvent* event);

// Scènes existantes (à adapter plus tard)
Scene* create_main_menu_scene(void);
Scene* create_game_scene(void);
Scene* create_home_scene(void);
Scene* create_menu_scene(void);  // 🆕 Ajout de la déclaration pour la nouvelle scène menu
Scene* create_ai_scene(void);    // 🆕 Ajout de la déclaration pour la scène de configuration IA
Scene* create_profile_scene(void); // 🆕 Ajout de la déclaration pour profile_scene
Scene* create_choice_scene(void);  // 🆕 Ajout de la déclaration pour choice_scene
Scene* create_wiki_scene(void);    // 🆕 Ajout de la déclaration pour wiki_scene
Scene* create_pieces_scene(void);  // 🆕 Ajout de la déclaration pour pieces_scene
Scene* create_net_start_scene(void);
Scene* create_lobby_scene(void);
Scene* create_player_list_scene(void);  // 🆕 ADD

// Fonction spéciale pour connecter les événements après création du core
void home_scene_connect_events(Scene* scene, GameCore* core);
void menu_scene_connect_events(Scene* scene, GameCore* core);
void game_scene_connect_events(Scene* scene, GameCore* core);
void ai_scene_connect_events(Scene* scene, GameCore* core);
void profile_scene_connect_events(Scene* scene, GameCore* core);
void choice_scene_connect_events(Scene* scene, GameCore* core);
void wiki_scene_connect_events(Scene* scene, GameCore* core);
void pieces_scene_connect_events(Scene* scene, GameCore* core);
void net_start_scene_connect_events(Scene* scene, GameCore* core);
void lobby_scene_connect_events(Scene* scene, GameCore* core);
void player_list_scene_connect_events(Scene* scene, GameCore* core);  // 🆕 ADD
// 🆕 Associer le core au manager pour les transitions automatiques
void scene_manager_set_core(SceneManager* manager, GameCore* core);
void scene_manager_set_core(SceneManager* manager, GameCore* core);
#endif // SCENE_H
