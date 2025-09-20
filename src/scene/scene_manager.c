#include "../utils/log_console.h"
#include "scene.h"
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
    
    // Nettoyer les scènes en cours et suivantes
    if (manager->current_scene) {
        manager->current_scene->cleanup(manager->current_scene);
    }
    if (manager->next_scene) {
        manager->next_scene->cleanup(manager->next_scene);
    }
    
    // Libérer la mémoire des transitions
    free(manager->transitions);
    free(manager);
}

// Définir la scène actuelle
bool scene_manager_set_scene(SceneManager* manager, Scene* scene) {
    if (!manager || !scene) {
        printf("❌ SceneManager ou Scene est NULL dans scene_manager_set_scene\n");
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

// Changer vers une nouvelle scène (avec transition)
void scene_manager_transition_to(SceneManager* manager, Scene* new_scene) {
    if (!manager || !new_scene) return;
    
    // Ajouter une nouvelle transition
    if (manager->transition_count >= manager->transition_capacity) {
        // Doubler la capacité
        manager->transition_capacity *= 2;
        manager->transitions = (SceneTransition*)realloc(manager->transitions, sizeof(SceneTransition) * manager->transition_capacity);
    }
    
    manager->transitions[manager->transition_count].old_scene = manager->current_scene;
    manager->transitions[manager->transition_count].new_scene = new_scene;
    manager->transition_count++;
    
    // Définir la nouvelle scène comme scène suivante
    manager->next_scene = new_scene;
}

// Mettre à jour le gestionnaire de scènes
void scene_manager_update(SceneManager* manager, float delta_time) {
    if (!manager) return;
    
    // Mettre à jour SEULEMENT la scène actuelle
    if (manager->current_scene && manager->current_scene->active) {
        manager->current_scene->update(manager->current_scene, delta_time);
    }
    
    // 🔧 FIX: SUPPRIMER la gestion automatique des transitions avec cleanup
    // Les transitions sont maintenant immédiates et sans destruction
}

// Rendre la scène actuelle
void scene_manager_render(SceneManager* manager) {
    if (!manager) return;
    
    // Rendre la scène actuelle
    if (manager->current_scene) {
        manager->current_scene->render(manager->current_scene, use_main_window());
    }
}

// Obtenir la scène courante
Scene* scene_manager_get_current_scene(SceneManager* manager) {
    return manager ? manager->current_scene : NULL;
}

// Fonctions de rendu séparées pour les différentes fenêtres
void scene_manager_render_main(SceneManager* manager) {
    if (!manager || !manager->current_scene) return;
    
    GameWindow* main_window = use_main_window();
    if (main_window) {
        manager->current_scene->render(manager->current_scene, main_window);
    }
}

void scene_manager_render_mini(SceneManager* manager) {
    if (!manager || !manager->current_scene) return;
    
    GameWindow* mini_window = use_mini_window();
    if (mini_window) {
        manager->current_scene->render(manager->current_scene, mini_window);
    }
}

// Fonction manquante pour ui_link.c - VERSION AMÉLIORÉE
void scene_manager_transition_to_scene_from_element(UINode* element) {
    if (!element) {
        printf("❌ UINode est NULL dans scene_manager_transition_to_scene_from_element\n");
        return;
    }
    
    // Utiliser void* et accès générique aux données
    void* component_data = element->component_data;
    if (!component_data) {
        printf("❌ Component data est NULL\n");
        return;
    }
    
    // Utiliser une fonction helper depuis ui_link.c
    extern const char* ui_link_get_target_scene_id_from_data(void* data);
    
    const char* target_scene_id = ui_link_get_target_scene_id_from_data(component_data);
    if (!target_scene_id) {
        printf("❌ Target scene ID manquant dans les données de lien\n");
        return;
    }
    
    printf("🔄 Transition vers la scène '%s' demandée depuis l'élément UI '%s'\n", 
           target_scene_id, element->id ? element->id : "unknown");
    
    // 🔧 FIX: Cette fonction sera appelée SEULEMENT en fallback
    printf("⚠️ FALLBACK: Transition simulée - utilisez ui_link_connect_to_manager() pour de vraies transitions\n");
    printf("🔧 Pour activer les transitions : connectez le lien au SceneManager via ui_link_connect_to_manager()\n");
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

Scene* scene_manager_get_active_scene_for_window(SceneManager* manager, WindowType window_type) {
    if (!manager || window_type > WINDOW_TYPE_BOTH) return NULL;
    
    return manager->active_scenes[window_type];
}

bool scene_manager_set_scene_for_window(SceneManager* manager, Scene* scene, WindowType window_type) {
    if (!manager || !scene || window_type > WINDOW_TYPE_BOTH) return false;
    
    manager->active_scenes[window_type] = scene;
    printf("✅ Scène '%s' assignée à la fenêtre type %d\n", scene->name, window_type);
    return true;
}

bool scene_manager_transition_to_scene(SceneManager* manager, const char* scene_id, 
                                     SceneTransitionOption option) {
    if (!manager || !scene_id) {
        printf("❌ Paramètres invalides pour la transition\n");
        return false;
    }
    
    // 🔧 FIX: Éviter le warning unused parameter
    (void)option; // Marquer comme intentionnellement non utilisé
    
    Scene* target_scene = scene_manager_get_scene_by_id(manager, scene_id);
    if (!target_scene) {
        printf("❌ Scène '%s' introuvable\n", scene_id);
        return false;
    }
    
    printf("🔄 SWAP vers la scène '%s' (sans cleanup)\n", scene_id);
    
    // 🔧 FIX: SIMPLE SWAP SANS CLEANUP
    Scene* old_scene = manager->current_scene;
    
    // Initialiser la nouvelle scène seulement si pas déjà initialisée
    if (!target_scene->initialized) {
        printf("🔧 Initialisation de la scène '%s'...\n", scene_id);
        if (target_scene->init) {
            target_scene->init(target_scene);
            target_scene->initialized = true;
        }
    } else {
        printf("ℹ️ Scène '%s' déjà initialisée, réutilisation\n", scene_id);
    }
    
    // Swap simple
    manager->current_scene = target_scene;
    target_scene->active = true;
    
    // Désactiver l'ancienne scène SANS cleanup
    if (old_scene) {
        old_scene->active = false;
        printf("📴 Ancienne scène '%s' désactivée (conservée en mémoire)\n", 
               old_scene->name ? old_scene->name : "unknown");
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

void scene_destroy(Scene* scene) {
    if (!scene) return;
    
    if (scene->cleanup) {
        scene->cleanup(scene);
    }
    
    if (scene->id) free((void*)scene->id);
    if (scene->name) free((void*)scene->name);
    
    free(scene);
}

void scene_initialize(Scene* scene) {
    if (!scene) return;
    
    if (scene->init) {
        scene->init(scene);
        scene->initialized = true;
    }
}