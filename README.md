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
    │   ├── animation.h     # 🆕 Système d'animations keyframe-based
    │   ├── animation.c     # 🆕 Implémentation des animations
    │   ├── components/
    │   │   └── ui_link.c # Composant de liens de navigation
    │   └── ui_components.h # Interface des composants UI
    └── window/
        ├── window.h    # Interface du window manager
        └── window.c    # Gestion des fenêtres SDL2
```

## Fonctionnalités

### ✨ Système d'animations (NOUVEAU)
- **🎬 Animations keyframe-based**: Système inspiré de CSS avec support complet des keyframes
- **🔄 Fonctions d'easing**: linear, ease-in, ease-out, ease-in-out
- **📊 Propriétés animables**: Position (X,Y), taille (W,H), opacité
- **♾️ Contrôles avancés**: Itérations, va-et-vient, modes de remplissage
- **🎭 Animations prédéfinies**: Fade-in/out, slide, shake, pulse, bounce
- **⚡ Performance optimisée**: Nettoyage automatique des animations terminées

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

## ✨ Utilisation du système d'animations

### Animations prédéfinies (usage simple)

```c
// Dans votre fonction d'initialisation de scène
static void my_scene_init(Scene* scene) {
    UINode* titre = ui_text(tree, "titre", "FANORONA");
    UINode* bouton = ui_button(tree, "play-btn", "JOUER", on_play_click, NULL);
    
    // Animations d'entrée
    ANIMATE_FADE_IN(titre, 1.0f);                    // Apparition en 1 seconde
    ANIMATE_SLIDE_LEFT(bouton, 0.8f, 200.0f);       // Glissement depuis la gauche
    ANIMATE_PULSE(bouton, 2.0f);                     // Pulsation continue
}

// Dans votre fonction update (OBLIGATOIRE)
static void my_scene_update(Scene* scene, float delta_time) {
    // Mettre à jour les animations
    ui_update_animations(delta_time);
    
    // ...reste de votre logique...
}
```

### Animations personnalisées (usage avancé)

```c
// Créer une animation bounce personnalisée
Animation* bounce = animation_create("custom-bounce", ANIMATION_PROPERTY_Y, 1.5f);
animation_add_keyframe(bounce, 0.0f, 0.0f, "ease-out");     // Début
animation_add_keyframe(bounce, 0.3f, -40.0f, "ease-in");    // Premier saut
animation_add_keyframe(bounce, 0.6f, 0.0f, "ease-out");     // Retour
animation_add_keyframe(bounce, 0.8f, -20.0f, "ease-in");    // Petit saut
animation_add_keyframe(bounce, 1.0f, 0.0f, "ease-out");     // Position finale

// Configuration avancée
animation_set_iterations(bounce, 3);       // Répéter 3 fois
animation_set_alternate(bounce, false);    // Pas de va-et-vient

// Appliquer à un élément
ui_node_add_animation(logo, bounce);
```

### Animations d'erreur et feedback

```c
// Secouer un bouton en cas d'erreur
void show_error_on_button(UINode* button) {
    ANIMATE_SHAKE(button, 0.4f, 8.0f);  // Secousse 0.4s avec intensité 8px
}

// Animation de succès
void show_success_feedback(UINode* element) {
    // Animation combinée : scale + fade
    Animation* scale_up = animation_create("success-scale", ANIMATION_PROPERTY_WIDTH, 0.3f);
    animation_add_keyframe(scale_up, 0.0f, 100.0f, "ease-out");
    animation_add_keyframe(scale_up, 1.0f, 110.0f, "ease-out");
    
    ANIMATE_FADE_IN(element, 0.2f);
    ui_node_add_animation(element, scale_up);
}
```

### Transitions entre scènes

```c
// Animation de sortie avant changement de scène
void animate_scene_exit(UINode* container, void (*on_complete_callback)(void)) {
    // Faire disparaître la scène actuelle
    ANIMATE_FADE_OUT(container, 0.5f);
    
    // Programmer le changement de scène après l'animation
    // (utilisez un timer ou un callback dans votre système)
    schedule_callback(on_complete_callback, 0.5f);
}
```

### Macros disponibles

```c
ANIMATE_FADE_IN(node, duration)                    // Apparition en fondu
ANIMATE_FADE_OUT(node, duration)                   // Disparition en fondu
ANIMATE_SLIDE_LEFT(node, duration, distance)       // Glissement gauche
ANIMATE_SLIDE_RIGHT(node, duration, distance)      // Glissement droite
ANIMATE_SHAKE(node, duration, intensity)           // Secousse horizontale
ANIMATE_PULSE(node, duration)                      // Pulsation continue
STOP_ANIMATIONS(node)                              // Arrêter toutes les animations
```

### Debug et monitoring

```c
// Vérifier le nombre d'animations actives
int active_count = ui_get_active_animations_count();
printf("Animations actives: %d\n", active_count);

// Vérifier si un élément a des animations
if (ui_node_has_active_animations(mon_bouton)) {
    printf("Le bouton est en cours d'animation\n");
}

// Arrêter toutes les animations d'un élément
ui_node_stop_animations(mon_bouton);
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

### Animations qui ne fonctionnent pas
1. **Oublier ui_update_animations()** : Cette fonction DOIT être appelée dans chaque scene_update()
2. **Delta_time incorrect** : Vérifier que delta_time est en secondes, pas en millisecondes
3. **Nœud détruit** : Ne pas détruire un nœud qui a des animations actives
4. **Mémoire insuffisante** : Le système peut refuser de nouvelles animations si la mémoire est limitée

### Performance des animations
1. **Trop d'animations simultanées** : Limiter à ~20-30 animations actives maximum
2. **Nettoyage automatique** : Le système nettoie automatiquement les animations terminées
3. **Monitoring** : Utiliser `ui_get_active_animations_count()` pour surveiller

### Événements non détectés
1. **Vérifier l'initialisation**: La scène doit être `initialized = true`
2. **Vérifier l'EventManager**: Chaque scène doit avoir son EventManager
3. **Vérifier l'enregistrement**: Les éléments UI doivent être enregistrés
4. **Vérifier la fenêtre**: L'événement doit venir de la bonne fenêtre

### Transitions qui ne fonctionnent pas
1. **Vérifier la connexion**: `ui_link_connect_to_manager()` doit être appelé
2. **Vérifier les IDs**: Les IDs de scène doivent correspondre
3. **Vérifier les fenêtres**: Les fenêtres cibles doivent être créées