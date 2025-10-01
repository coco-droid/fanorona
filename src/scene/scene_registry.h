#ifndef SCENE_REGISTRY_H
#define SCENE_REGISTRY_H

#include "scene.h"
#include <stdbool.h>

// Fonction principale pour enregistrer toutes les scènes disponibles
bool scene_registry_register_all(SceneManager* manager);

// Fonction pour connecter les événements de toutes les scènes enregistrées
bool scene_registry_connect_all_events(SceneManager* manager, GameCore* core);

#endif // SCENE_REGISTRY_H
