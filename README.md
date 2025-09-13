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
    └── window/
        ├── window.h    # Interface du window manager
        └── window.c    # Gestion des fenêtres SDL2
```

## Fonctionnalités

### Event Manager
- Système de souscription d'éléments aux événements
- Gestion par position (x, y, largeur, hauteur) et z-index
- Attribut `display` pour contrôler la visibilité
- Fonctions pour souscrire, désinscrire et effacer les handlers

### Core
- Capture les événements SDL2
- Transmet les événements à l'event manager
- Gère l'état de fonctionnement du jeu

### Window Manager
- Création et gestion des fenêtres SDL2
- Fonctions spécialisées pour créer une mini fenêtre (400x300) et une grande fenêtre (800x600)

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

## Utilisation de l'Event Manager

```c
// Créer un event manager
EventManager* manager = event_manager_create();

// Souscrire un élément
event_manager_subscribe(manager, x, y, width, height, z_index, true, callback_function, user_data);

// Gérer les événements
event_manager_handle_event(manager, &event);

// Désinscrire un élément
event_manager_unsubscribe(manager, callback_function, user_data);

// Effacer tous les éléments
event_manager_clear_all(manager);
```

## Exemple de callback

```c
void my_button_callback(SDL_Event* event, void* user_data) {
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        printf("Bouton cliqué!\n");
    }
}
```