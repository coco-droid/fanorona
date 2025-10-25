#include "../utils/log_console.h"
#include "scene.h"
#include "../ui/native/atomic.h" // <-- ajout
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Créer un gestionnaire de scènes
SceneManager* scene_manager_create(void) {
    SceneManager* manager = (SceneManager*)malloc(sizeof(SceneManager));
    if (!manager) {
        printf("Erreur: Impossible d'allouer la mémoire pour SceneManager\n");
        return NULL;
    }
    
    // Initialiser tous les champs correctement
    manager->scene_count = 0;
    manager->current_scene = NULL;
    manager->next_scene = NULL;
    manager->scene_change_requested = false;
    manager->transition_count = 0;
    manager->transition_capacity = 4;
    manager->core = NULL;
    
    // Initialiser le tableau des scènes
    for (int i = 0; i < 16; i++) {
        manager->scenes[i] = NULL;
    }
    
    // Initialiser les scènes actives par fenêtre
    for (int i = 0; i <= WINDOW_TYPE_BOTH; i++) {
        manager->active_scenes[i] = NULL;
    }
    
    // Allouer de la mémoire pour les transitions
    manager->transitions = (SceneTransition*)malloc(sizeof(SceneTransition) * manager->transition_capacity);
    if (!manager->transitions) {
        free(manager);
        return NULL;
    }
    
    return manager;
}

// Détruire un gestionnaire de scènes
void scene_manager_destroy(SceneManager* manager) {
    if (!manager) return;
    
    printf("🧹 Destruction du SceneManager...\n");
    
    // 🆕 Nettoyer toutes les scènes enregistrées
    for (int i = 0; i < manager->scene_count; i++) {
        if (manager->scenes[i]) {
            printf("🧹 Nettoyage de la scène '%s'...\n", 
                   manager->scenes[i]->name ? manager->scenes[i]->name : "sans nom");
            
            // 🔧 FIX: Remove redundant cleanup call - scene_destroy() already calls cleanup()
            scene_destroy(manager->scenes[i]);
            manager->scenes[i] = NULL;
        }
    }
    
    // 🔧 FIX: Ne plus nettoyer current_scene et next_scene séparément 
    // car elles sont déjà nettoyées dans la boucle ci-dessus
    manager->current_scene = NULL;
    manager->next_scene = NULL;
    
    // Libérer la mémoire des transitions
    free(manager->transitions);
    free(manager);
    
    printf("✅ SceneManager détruit proprement\n");
}

// Définir la scène actuelle
bool scene_manager_set_scene(SceneManager* manager, Scene* scene) {
    if (!manager || !scene) {
        printf("❌ SceneManager ou Scene est NULL dans scene_manager_set_scene\n");
        return false;
    }
    
    printf("🔧 Définition de la scène '%s' comme scène courante...\n", scene->name ? scene->name : "sans nom");
    
    // 🔧 FIX CRITIQUE: Ne pas libérer la mémoire des scènes enregistrées !
    // Les scènes enregistrées sont gérées par le SceneManager, pas par cette fonction
    if (manager->current_scene && manager->current_scene != scene) {
        printf("🧹 Désactivation de la scène précédente...\n");
        if (manager->current_scene->cleanup) {
            manager->current_scene->cleanup(manager->current_scene);
        }
        manager->current_scene->active = false;
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

// Changer vers une nouvelle scène (avec transition)
void scene_manager_transition_to(SceneManager* manager, Scene* new_scene) {
    if (!manager || !new_scene) return;
    
    // Ajouter une nouvelle transition
    if (manager->transition_count >= manager->transition_capacity) {
        manager->transition_capacity *= 2;
        manager->transitions = (SceneTransition*)realloc(manager->transitions, sizeof(SceneTransition) * manager->transition_capacity);
    }
    
    manager->transitions[manager->transition_count].old_scene = manager->current_scene;
    manager->transitions[manager->transition_count].new_scene = new_scene;
    manager->transition_count++;
    
    manager->next_scene = new_scene;
}

// Mettre à jour le gestionnaire de scènes
void scene_manager_update(SceneManager* manager, float delta_time) {
    if (!manager) return;
    
    if (manager->current_scene && manager->current_scene->active) {
        manager->current_scene->update(manager->current_scene, delta_time);
    }
}

// Rendre la scène actuelle
void scene_manager_render(SceneManager* manager) {
    if (!manager) return;
    
    if (manager->current_scene) {
        manager->current_scene->render(manager->current_scene, use_main_window());
    }
}

// Fonctions de rendu séparées pour les différentes fenêtres
void scene_manager_render_main(SceneManager* manager) {
    if (!manager) return;

    GameWindow* main_window = use_main_window();
    if (!main_window) return;

    AtomicContext ctx_main = { main_window->renderer, main_window->width, main_window->height, false };
    atomic_set_context(&ctx_main);

    Scene* scene = scene_manager_get_active_scene_for_window(manager, WINDOW_TYPE_MAIN);

    if (scene && scene->active && scene->render) {
        scene->render(scene, main_window);
    }
}

void scene_manager_render_mini(SceneManager* manager) {
    if (!manager) return;

    GameWindow* mini_window = use_mini_window();
    if (!mini_window) return;

    AtomicContext ctx_mini = { mini_window->renderer, mini_window->width, mini_window->height, false };
    atomic_set_context(&ctx_mini);

    Scene* scene = scene_manager_get_active_scene_for_window(manager, WINDOW_TYPE_MINI);

    if (scene && scene->active && scene->render) {
        scene->render(scene, mini_window);
    }
}

// Obtenir la scène courante
Scene* scene_manager_get_current_scene(SceneManager* manager) {
    return manager ? manager->current_scene : NULL;
}

// Fonctions améliorées pour l'association scène-fenêtre
Scene* scene_manager_get_active_scene_for_window(SceneManager* manager, WindowType window_type) {
    if (!manager || window_type > WINDOW_TYPE_BOTH) {
        return NULL;
    }
    
    if (manager->active_scenes[window_type]) {
        return manager->active_scenes[window_type];
    }
    
    return manager->current_scene;
}

bool scene_manager_set_scene_for_window(SceneManager* manager, Scene* scene, WindowType window_type) {
    if (!manager || !scene || window_type > WINDOW_TYPE_BOTH) {
        printf("❌ Paramètres invalides pour set_scene_for_window\n");
        return false;
    }
    
    if (!scene->initialized && scene->init) {
        printf("🔧 Initialisation de la scène '%s' pour la fenêtre %d...\n", 
               scene->name ? scene->name : "sans nom", window_type);
        scene->init(scene);
        scene->initialized = true;
    }
    
    manager->active_scenes[window_type] = scene;
    scene->active = true;
    
    printf("✅ Scène '%s' assignée à la fenêtre type %d\n", 
           scene->name ? scene->name : "sans nom", window_type);
    return true;
}

// Nouvelles fonctions pour l'API étendue
bool scene_manager_register_scene(SceneManager* manager, Scene* scene) {
    if (!manager || !scene) return false;
    
    if (manager->scene_count >= 16) {
        printf("❌ Impossible d'ajouter plus de scènes (limite: 16)\n");
        return false;
    }
    
    manager->scenes[manager->scene_count] = scene;
    manager->scene_count++;
    
    printf("✅ Scène '%s' enregistrée (total: %d)\n", scene->name, manager->scene_count);
    return true;
}

Scene* scene_manager_get_scene_by_id(SceneManager* manager, const char* id) {
    if (!manager || !id) return NULL;
    
    for (int i = 0; i < manager->scene_count; i++) {
        if (manager->scenes[i] && manager->scenes[i]->id && 
            strcmp(manager->scenes[i]->id, id) == 0) {
            return manager->scenes[i];
        }
    }
    
    return NULL;
}

Scene* scene_manager_get_active_scene(SceneManager* manager) {
    return scene_manager_get_current_scene(manager);
}

bool scene_manager_transition_to_scene(SceneManager* manager, const char* scene_id, 
                                     SceneTransitionOption option) {
    if (!manager || !scene_id) {
        printf("❌ Paramètres invalides pour la transition\n");
        return false;
    }
    
    Scene* target_scene = scene_manager_get_scene_by_id(manager, scene_id);
    if (!target_scene) {
        printf("❌ Scène '%s' introuvable\n", scene_id);
        return false;
    }
    
    printf("🔄 Transition vers la scène '%s' (option: %d)\n", scene_id, option);
    
    WindowType source_window_type = window_get_active_window();
    WindowType target_window = target_scene->target_window;
    
    Scene* old_scene = manager->current_scene;
    
    switch (option) {
        case SCENE_TRANSITION_REPLACE:
            if (old_scene) {
                old_scene->active = false;
            }
            
            manager->current_scene = target_scene;
            
            scene_manager_set_scene_for_window(manager, target_scene, source_window_type);
            if (target_window != source_window_type) {
                scene_manager_set_scene_for_window(manager, target_scene, target_window);
            }
            
            window_set_active_window(target_window);
            break;
            
        case SCENE_TRANSITION_OPEN_NEW_WINDOW:
            scene_manager_set_scene_for_window(manager, target_scene, target_window);
            window_set_active_window(WINDOW_TYPE_BOTH);
            break;
            
        case SCENE_TRANSITION_CLOSE_AND_OPEN:
            manager->current_scene = target_scene;
            scene_manager_set_scene_for_window(manager, target_scene, target_window);
            
            if (!target_scene->initialized && target_scene->init) {
                target_scene->init(target_scene);
                target_scene->initialized = true;
            }
            
            if (old_scene) {
                old_scene->active = false;
            }
            
            target_scene->active = true;
            
            extern void window_transition_safely(WindowType from_type, WindowType to_type);
            window_transition_safely(source_window_type, target_window);
            break;
            
        case SCENE_TRANSITION_SWAP_WINDOWS:
            if (manager->current_scene) {
                WindowType old_window = manager->current_scene->target_window;
                scene_manager_set_scene_for_window(manager, target_scene, old_window);
            }
            manager->current_scene = target_scene;
            break;
    }
    
    if (!target_scene->initialized && target_scene->init) {
        target_scene->init(target_scene);
        target_scene->initialized = true;
    }
    
    target_scene->active = true;
    
    if (!target_scene->event_manager) {
        target_scene->event_manager = event_manager_create();
    }
    
    // 🔧 FIX: Connecter les événements IMMÉDIATEMENT après activation
    if (manager->core) {
        printf("🔗 Connexion des événements pour la nouvelle scène '%s'...\n", scene_id);
        
        // 🆕 FIX: Utiliser un tableau de correspondance ID → fonction
        if (strcmp(scene_id, "home") == 0) {
            home_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "menu") == 0) {
            menu_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "choice") == 0) {
            choice_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "ai") == 0) {
            ai_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "profile") == 0) {
            profile_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "game") == 0) {
            game_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "wiki") == 0) {
            wiki_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "pieces") == 0) {  // 🆕 Ajout de pieces_scene
            pieces_scene_connect_events(target_scene, manager->core);
        } else {
            printf("⚠️ Pas de fonction de connexion pour '%s'\n", scene_id);
        }
        
        printf("✅ Événements connectés pour '%s'\n", scene_id);
    } else {
        printf("❌ Core NULL - impossible de connecter les événements\n");
    }
    
    printf("✅ Transition réussie vers '%s' !\n", scene_id);
    return true;
}

void scene_manager_dispatch_event(SceneManager* manager, WindowEvent* event) {
    if (!manager || !event) return;
    
    Scene* current = scene_manager_get_current_scene(manager);
    if (current && current->event_manager) {
        event_manager_handle_event(current->event_manager, &event->sdl_event);
    }
}

Scene* scene_create(const char* id, const char* name, WindowType target_window) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) return NULL;
    
    scene->id = id ? strdup(id) : NULL;
    scene->name = name ? strdup(name) : NULL;
    scene->target_window = target_window;
    scene->event_manager = NULL;
    scene->ui_tree = NULL;
    scene->init = NULL;
    scene->update = NULL;
    scene->render = NULL;
    scene->cleanup = NULL;
    scene->data = NULL;
    scene->initialized = false;
    scene->active = false;
    
    return scene;
}

void scene_initialize(Scene* scene) {
    if (!scene) return;
    
    if (scene->init) {
        scene->init(scene);
        scene->initialized = true;
    }
}

// 🆕 Nouvelle fonction pour associer le core au manager
void scene_manager_set_core(SceneManager* manager, GameCore* core) {
    if (manager) {
        manager->core = core;
        printf("✅ Core associé au SceneManager\n");
    }
}