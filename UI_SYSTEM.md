# Système UI Atomique - Documentation

## Vue d'ensemble

Le système UI de Fanorona est basé sur une architecture atomique où tous les composants héritent d'un élément de base `AtomicElement` qui fournit des propriétés CSS-like et une gestion d'événements intégrée.

## Architecture

```
AtomicElement (base)
├── Propriétés CSS-like (position, taille, margin, padding, etc.)
├── Gestion d'événements intégrée
├── Rendu personnalisable
└── Hiérarchie parent-enfant

Components (héritent d'AtomicElement)
├── Button
├── Label (à venir)
├── Input (à venir)
└── Panel (à venir)
```

## AtomicElement - Élément de base

### Propriétés de style disponibles

```c
// Position et taille
atomic_set_position(element, x, y);
atomic_set_size(element, width, height);

// Espacement (comme CSS)
atomic_set_margin(element, top, right, bottom, left);
atomic_set_padding(element, top, right, bottom, left);

// Couleurs
atomic_set_background_color(element, r, g, b, a);
atomic_set_border(element, width, r, g, b, a);

// Affichage
atomic_set_display(element, DISPLAY_BLOCK | DISPLAY_NONE | DISPLAY_INLINE | DISPLAY_FLEX);
atomic_set_visibility(element, true/false);
atomic_set_z_index(element, index);
atomic_set_opacity(element, 0-255);
```

### Gestion du contenu

```c
// Texte et textures
atomic_set_text(element, "Mon texte");
atomic_set_texture(element, ma_texture);

// Hiérarchie
atomic_add_child(parent, enfant);
atomic_remove_child(parent, enfant);
```

### Événements

```c
// Gestionnaires d'événements
atomic_set_click_handler(element, ma_fonction_click);
atomic_set_hover_handler(element, ma_fonction_hover);
atomic_set_focus_handler(element, ma_fonction_focus);

// Enregistrement avec l'event manager
atomic_register_with_event_manager(element, event_manager);
```

## Button - Premier composant

### Utilisation de base

```c
// Créer un bouton
Button* btn = button_create("mon_bouton", "Cliquez-moi");

// Configurer
button_set_position(btn, 100, 50);
button_set_size(btn, 120, 40);
button_set_type(btn, BUTTON_TYPE_PRIMARY);

// Événement
button_set_click_callback(btn, ma_fonction, mes_donnees);

// Enregistrer les événements
button_register_events(btn, event_manager);

// Dans la boucle de rendu
button_update(btn, delta_time);
button_render(btn, renderer);

// Nettoyage
button_destroy(btn);
```

### Types de boutons disponibles

- `BUTTON_TYPE_PRIMARY` - Bleu, bouton principal
- `BUTTON_TYPE_SECONDARY` - Gris, bouton secondaire
- `BUTTON_TYPE_SUCCESS` - Vert, actions de succès
- `BUTTON_TYPE_WARNING` - Orange, avertissements
- `BUTTON_TYPE_DANGER` - Rouge, actions dangereuses

### États automatiques

- `BUTTON_STATE_NORMAL` - État par défaut
- `BUTTON_STATE_HOVERED` - Survol de la souris
- `BUTTON_STATE_PRESSED` - Bouton pressé
- `BUTTON_STATE_DISABLED` - Bouton désactivé

## Exemple complet

```c
#include "src/ui/button.h"

void exemple_utilisation() {
    // Créer l'event manager
    EventManager* manager = event_manager_create();
    
    // Créer des boutons
    Button* btn_save = button_create("save", "Sauvegarder");
    Button* btn_cancel = button_create("cancel", "Annuler");
    
    // Configurer
    button_set_position(btn_save, 50, 100);
    button_set_type(btn_save, BUTTON_TYPE_SUCCESS);
    button_set_click_callback(btn_save, on_save_clicked, NULL);
    
    button_set_position(btn_cancel, 180, 100);
    button_set_type(btn_cancel, BUTTON_TYPE_SECONDARY);
    button_set_click_callback(btn_cancel, on_cancel_clicked, NULL);
    
    // Enregistrer les événements
    button_register_events(btn_save, manager);
    button_register_events(btn_cancel, manager);
    
    // Dans la boucle principale
    while (running) {
        // Gérer événements
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            event_manager_handle_event(manager, &event);
        }
        
        // Mettre à jour
        button_update(btn_save, delta_time);
        button_update(btn_cancel, delta_time);
        
        // Rendre
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        button_render(btn_save, renderer);
        button_render(btn_cancel, renderer);
        
        SDL_RenderPresent(renderer);
    }
    
    // Nettoyage
    button_destroy(btn_save);
    button_destroy(btn_cancel);
    event_manager_destroy(manager);
}

void on_save_clicked(void* button, void* user_data) {
    printf("Sauvegarde en cours...\n");
}

void on_cancel_clicked(void* button, void* user_data) {
    printf("Annulation...\n");
}
```

## Scène de démonstration

Une scène de démonstration `button_demo_scene.c` est disponible pour tester tous les types de boutons. Pour l'utiliser :

```c
// Dans votre core ou main
Scene* demo_scene = create_button_demo_scene();
scene_manager_set_scene(scene_manager, demo_scene);
```

## Prochaines étapes

- Label component pour le texte
- Input component pour la saisie
- Panel component pour les conteneurs
- Layout automatique (flex, grid)
- Animations CSS-like
- Thèmes globaux