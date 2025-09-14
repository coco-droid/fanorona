#include "../utils/log_console.h"
#include "scene.h"
#include <stdlib.h>
#include <stdio.h>

// Créer un gestionnaire de scènes
SceneManager* scene_manager_create(void) {
    SceneManager* manager = (SceneManager*)malloc(sizeof(SceneManager));
    if (!manager) {
        printf("Erreur: Impossible d'allouer la mémoire pour SceneManager\n");
        return NULL;
    }
    
    manager->current_scene = NULL;
    manager->next_scene = NULL;
    manager->transition_count = 0;
    manager->transition_capacity = 4; // Capacité initiale
    
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
    
    // Mettre à jour la scène actuelle
    if (manager->current_scene) {
        manager->current_scene->update(manager->current_scene, delta_time);
    }
    
    // Vérifier les transitions
    if (manager->transition_count > 0) {
        // Obtenir la dernière transition
        SceneTransition* transition = &manager->transitions[manager->transition_count - 1];
        
        // Nettoyer l'ancienne scène
        if (transition->old_scene) {
            transition->old_scene->cleanup(transition->old_scene);
        }
        
        // Passer à la nouvelle scène
        manager->current_scene = transition->new_scene;
        manager->current_scene->init(manager->current_scene);
        
        // Réduire le nombre de transitions en attente
        manager->transition_count--;
    }
}

// Rendre la scène actuelle
void scene_manager_render(SceneManager* manager) {
    if (!manager) return;
    
    // Rendre la scène actuelle
    if (manager->current_scene) {
        manager->current_scene->render(manager->current_scene, use_main_window());
    }
}

// 🆕 Obtenir la scène courante
Scene* scene_manager_get_current_scene(SceneManager* manager) {
    return manager ? manager->current_scene : NULL;
}

// 🆕 Fonctions de rendu séparées pour les différentes fenêtres
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