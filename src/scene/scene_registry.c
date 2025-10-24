#include "scene_registry.h"
#include "scene.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <string.h>

// Type pour les fonctions de création de scènes
typedef Scene* (*SceneFactory)(void);

// Liste des usines de scènes disponibles
static SceneFactory factories[] = {
    create_home_scene,
    create_choice_scene,  // 🆕 Ajout de choice_scene
    create_menu_scene,
    create_game_scene,
    create_ai_scene,
    create_profile_scene, // 🆕 Ajout de profile_scene
    create_wiki_scene,   // 🆕 Ajout de wiki_scene
    NULL  // Marqueur de fin
};

bool scene_registry_register_all(SceneManager* manager) {
    if (!manager) {
        printf("❌ scene_registry: SceneManager NULL\n");
        return false;
    }

    int registered = 0;
    Scene* home_scene = NULL;

    printf("🎭 Registre de scènes : Début de l'enregistrement automatique...\n");

    // Parcourir toutes les usines de scènes
    for (SceneFactory* factory = factories; *factory != NULL; ++factory) {
        Scene* scene = (*factory)();
        if (!scene) {
            printf("⚠️ scene_registry: création d'une scène a échoué, ignorée\n");
            continue;
        }

        // Vérifier que la scène a un ID valide
        if (!scene->id) {
            printf("❌ scene_registry: scène sans ID trouvée ('%s'), destruction\n", 
                   scene->name ? scene->name : "sans nom");
            scene_destroy(scene);
            continue;
        }

        // Enregistrer la scène dans le manager
        if (!scene_manager_register_scene(manager, scene)) {
            printf("❌ scene_registry: échec enregistrement scène '%s' (ID: %s)\n", 
                   scene->name ? scene->name : "sans nom", scene->id);
            scene_destroy(scene);
            continue;
        }

        registered++;
        printf("✅ scene_registry: scène '%s' (ID: %s) enregistrée\n", 
               scene->name ? scene->name : "sans nom", scene->id);

        // Garder une référence à la scène home pour la définir comme courante
        if (strcmp(scene->id, "home") == 0) {
            home_scene = scene;
        }
    }

    // Définir la scène home comme scène courante si elle existe
    if (home_scene) {
        printf("🏠 scene_registry: définition de 'home' comme scène courante...\n");
        
        if (!scene_manager_set_scene(manager, home_scene)) {
            printf("❌ scene_registry: impossible de définir 'home' comme scène courante\n");
            return false;
        }

        // Initialiser la scène home immédiatement
        if (home_scene->init && !home_scene->initialized) {
            printf("🔧 scene_registry: initialisation de la scène home...\n");
            home_scene->init(home_scene);
            home_scene->initialized = true;
            
            if (home_scene->data) {
                printf("✅ scene_registry: scène home initialisée avec succès\n");
            } else {
                printf("⚠️ scene_registry: scène home initialisée mais données manquantes\n");
            }
        }
    } else {
        printf("⚠️ scene_registry: aucune scène 'home' trouvée, pas de scène courante définie\n");
    }

    printf("🎭 Registre de scènes : %d scènes enregistrées au total\n", registered);
    
    // Log dans la console de debug si disponible
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), 
             "[scene_registry.c] %d scenes registered, home scene %s", 
             registered, home_scene ? "set as current" : "not found");
    log_console_write("SceneRegistry", "Complete", "scene_registry.c", log_msg);
    
    return registered > 0;
}

bool scene_registry_connect_all_events(SceneManager* manager, GameCore* core) {
    if (!manager || !core) {
        printf("❌ scene_registry: manager ou core NULL pour la connexion d'événements\n");
        return false;
    }

    // 🔧 FIX: Connect events ONLY for the currently active scene, not all scenes
    Scene* active_scene = scene_manager_get_current_scene(manager);
    if (!active_scene) {
        printf("❌ scene_registry: Aucune scène active à connecter\n");
        return false;
    }

    printf("🔗 scene_registry: connexion des événements UNIQUEMENT pour '%s' (ID: %s)\n", 
           active_scene->name ? active_scene->name : "sans nom", 
           active_scene->id ? active_scene->id : "no-id");
    
    // Initialiser la scène si ce n'est pas déjà fait
    if (!active_scene->initialized && active_scene->init) {
        printf("🔧 scene_registry: initialisation de la scène '%s'...\n", active_scene->id);
        active_scene->init(active_scene);
        active_scene->initialized = true;
    }
    
    // Connecter les événements selon le type de scène
    if (active_scene->id) {
        if (strcmp(active_scene->id, "home") == 0) {
            home_scene_connect_events(active_scene, core);
        } else if (strcmp(active_scene->id, "menu") == 0) {
            menu_scene_connect_events(active_scene, core);
        } else if (strcmp(active_scene->id, "game") == 0) {
            game_scene_connect_events(active_scene, core);
        } else if (strcmp(active_scene->id, "ai") == 0) {
            ai_scene_connect_events(active_scene, core);
        } else if (strcmp(active_scene->id, "profile") == 0) {
            profile_scene_connect_events(active_scene, core);
        } else if (strcmp(active_scene->id, "choice") == 0) {
            choice_scene_connect_events(active_scene, core);
        } else if (strcmp(active_scene->id, "wiki") == 0) {
            wiki_scene_connect_events(active_scene, core);
        } else {
            printf("⚠️ scene_registry: type de scène '%s' inconnu, pas de connexion d'événements spécifique\n", 
                   active_scene->id);
        }
    }
    
    printf("✅ scene_registry: Événements connectés UNIQUEMENT pour la scène '%s'\n", active_scene->id);
    
    // Log dans la console de debug
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), 
             "[scene_registry.c] Events connected ONLY for active scene '%s'", 
             active_scene->id);
    log_console_write("SceneRegistry", "EventsConnected", "scene_registry.c", log_msg);
    
    return true;
}
