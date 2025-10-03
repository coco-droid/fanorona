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
            
            if (manager->scenes[i]->cleanup) {
                manager->scenes[i]->cleanup(manager->scenes[i]);
            }
            
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
        // 🔧 NE PAS FAIRE free() ici - les scènes sont dans le registre !
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

// Fonctions de rendu séparées pour les différentes fenêtres
void scene_manager_render_main(SceneManager* manager) {
    if (!manager) return;

    GameWindow* main_window = use_main_window();
    if (!main_window) return;

    // Définir le contexte atomic pour la fenêtre principale
    AtomicContext ctx_main = { main_window->renderer, main_window->width, main_window->height, false };
    atomic_set_context(&ctx_main);

    // Utiliser la scène assignée à la fenêtre principale ou la scène courante
    Scene* scene = scene_manager_get_active_scene_for_window(manager, WINDOW_TYPE_MAIN);

    if (scene && scene->active && scene->render) {
        scene->render(scene, main_window);
    }
}

void scene_manager_render_mini(SceneManager* manager) {
    if (!manager) return;

    GameWindow* mini_window = use_mini_window();
    if (!mini_window) return;

    // Définir le contexte atomic pour la mini fenêtre
    AtomicContext ctx_mini = { mini_window->renderer, mini_window->width, mini_window->height, false };
    atomic_set_context(&ctx_mini);

    // Utiliser la scène assignée à la mini fenêtre ou la scène courante
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
    
    // Si une scène est explicitement assignée pour ce type de fenêtre, la renvoyer
    if (manager->active_scenes[window_type]) {
        return manager->active_scenes[window_type];
    }
    
    // Sinon, renvoyer la scène courante comme fallback
    return manager->current_scene;
}

bool scene_manager_set_scene_for_window(SceneManager* manager, Scene* scene, WindowType window_type) {
    if (!manager || !scene || window_type > WINDOW_TYPE_BOTH) {
        printf("❌ Paramètres invalides pour set_scene_for_window\n");
        return false;
    }
    
    // Si la scène n'est pas initialisée, l'initialiser
    if (!scene->initialized && scene->init) {
        printf("🔧 Initialisation de la scène '%s' pour la fenêtre %d...\n", 
               scene->name ? scene->name : "sans nom", window_type);
        scene->init(scene);
        scene->initialized = true;
    }
    
    // Assigner la scène à la fenêtre spécifique
    manager->active_scenes[window_type] = scene;
    
    // Activer la scène
    scene->active = true;
    
    printf("✅ Scène '%s' assignée à la fenêtre type %d\n", 
           scene->name ? scene->name : "sans nom", window_type);
    return true;
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
    
    printf("🔄 FALLBACK: Transition vers la scène '%s' demandée depuis l'élément UI '%s'\n", 
           target_scene_id, element->id ? element->id : "unknown");
    
    // 🆕 AVERTISSEMENT CRITIQUE POUR INDIQUER QUE LE FALLBACK EST DÉCONSEILLÉ
    printf("⚠️ ATTENTION CRITIQUE: Cette méthode de transition est dépréciée et peut causer des comportements incorrects\n");
    printf("📋 Solution: Utilisez ui_link_connect_to_manager() pour configurer correctement les transitions\n");
    
    // 🆕 NE PAS FAIRE DE TRANSITION AUTOMATIQUE AU GAME_SCENE!
    // Ne rien faire d'autre, simplement afficher l'avertissement.
    printf("❌ Transition annulée pour éviter des comportements incorrects\n");
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
    
    // 🆕 Déterminer quelle fenêtre source est actuellement active
    WindowType source_window_type = window_get_active_window();
    // Déterminer la fenêtre cible selon la préférence de la scène
    WindowType target_window = target_scene->target_window;
    
    // 🆕 LOG DE DIAGNOSTIC DÉTAILLÉ
    char transit_msg[256];
    snprintf(transit_msg, sizeof(transit_msg),
             "[scene_manager.c] Transition: from window %d to scene '%s' (target window %d) with option %d",
             source_window_type, scene_id, target_window, option);
    log_console_write("SceneTransition", "Start", "scene_manager.c", transit_msg);
    
    // Stocker l'ancienne scène pour référence
    Scene* old_scene = manager->current_scene;
    
    // 🆕 Gestion des différentes options de transition
    switch (option) {
        case SCENE_TRANSITION_REPLACE:
            // Désactiver l'ancienne scène pour la fenêtre source
            if (old_scene) {
                old_scene->active = false;
                log_console_write("SceneTransition", "DeactivateOld", "scene_manager.c",
                                 "[scene_manager.c] Deactivated old scene");
            }
            
            // Définir la nouvelle scène comme courante
            manager->current_scene = target_scene;
            
            // 🔧 FIX CRITIQUE: Assigner la nouvelle scène à la fenêtre SOURCE ET CIBLE
            scene_manager_set_scene_for_window(manager, target_scene, source_window_type);
            if (target_window != source_window_type) {
                scene_manager_set_scene_for_window(manager, target_scene, target_window);
            }
            
            // 🆕 Activer la fenêtre cible
            window_set_active_window(target_window);
            break;
            
        case SCENE_TRANSITION_OPEN_NEW_WINDOW:
            // Garder l'ancienne scène active et ajouter la nouvelle dans une autre fenêtre
            scene_manager_set_scene_for_window(manager, target_scene, target_window);
            window_set_active_window(WINDOW_TYPE_BOTH); // Activer les deux fenêtres
            break;
            
        case SCENE_TRANSITION_CLOSE_AND_OPEN:
            // 🔧 FIX CRITIQUE: Utiliser la nouvelle fonction de transition sécurisée
            printf("🔄 TRANSITION SÉCURISÉE: Initialisation de la scène cible avant fermeture\n");
            
            // 1. Définir la nouvelle scène comme courante
            manager->current_scene = target_scene;
            
            // 2. Assigner la scène à sa fenêtre cible
            printf("🔍 VÉRIFICATION: Assignation de '%s' à la fenêtre %d\n", 
                   target_scene->name, target_window);
            scene_manager_set_scene_for_window(manager, target_scene, target_window);
            
            // 3. S'assurer que la scène est initialisée
            if (!target_scene->initialized && target_scene->init) {
                printf("🔍 INITIALISATION de la scène '%s'\n", target_scene->name);
                target_scene->init(target_scene);
                target_scene->initialized = true;
            }
            
            // 4. Désactiver l'ancienne scène
            if (old_scene) {
                printf("🔍 DÉSACTIVATION de l'ancienne scène '%s'\n", old_scene->name);
                old_scene->active = false;
            }
            
            // 5. Activer la nouvelle scène
            target_scene->active = true;
            printf("🔍 ACTIVATION de la nouvelle scène '%s'\n", target_scene->name);
            
            // 6. Effectuer la transition sécurisée des fenêtres
            extern void window_transition_safely(WindowType from_type, WindowType to_type);
            window_transition_safely(source_window_type, target_window);
            break;
            
        case SCENE_TRANSITION_SWAP_WINDOWS:
            // Échanger les fenêtres des scènes
            if (manager->current_scene) {
                WindowType old_window = manager->current_scene->target_window;
                scene_manager_set_scene_for_window(manager, target_scene, old_window);
            }
            manager->current_scene = target_scene;
            break;
    }
    
    // S'assurer que la scène cible est initialisée et active
    if (!target_scene->initialized && target_scene->init) {
        log_console_write("SceneTransition", "InitTarget", "scene_manager.c",
                         "[scene_manager.c] Initializing target scene");
        target_scene->init(target_scene);
        target_scene->initialized = true;
    }
    
    // Activer la scène
    target_scene->active = true;
    
    // 🆕 Vérifier la configuration des EventManagers après transition
    if (!target_scene->event_manager) {
        log_console_write("SceneTransition", "CreateEventManager", "scene_manager.c",
                         "[scene_manager.c] Creating EventManager for target scene");
        target_scene->event_manager = event_manager_create();
    }
    
    // 🆕 LOG DE CONFIRMATION DÉTAILLÉ
    char result_msg[256];
    snprintf(result_msg, sizeof(result_msg),
             "[scene_manager.c] Transition complete: scene '%s' now active in window %d",
             scene_id, target_window);
    log_console_write("SceneTransition", "Complete", "scene_manager.c", result_msg);
    
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
    
    // 🔧 FIX: No need to cast since id and name are now char* (non-const)
    if (scene->id) free(scene->id);
    if (scene->name) free(scene->name);
    
    free(scene);
}

void scene_initialize(Scene* scene) {
    if (!scene) return;
    
    if (scene->init) {
        scene->init(scene);
        scene->initialized = true;
    }
}