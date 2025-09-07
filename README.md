# Fanorona Game Project

Un jeu de plateau traditionnel malgache implémenté en C avec SDL2, utilisant une architecture modulaire moderne.

## Table des Matières

- [Vue d'ensemble](#vue-densemble)
- [Architecture](#architecture)
- [Modules](#modules)
- [Installation](#installation)
- [Utilisation](#utilisation)
- [Exemples de Code](#exemples-de-code)
- [Développement](#développement)

## Vue d'ensemble

Fanorona est un jeu de stratégie traditionnel de Madagascar. Ce projet implémente une version moderne avec :
- Interface graphique avec SDL2
- Système de fenêtres multiples avec coins arrondis
- Gestionnaire de couches (layers) pour l'interface
- Support audio
- Intelligence artificielle
- Mode multijoueur réseau
- Animations fluides

## Architecture

Le projet suit une architecture modulaire en couches :

```
┌─────────────────────────────────────┐
│           Application               │
│  (main.c, scenes/, ui/)            │
├─────────────────────────────────────┤
│          Core Systems               │
│ (window/, layer/, event/, audio/)  │
├─────────────────────────────────────┤
│         Game Logic                  │
│    (engine/, ai/, analyzer/)       │
├─────────────────────────────────────┤
│        Infrastructure              │
│     (core/, net/, config/)         │
└─────────────────────────────────────┘
```

## Modules

### 1. Core System (`src/core/`)

**Rôle :** Initialisation et configuration de base

**Fichiers :**
- `sdl_init.h/c` : Gestion des fenêtres et initialisation SDL
- `timer.h/c` : Système de timing haute précision
- `config.h/c` : Configuration et paramètres

**Exemple d'utilisation :**
```c
CoreState core;
if (!core_init(&core)) {
    printf("Erreur d'initialisation\n");
    return 1;
}

// Basculer vers le menu
core_switch_to_menu(&core);

// Nettoyer
core_quit(&core);
```

### 2. Window Manager (`src/window/`)

**Rôle :** Gestion avancée des fenêtres avec coins arrondis

**Fonctionnalités :**
- Fenêtres multiples (menu, jeu, paramètres)
- Coins arrondis automatiques
- Masquage/affichage dynamique
- Icônes d'application

**Exemple d'utilisation :**
```c
WindowManager wm;
wm_init(&wm);

// Créer une fenêtre avec coins arrondis
GameWindow *win = wm_create_window(&wm, WINDOW_GAME, 
    "Mon Jeu", 800, 600, true, 15);

// Rendre avec callback personnalisé
wm_render_window(win, ma_fonction_rendu);

wm_quit(&wm);
```

### 3. Layer System (`src/layer/`)

**Rôle :** Système de couches pour l'interface utilisateur

**Composants :**
- `layer.h/c` : Couche de base avec transformations
- `layer_manager.h/c` : Gestionnaire de hiérarchie
- `dirty_rect.h/c` : Optimisation du rendu
- `render_target.h/c` : Cibles de rendu multiples

**Exemple d'utilisation :**
```c
LayerManager *lm = layer_manager_create();

// Créer des couches
Layer *bg = layer_create(0, 0, 800, 600);
Layer *ui = layer_create(10, 10, 200, 100);

// Hiérarchie
layer_manager_add_layer(lm, bg);
layer_add_child(bg, ui);

// Rendu automatique
layer_manager_render(lm, renderer);
```

### 4. UI System (`src/ui/`)

**Rôle :** Widgets et interface utilisateur

**Widgets disponibles :**
- `widget.h/c` : Widget de base
- `button.h/c` : Boutons interactifs
- `pieces_widget.h/c` : Pièces de jeu avec drag & drop
- `animation.h/c` : Système d'animations

**Exemple d'utilisation :**
```c
// Créer un bouton
Button *btn = button_create("Jouer", 100, 50, 150, 40);
button_set_callback(btn, on_play_clicked, NULL);

// Créer une pièce de jeu
Piece piece = WHITE_PIECE;
PieceWidget *pw = piece_widget_create(&piece);
piece_widget_set_highlight(pw, true);

// Animation
piece_widget_animate_to(pw, (SDL_Point){200, 200}, 1.0);
```

### 5. Scene Management (`src/scenes/`)

**Rôle :** Gestion des écrans du jeu

**Scènes :**
- Menu principal
- Jeu principal
- Paramètres

**Exemple d'utilisation :**
```c
Scene *game_scene = game_scene_create();
game_scene->init(game_scene);
game_scene->layout(game_scene, 1024, 768);

// Dans la boucle principale
layer_manager_render(game_scene->lm, renderer);
```

### 6. Game Engine (`src/engine/`)

**Rôle :** Logique de jeu Fanorona

**Fonctionnalités :**
- Règles du jeu
- Validation des mouvements
- État de jeu
- Historique des coups

**Exemple d'utilisation :**
```c
GameState *state = game_state_create();

// Effectuer un mouvement
Move move = {.from = {0, 0}, .to = {0, 1}, .type = MOVE_APPROACH};
if (fanorona_is_valid_move(state, &move)) {
    fanorona_apply_move(state, &move);
}

// Vérifier fin de partie
if (fanorona_is_game_over(state)) {
    Player winner = fanorona_get_winner(state);
}
```

### 7. Event System (`src/event/`)

**Rôle :** Gestion des événements et interactions

**Composants :**
- Dispatcher d'événements
- Hitboxes et détection de collision
- Utilitaires de coordonnées

**Exemple d'utilisation :**
```c
EventDispatcher *ed = event_dispatcher_create();

// Enregistrer un handler
event_dispatcher_register(ed, SDL_MOUSEBUTTONDOWN, on_mouse_click);

// Dans la boucle d'événements
SDL_Event e;
while (SDL_PollEvent(&e)) {
    event_dispatcher_handle(ed, &e);
}
```

### 8. Audio System (`src/audio/`)

**Rôle :** Effets sonores et musique

**Utilisation :**
```c
if (audio_init()) {
    audio_play("move");     // Son de mouvement
    audio_play("capture");  // Son de capture
}
```

### 9. AI System (`src/ai/`)

**Rôle :** Intelligence artificielle

**Algorithmes :**
- Minimax avec élagage alpha-beta
- Réseau de neurones (GNN)

### 10. Network (`src/net/`)

**Rôle :** Multijoueur peer-to-peer

## Installation

### Prérequis

```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev

# Arch Linux
sudo pacman -S sdl2 sdl2_ttf sdl2_image sdl2_mixer

# macOS
brew install sdl2 sdl2_ttf sdl2_image sdl2_mixer
```

### Compilation

```bash
# Compilation simple
./run.sh

# Mode debug
./run.sh --debug

# Clean build
./run.sh --clean
```

## Utilisation

### Lancer le jeu

```bash
cd build
./fanorona
```

### Contrôles

- **ESPACE** : Basculer entre menu et jeu (mode test)
- **Clic souris** : Sélectionner/déplacer pièces
- **ESC** : Fermer fenêtre

## Exemples de Code

### Créer une nouvelle fenêtre personnalisée

```c
#include "window/window_manager.h"

void create_custom_window() {
    WindowManager wm;
    wm_init(&wm);
    
    // Fenêtre avec coins très arrondis
    GameWindow *win = wm_create_window(&wm, WINDOW_MENU,
        "Ma Fenêtre", 600, 400, true, 25);
    
    wm_show_window(&wm, WINDOW_MENU);
}
```

### Ajouter un nouveau widget

```c
#include "ui/widget.h"

typedef struct {
    Widget base;
    char text[64];
    SDL_Color color;
} LabelWidget;

LabelWidget *label_create(const char *text) {
    LabelWidget *label = malloc(sizeof(LabelWidget));
    // Initialiser le widget de base
    widget_init((Widget*)label, WIDGET_LABEL);
    
    strcpy(label->text, text);
    label->color = (SDL_Color){255, 255, 255, 255};
    
    return label;
}
```

### Gérer les événements personnalisés

```c
#include "event/event_dispatcher.h"

void on_piece_moved(SDL_Event *e, void *user_data) {
    printf("Pièce déplacée !\n");
    audio_play("move");
}

void setup_events() {
    EventDispatcher *ed = event_dispatcher_create();
    event_dispatcher_register(ed, SDL_MOUSEBUTTONUP, on_piece_moved);
}
```

### Créer une animation personnalisée

```c
#include "ui/animation.h"

void animate_piece_capture() {
    PieceWidget *piece = /* obtenir la pièce */;
    
    // Animation de disparition
    Animation *fade = animation_create(ANIM_FADE_OUT, 0.5);
    animation_start(fade, piece);
    
    // Callback à la fin
    animation_set_completion_callback(fade, on_piece_removed, piece);
}
```

## Développement

### Structure de fichiers recommandée

```
src/
├── core/          # Systèmes de base
├── window/        # Gestion fenêtres
├── layer/         # Système de couches
├── ui/            # Interface utilisateur
├── scenes/        # Écrans du jeu
├── engine/        # Logique de jeu
├── event/         # Gestion événements
├── audio/         # Système audio
├── ai/            # Intelligence artificielle
├── net/           # Réseau
└── analyzer/      # Analyse post-partie
```

### Conventions de nommage

- **Fichiers** : `snake_case.c/h`
- **Fonctions** : `module_function_name()`
- **Types** : `PascalCase`
- **Constantes** : `UPPER_CASE`

### Cycle de développement

1. **Modifier le code** dans `src/`
2. **Compiler** avec `./run.sh --debug`
3. **Tester** les fonctionnalités
4. **Optimiser** en mode release

### Extensions possibles

- Nouveaux types de widgets
- Effets visuels avancés
- Modes de jeu supplémentaires
- Sauvegarde/chargement de parties
- Replay des parties
- Éditeur de plateau

## Dépannage

### Problèmes courants

**Erreur de compilation SDL2 :**
```bash
# Vérifier l'installation
pkg-config --exists sdl2 && echo "SDL2 OK"
```

**Icône manquante :**
```bash
# Créer une icône de test
convert -size 32x32 xc:blue icone.png
```

**Fenêtre ne s'affiche pas :**
- Vérifier les drivers graphiques
- Tester en mode fenêtré plutôt que plein écran

---

**Auteur :** Projet Fanorona  
**Version :** 1.0  
**Licence :** MIT
