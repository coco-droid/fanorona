# Fanorona

Un jeu Fanorona développé en C avec SDL2.

## Structure du projet

```
fanoron-sivy/
├── main.c              # Point d'entrée principal avec la boucle de jeu
├── run.sh              # Script de compilation et d'exécution
├── Makefile            # Makefile pour la compilation
├── build/              # Dossier de compilation (créé automatiquement)
└── src/
    ├── core/
    │   ├── core.h      # Interface du core du jeu
    │   └── core.c      # Implémentation du core (capture les événements)
    ├── event/
    │   ├── event.h     # Interface de l'event manager
    │   └── event.c     # Système de gestion des événements
    ├── scene/
    │   ├── scene.h     # Système de gestion des scènes
    │   ├── scene_manager.c # Gestionnaire de scènes
    │   ├── scene_registry.c # Registre automatique des scènes
    │   ├── home_scene.c # Scène d'accueil
    │   ├── menu_scene.c # Scène de menu
    │   └── game_scene.c # Scène de jeu
    ├── ui/
    │   ├── components/
    │   │   └── ui_link.c # Composant de liens de navigation
    │   └── ui_components.h # Interface des composants UI
    └── window/
        ├── window.h    # Interface du window manager
        └── window.c    # Gestion des fenêtres SDL2
```

## Fonctionnalités

### Event Manager
- Système de souscription d'éléments aux événements par scène
- Gestion par position (x, y, largeur, hauteur) et z-index
- Attribut `display` pour contrôler la visibilité
- Fonctions pour souscrire, désinscrire et effacer les handlers
- **🆕 Debug intégré**: `event_manager_debug_elements()` pour diagnostiquer

### Scene Manager
- **🆕 Gestion par scène**: Chaque scène a son propre EventManager
- **🆕 Transitions automatiques**: Via SDL_USEREVENT et ui_link
- **🆕 Multi-fenêtres**: Support MINI (700x500) et MAIN (800x600)
- **🆕 Debug**: `scene_manager_debug_active_scenes()` pour diagnostiquer

### Core
- **🆕 Thread dédié**: Capture des événements en thread séparé
- **🆕 Buffer circulaire**: Queue thread-safe pour les événements
- **🆕 Routage intelligent**: Événements routés vers la bonne scène/fenêtre
- **🆕 Debug complet**: `game_core_debug_event_system()` pour diagnostiquer

### UI Links
- **🆕 Navigation entre scènes**: `ui_create_link()` pour créer des liens
- **🆕 Transitions automatiques**: Support de 4 types de transitions
- **🆕 Multi-fenêtres**: Transition entre MINI et MAIN window

## Compilation et exécution

### Avec le script run.sh (recommandé)
```bash
chmod +x run.sh
./run.sh
```

### Avec Make
```bash
make
make run
```

### Installation des dépendances (Ubuntu/Debian)
```bash
make install-deps
```

## Debug du système d'événements

**🔧 Logs réduits** : Les logs verbeux ont été considérablement réduits pour une meilleure lisibilité.

Si aucun événement ne fonctionne, utiliser ces fonctions de debug dans le code:

```c
// Dans main.c ou dans une fonction de debug
game_core_debug_event_system(core);  // Debug complet du système
scene_manager_debug_active_scenes(scene_manager);  // Debug des scènes
game_core_force_scene_event_registration(core);  // Force la re-connexion
```

**📊 Console plus propre** :
- ✅ Logs critiques conservés (QUIT, fermeture fenêtre)
- ❌ Logs de routine supprimés (clics, mouvements souris)
- ❌ Logs de rendu périodiques supprimés
- ❌ Logs de synchronisation verbeux supprimés

## Problèmes courants et solutions

### Événements non détectés
1. **Vérifier l'initialisation**: La scène doit être `initialized = true`
2. **Vérifier l'EventManager**: Chaque scène doit avoir son EventManager
3. **Vérifier l'enregistrement**: Les éléments UI doivent être enregistrés
4. **Vérifier la fenêtre**: L'événement doit venir de la bonne fenêtre

### Transitions qui ne fonctionnent pas
1. **Vérifier la connexion**: `ui_link_connect_to_manager()` doit être appelé
2. **Vérifier les IDs**: Les IDs de scène doivent correspondre
3. **Vérifier les fenêtres**: Les fenêtres cibles doivent être créées

## Utilisation de l'Event Manager

```c
// Créer un event manager par scène
EventManager* manager = event_manager_create();

// Debug des éléments enregistrés
event_manager_debug_elements(manager);

// Souscrire un élément
event_manager_subscribe(manager, x, y, width, height, z_index, true, callback_function, user_data);

// Gérer les événements
event_manager_handle_event(manager, &event);
```

## Exemple de callback

```c
void my_button_callback(SDL_Event* event, void* user_data) {
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        printf("Bouton cliqué!\n");
        
        // Debug optionnel
        printf("Position clic: (%d, %d)\n", event->button.x, event->button.y);
    }
}
```

## Navigation entre scènes

```c
// Créer un lien de navigation
UINode* link = ui_create_link(tree, "my-link", "Aller au menu", "menu", SCENE_TRANSITION_REPLACE);

// Connecter au SceneManager (OBLIGATOIRE)
ui_link_connect_to_manager(link, scene_manager);

// NOUVEAUTÉ: Ajouter un délai de sécurité pour éviter les clics prématurés
ui_link_set_activation_delay(link, 0.5f);  // 500ms avant activation

// Mettre à jour le lien dans la boucle de jeu
ui_link_update(link, delta_time);

// Le clic déclenchera automatiquement la transition après le délai
```

### Résolution de problèmes avec UI Links

Si vos transitions entre scènes fonctionnent de manière erratique:

1. **Délais de sécurité**: Utilisez `ui_link_set_activation_delay()` pour éviter les clics prématurés
2. **Vérifiez les IDs**: Assurez-vous que les IDs de scène correspondent exactement
3. **Connectez avant utilisation**: Appelez toujours `ui_link_connect_to_manager()`
4. **Mise à jour**: N'oubliez pas d'appeler `ui_link_update()` dans votre fonction `update()`

Pour déboguer les transitions:
```c
// Affichez toutes les scènes disponibles
for (int i = 0; i < manager->scene_count; i++) {
    Scene* s = manager->scenes[i];
    printf("[%d] ID:'%s' Name:'%s'\n", i, s->id, s->name);
}
```