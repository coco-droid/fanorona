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
        // 🆕 FIX: Vérifier si la scène est encore active dans une fenêtre avant de la nettoyer
        bool is_still_active_in_window = false;
        if (manager->active_scenes[WINDOW_TYPE_MAIN] == manager->current_scene) is_still_active_in_window = true;
        if (manager->active_scenes[WINDOW_TYPE_MINI] == manager->current_scene) is_still_active_in_window = true;
        
        if (!is_still_active_in_window) {
            printf("🧹 Désactivation de la scène précédente...\n");
            if (manager->current_scene->cleanup) {
                manager->current_scene->cleanup(manager->current_scene);
            }
            manager->current_scene->active = false;
        } else {
            printf("🔒 Scène précédente '%s' maintenue active (utilisée dans une fenêtre)\n", 
                   manager->current_scene->name);
        }
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
    
    // 🆕 UPDATE MULTI-FENÊTRES : Mettre à jour toutes les scènes actives
    
    // 1. Mettre à jour la scène MAIN si elle est active
    Scene* main_scene = manager->active_scenes[WINDOW_TYPE_MAIN];
    if (main_scene && main_scene->active) {
        main_scene->update(main_scene, delta_time);
    }
    
    // 2. Mettre à jour la scène MINI si elle est active
    Scene* mini_scene = manager->active_scenes[WINDOW_TYPE_MINI];
    if (mini_scene && mini_scene->active) {
        // Éviter la double mise à jour si c'est la même scène (cas rare mais possible)
        if (mini_scene != main_scene) {
            mini_scene->update(mini_scene, delta_time);
        }
    }
    
    // 3. Fallback pour current_scene si elle n'est pas dans les slots actifs (transition/init)
    if (manager->current_scene && manager->current_scene->active && 
        manager->current_scene != main_scene && manager->current_scene != mini_scene) {
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
        // 🔧 FIX: Vérification de sécurité supplémentaire
        if (scene->target_window == WINDOW_TYPE_MAIN || scene->target_window == WINDOW_TYPE_BOTH) {
            scene->render(scene, main_window);
        }
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
        // 🔧 FIX: Vérification de sécurité supplémentaire
        if (scene->target_window == WINDOW_TYPE_MINI || scene->target_window == WINDOW_TYPE_BOTH) {
            scene->render(scene, mini_window);
        }
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
    
    // 🔧 FIX: Fallback strict - ne retourner current_scene que si elle correspond au type de fenêtre demandé
    // 🆕 AJOUT: Supporter aussi WINDOW_TYPE_BOTH si la scène est conçue pour les deux
    if (manager->current_scene && 
        (manager->current_scene->target_window == window_type || 
         manager->current_scene->target_window == WINDOW_TYPE_BOTH)) {
        return manager->current_scene;
    }
    
    return NULL;
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
        case SCENE_TRANSITION_NONE: // 🆕 Traiter NONE comme REPLACE
        case SCENE_TRANSITION_FADE: // 🆕 Traiter FADE comme REPLACE (pour l'instant)
            manager->current_scene = target_scene;
            
            // 🔧 FIX: Assigner la scène UNIQUEMENT à sa fenêtre cible
            // L'ancien code écrasait active_scenes[source_window] même si la cible était différente
            scene_manager_set_scene_for_window(manager, target_scene, target_window);
            
            // 🔧 FIX: Gestion intelligente des fenêtres pour le multi-fenêtrage
            if (target_window == WINDOW_TYPE_MINI) {
                // Vérifier si on doit garder la fenêtre principale active (si on vient de MAIN/BOTH ou si une scène MAIN est déjà active)
                bool keep_main_active = (source_window_type == WINDOW_TYPE_MAIN || 
                                       source_window_type == WINDOW_TYPE_BOTH || 
                                       manager->active_scenes[WINDOW_TYPE_MAIN] != NULL);
                
                if (keep_main_active) {
                    // Si on ouvre une Mini depuis Main ou Both, on force le mode BOTH pour garder le fond (Jeu) visible
                    printf("🔀 Transition vers Mini : Mode BOTH maintenu/activé pour garder le fond\n");
                    window_set_active_window(WINDOW_TYPE_BOTH);
                    
                    // 🔧 CRITIQUE: S'assurer que la scène MAIN reste active
                    if (manager->active_scenes[WINDOW_TYPE_MAIN]) {
                        manager->active_scenes[WINDOW_TYPE_MAIN]->active = true;
                        printf("✅ Scène MAIN '%s' maintenue active\n", manager->active_scenes[WINDOW_TYPE_MAIN]->name);
                    }
                    
                    // Si on remplace une scène Mini existante (ex: Settings -> Profile), on la désactive
                    if (old_scene && old_scene->target_window == WINDOW_TYPE_MINI) {
                        old_scene->active = false;
                    }
                    // NOTE: On ne désactive PAS old_scene si c'est la scène Main (Jeu), elle reste visible
                } else {
                    // Cas Mini -> Mini sans Main actif
                    window_set_active_window(WINDOW_TYPE_MINI);
                    if (old_scene) old_scene->active = false;
                }
            } else {
                // Cas standard (Main->Main, Mini->Main, etc.) : on bascule vers la fenêtre cible
                window_set_active_window(target_window);
                
                // On désactive l'ancienne scène car on change de contexte complet
                if (old_scene) {
                    old_scene->active = false;
                }
            }
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
        } else if (strcmp(scene_id, "net_start") == 0) {  // 🆕 Ajout de net_start_scene
            net_start_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "lobby") == 0) {  // 🆕 Ajout de lobby_scene
            lobby_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "player_list") == 0) {  // 🆕 Ajout de player_list_scene
            player_list_scene_connect_events(target_scene, manager->core);
        } else if (strcmp(scene_id, "setting") == 0) {      // 🆕 Ajout de setting_scene
            setting_scene_connect_events(target_scene, manager->core);
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
    
    // 🔧 FIX: Router l'événement vers la scène correspondant à la fenêtre source
    Scene* target_scene = NULL;
    
    if (event->is_valid) {
        // Essayer de trouver la scène associée à la fenêtre de l'événement
        target_scene = scene_manager_get_active_scene_for_window(manager, event->window_type);
    }
    
    // Fallback sur la scène courante si non trouvé ou événement global
    if (!target_scene) {
        target_scene = scene_manager_get_current_scene(manager);
    }
    
    if (target_scene && target_scene->event_manager) {
        event_manager_handle_event(target_scene->event_manager, &event->sdl_event);
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