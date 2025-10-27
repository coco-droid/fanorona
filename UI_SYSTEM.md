# Système UI Atomique - Documentation

## Vue d'ensemble

Le système UI de Fanorona est basé sur une architecture atomique où tous les composants héritent d'un élément de base `AtomicElement` qui fournit des propriétés CSS-like et une gestion d'événements intégrée.

**🆕 Nouvelles fonctionnalités :**
- ✅ **Moteur de rendu Optimum** - Rendu dédié et optimisé séparé de la logique atomique
- ✅ **Logs de traçage des événements** pour debugging
- ✅ **Z-index implicites** basés sur l'ordre d'ajout
- ✅ **Support des images PNG** pour les backgrounds de boutons
- ✅ **Correction de l'affichage du texte** sur les boutons
- 🆕 **Feedback visuel interactif** pour les clics et survols
- 🆕 **Composant Container** avec style modal
- 🔧 **API atomic simplifiée** avec fonctions unifiées
- 🔧 **Correction du rendu avec padding** - les éléments respectent maintenant les content_rect
- 🆕 **Gestion des débordements par calcul** - plus de clipping SDL, contraintes intelligentes
- 🆕 **Calculs de position absolue** - SET_POS résolu en coordonnées écran réelles
- 🎯 **Synchronisation post-calculs des hitboxes** - Assignation après tous les calculs de layout
- ✨ **Système d'animations keyframe-based** - Animations CSS-like avec easing

## ✨ Système d'animations keyframe-based

Le nouveau système d'animation permet de créer des animations fluides inspirées de CSS avec support complet des keyframes et fonctions d'easing.

### Architecture du système d'animation

```c
// CRÉATION D'ANIMATION
Animation* anim = animation_create("mon-animation", ANIMATION_PROPERTY_X, 2.0f);
├── animation_add_keyframe(anim, 0.0f, 0.0f, "ease-out");    // Début
├── animation_add_keyframe(anim, 0.5f, 200.0f, "ease-in");   // Milieu
└── animation_add_keyframe(anim, 1.0f, 100.0f, "ease-out");  // Fin

// APPLICATION À UN NOEUD
ui_node_add_animation(mon_bouton, anim);

// MISE À JOUR AUTOMATIQUE (dans la boucle de jeu)
ui_update_animations(delta_time);
```

### Propriétés animables

```c
typedef enum {
    ANIMATION_PROPERTY_X,        // Position horizontale
    ANIMATION_PROPERTY_Y,        // Position verticale
    ANIMATION_PROPERTY_WIDTH,    // Largeur
    ANIMATION_PROPERTY_HEIGHT,   // Hauteur
    ANIMATION_PROPERTY_OPACITY,  // Transparence (0-255)
    ANIMATION_PROPERTY_SCALE_X,  // Échelle horizontale (futur)
    ANIMATION_PROPERTY_SCALE_Y,  // Échelle verticale (futur)
    ANIMATION_PROPERTY_ROTATION  // Rotation (futur)
} AnimationProperty;
```

### Fonctions d'easing disponibles

- **"linear"** : Animation linéaire
- **"ease-in"** : Accélération progressive
- **"ease-out"** : Décélération progressive  
- **"ease-in-out"** : Accélération puis décélération

### Animations prédéfinies

```c
// Apparition en fondu
Animation* fade_in = animation_fade_in(1.0f);
ui_node_add_animation(mon_element, fade_in);

// Disparition en fondu
Animation* fade_out = animation_fade_out(0.8f);
ui_node_add_animation(mon_element, fade_out);

// Glissement depuis la gauche
Animation* slide_left = animation_slide_in_left(1.2f, 300.0f);
ui_node_add_animation(mon_element, slide_left);

// Glissement depuis la droite
Animation* slide_right = animation_slide_in_right(1.0f, 250.0f);
ui_node_add_animation(mon_element, slide_right);

// Secousse horizontale
Animation* shake = animation_shake_x(0.5f, 10.0f);
ui_node_add_animation(mon_element, shake);

// Pulsation infinie
Animation* pulse = animation_pulse(2.0f);
animation_set_iterations(pulse, -1); // Infini
ui_node_add_animation(mon_element, pulse);
```

### API simplifiée avec macros

```c
// Macros pour usage rapide
ANIMATE_FADE_IN(mon_bouton, 1.0f);
ANIMATE_SLIDE_LEFT(mon_titre, 0.8f, 200.0f);
ANIMATE_SHAKE(bouton_erreur, 0.3f, 5.0f);
ANIMATE_PULSE(bouton_important, 1.5f);

// Arrêter toutes les animations
STOP_ANIMATIONS(mon_element);
```

### Contrôles avancés d'animation

```c
// Créer une animation personnalisée
Animation* custom = animation_create("bounce", ANIMATION_PROPERTY_Y, 2.0f);
animation_add_keyframe(custom, 0.0f, 0.0f, "ease-out");
animation_add_keyframe(custom, 0.25f, -50.0f, "ease-in");
animation_add_keyframe(custom, 0.5f, 0.0f, "ease-out");
animation_add_keyframe(custom, 0.75f, -25.0f, "ease-in");
animation_add_keyframe(custom, 1.0f, 0.0f, "ease-out");

// Configuration avancée
animation_set_iterations(custom, 3);      // 3 répétitions
animation_set_alternate(custom, true);    // Va-et-vient
animation_set_fill_mode(custom, "forwards"); // Garder la valeur finale

ui_node_add_animation(logo, custom);
```

### Intégration dans les scènes

```c
// Dans menu_scene.c - Animations d'entrée
static void menu_scene_init(Scene* scene) {
    // ...création des éléments...
    
    // Animation du container principal
    ui_animate_fade_in(modal_container, 0.8f);
    
    // Animation des boutons avec délais
    ui_animate_slide_in_left(multiplayer_btn, 1.0f, 300.0f);
    ui_animate_pulse(wiki_btn, 2.0f); // Pulsation continue
    
    // ...reste de l'init...
}

// Dans la fonction update - OBLIGATOIRE
static void menu_scene_update(Scene* scene, float delta_time) {
    // Mettre à jour les animations
    ui_update_animations(delta_time);
    
    // ...reste de l'update...
}
```

### Exemple complet d'animation de transition

```c
// Animation de sortie de scène
void transition_out_scene(UINode* container) {
    // Créer une animation de sortie customisée
    Animation* slide_out = animation_create("exit-transition", ANIMATION_PROPERTY_X, 0.6f);
    animation_add_keyframe(slide_out, 0.0f, 0.0f, "ease-in");
    animation_add_keyframe(slide_out, 1.0f, -800.0f, "ease-in");
    
    Animation* fade_out = animation_fade_out(0.6f);
    
    // Appliquer les deux animations en parallèle
    ui_node_add_animation(container, slide_out);
    ui_node_add_animation(container, fade_out);
    
    printf("🎬 Transition de sortie démarrée\n");
}
```

### Gestion automatique des ressources

Le système d'animation gère automatiquement :
- ✅ **Nettoyage des animations terminées**
- ✅ **Gestion mémoire des keyframes**
- ✅ **Interpolation fluide entre keyframes**
- ✅ **Support des itérations et va-et-vient**
- ✅ **Performance optimisée** (suppression automatique des animations inactives)

### Logs et debugging

```
✨ Animation 'fade-in' created (property: 4, duration: 1.00s)
🔧 Keyframe added to 'fade-in': time=0.00, value=0.00, easing=ease-out
🔧 Keyframe added to 'fade-in': time=1.00, value=255.00, easing=ease-out
🎬 Animation 'fade-in' started on node 'modal-container' (start value: 255.00)
🎬 Active animations: 3 (completed: 1 this frame)
```

### Intégration avec les autres systèmes

- 🔗 **Compatible avec le feedback visuel** : Les animations coexistent avec les effets de scale des boutons
- 🔗 **Compatible avec les neon buttons** : Les animations peuvent s'appliquer aux boutons neon
- 🔗 **Compatible avec les transitions de scènes** : Animations de sortie/entrée lors des changements de scène
- 🔗 **Performance intégrée** : Mise à jour dans la boucle principale sans overhead

Cette implémentation offre un système d'animation puissant et flexible, permettant de créer des interfaces utilisateur modernes et engageantes ! ✨🎬

## 🎯 Système de synchronisation post-calculs des hitboxes

Le nouveau système garantit que les hitboxes sont assignées avec les positions finales calculées, après tous les calculs de layout :

### Architecture de synchronisation en 3 phases

```c
// PHASE 1: Calculs de layout complets
ui_tree_update(tree, delta_time);
├── ui_tree_update_node_recursive()  // Tous les calculs individuels
├── atomic_calculate_layout()         // Flexbox, align-self, contraintes
└── atomic_apply_overflow_constraints() // Finalisation positions

// PHASE 2: Synchronisation des hitboxes (NOUVEAU)
optimum_sync_all_hitboxes_post_layout(tree);
├── optimum_sync_element_hitbox_recursive() // Parcours complet de l'arbre
├── atomic_get_final_render_rect()          // Position finale calculée
└── atomic_sync_event_manager_position()    // Mise à jour EventManager

// PHASE 3: Rendu avec hitboxes exactes
optimum_render_ui_tree(tree, renderer);
└── event_manager_render_hitboxes()  // Hitboxes aux bonnes positions
```

### Avantages de la synchronisation post-calculs

- 🎯 **Positions exactes** : Hitboxes basées sur les positions finales après tous les calculs
- 🔄 **Ordre unifié** : Même séquence que le moteur de rendu (calculs → positions → affichage)
- ⚡ **Performance optimisée** : Une seule synchronisation par frame au lieu de multiples
- 🐛 **Debugging précis** : Les hitboxes correspondent exactement aux éléments visibles
- 🔧 **Maintenance simplifiée** : Logique centralisée dans le moteur Optimum

### Utilisation du nouveau système

```c
// L'ancienne méthode (problématique) :
atomic_update(element, delta_time);  // Calculs individuels
atomic_register_with_event_manager(element, manager); // Position peut être incorrecte !

// La nouvelle méthode (automatique) :
ui_tree_update(tree, delta_time);    // Tout est géré automatiquement :
                                     // 1. Calculs complets
                                     // 2. Synchronisation hitboxes
                                     // 3. Prêt pour le rendu
```

### API de synchronisation

```c
// Fonction principale (appelée automatiquement par ui_tree_update)
void optimum_sync_all_hitboxes_post_layout(UITree* tree);

// Parcours récursif pour synchroniser chaque élément
void optimum_sync_element_hitbox_recursive(UINode* node, EventManager* manager);

// Synchronisation individuelle (maintenant plus précise)
void atomic_sync_event_manager_position(AtomicElement* element, EventManager* manager);

// Position finale calculée (utilisée pour les hitboxes)
SDL_Rect atomic_get_final_render_rect(AtomicElement* element);
```

### Logs de synchronisation

```
[14:32:15] [UITree] [UpdateStarted] [ui_tree.c] : 🔄 Starting complete UI tree update
[14:32:15] [UITree] [LayoutCalculated] [ui_tree.c] : ✅ All layout calculations completed
[14:32:15] [OptimumSync] [SyncStarted] [optimum.c] : 🎯 Starting post-layout hitbox synchronization
[14:32:15] [AtomicSync] [ElementSynced] [atomic.c] : Element 'play-button' synchronized - Final position: (120,200,150x40)
[14:32:15] [AtomicSync] [ElementSynced] [atomic.c] : Element 'quit-button' synchronized - Final position: (120,260,150x40)
[14:32:15] [OptimumSync] [SyncCompleted] [optimum.c] : ✅ Post-layout hitbox synchronization completed
[14:32:15] [UITree] [HitboxesSynced] [ui_tree.c] : 🎯 All hitboxes synchronized with final positions
[14:32:15] [UITree] [UpdateCompleted] [ui_tree.c] : ✅ UI tree update completed (layout + hitboxes)
```

### Ordre d'exécution garanti

1. **Calculs individuels** : `atomic_update()` pour chaque élément
2. **Calculs de layout** : Flexbox, align-self, contraintes d'overflow
3. **Positions finales** : Tous les ajustements terminés
4. **Synchronisation hitboxes** : `optimum_sync_all_hitboxes_post_layout()`
5. **Rendu** : Les hitboxes correspondent parfaitement aux éléments

Cette approche garantit que les événements sont assignés aux bonnes positions, même après des calculs complexes de layout comme le flexbox ou l'align-self ! 🎯✨

## 🎨 Moteur de rendu Optimum

### Utilisation du moteur Optimum

```c
// Rendu d'un élément individuel
optimum_render_element(button->element, renderer);

// Rendu d'un arbre UI complet (recommandé) - inclut automatiquement les hitboxes
optimum_render_ui_tree(tree, renderer);

// 🆕 Synchronisation post-calculs (appelée automatiquement par ui_tree_update)
optimum_sync_all_hitboxes_post_layout(tree);

// Debug des limites d'éléments
optimum_debug_render_bounds(element, renderer, true);

// Contrôler la visualisation des hitboxes
ui_set_hitbox_visualization(true);  // Afficher les hitboxes (par défaut)
ui_set_hitbox_visualization(false); // Masquer les hitboxes

// Nettoyage des ressources
optimum_cleanup();
```

## 🎯 Système de visualisation des hitboxes intégré

Le moteur Optimum inclut maintenant un système de visualisation des hitboxes synchronisées avec les positions finales calculées :

```c
// Activation/désactivation globale
ui_set_hitbox_visualization(true);  // Activé par défaut

// Les hitboxes apparaissent automatiquement après la synchronisation post-calculs :
// - Rectangle rouge transparent (alpha: 30)
// - Bordure bleue opaque (2px d'épaisseur)
// - Position exacte finale après tous les calculs de flexbox, align-self, etc.
// - Support de la sélection avec un cercle vert et une croix blanche
```

**🎨 Caractéristiques des hitboxes synchronisées :**
- 🔴 **Fond rouge transparent** : Visualise la zone cliquable finale
- 🔵 **Bordure bleue** : Délimite précisément les limites calculées
- 📊 **Position post-calculs** : Utilise `atomic_get_final_render_rect()` après tous les ajustements
- ⚡ **Synchronisation unifiée** : Une seule passe après tous les calculs de l'arbre
- 🔧 **Précision garantie** : Les hitboxes correspondent exactement aux éléments visibles
- ✅ **Système de sélection visuelle** : Cercle vert épais + croix blanche pour la sélection, cercle doré fin pour le hover

### Migration transparente

L'ancienne fonction `atomic_render()` redirige automatiquement vers Optimum :

```c
// Cette ligne fonctionne toujours (compatibilité)
atomic_render(element, renderer);

// Mais elle appelle maintenant en interne :
optimum_render_element(element, renderer);
```

**🎯 Exemple complet avec hitboxes :**

```c
#include "src/ui/ui_components.h"

void create_debug_interface() {
    // Activer la visualisation des hitboxes
    ui_set_hitbox_visualization(true);
    
    UITree* tree = ui_tree_create();
    ui_set_global_tree(tree);
    
    // Créer des éléments
    UINode* container = UI_DIV(tree, "container");
    SET_POS(container, 50, 50);
    SET_SIZE(container, 300, 200);
    SET_BG(container, "rgb(100, 100, 100)");
    
    UINode* button = ui_button(tree, "test-btn", "Test", on_test_click, NULL);
    SET_POS(button, 20, 30);
    SET_SIZE(button, 120, 40);
    
    // Construction hiérarchie
    APPEND(tree->root, container);
    APPEND(container, button);
    
    // Les hitboxes seront automatiquement visibles :
    // - Rectangle rouge transparent pour chaque élément
    // - Bordure bleue pour délimiter précisément les zones cliquables
    // - Position exacte après tous les calculs de flexbox, align-self, etc.
}

// Fonction de rendu principale
void render_with_hitboxes(UITree* tree, SDL_Renderer* renderer) {
    // Un seul appel suffit - les hitboxes sont incluses automatiquement
    optimum_render_ui_tree(tree, renderer);
    
    // Les hitboxes apparaissent par-dessus les éléments normaux
    // Permettant de vérifier visuellement que les zones cliquables
    // correspondent bien aux positions des éléments à l'écran
}
```

Cette intégration des hitboxes permet de déboguer visuellement les problèmes de positionnement et de détection d'événements de manière très efficace ! 🎯✨

## 🎯 Système de feedback visuel :
- 🎨 **États visuels automatiques** : hover, pressed, normal
- 🌈 **Styles prédéfinis** : success, danger, info, warning
- ⚡ **Animations de clic** avec effets de taille et couleur
- 📊 **Logs détaillés** de tous les changements visuels

**🔧 Correction majeure du rendu :**
- ✅ **Respect du padding** : Les éléments enfants sont maintenant rendus dans le `content_rect` du parent
- ✅ **Pas de chevauchement** : Les bordures et le padding sont correctement respectés
- ✅ **Calcul correct des positions** : Utilisation de `atomic_get_render_rect()` et `atomic_get_content_rect()`
- 🆕 **Contraintes par calcul** : `atomic_constrain_child_position()` empêche les débordements
- 🆕 **Positions absolues calculées** : SET_POS(x,y) devient coordonnées écran réelles tenant compte du parent

**⚡ Système de contraintes intelligentes avec positions absolues :**

```c
// Les positions relatives sont automatiquement converties en absolues
UINode* parent = UI_DIV(tree, "container");
SET_POS(parent, 100, 50);  // Position absolue à l'écran
SET_SIZE(parent, 200, 150);

UINode* child = UI_DIV(tree, "child");
SET_POS(child, 20, 30);    // Position RELATIVE au content_rect du parent
SET_SIZE(child, 80, 60);   

APPEND(parent, child);

// Après rendu, l'enfant apparaîtra à (120+padding, 80+padding) à l'écran
// Car atomic_get_render_rect() calcule : parent_content_rect + child_relative_pos + margins
```

**📊 Avantages des contraintes par calcul :**
- 🧮 **Calculs précis** : Positions optimales calculées mathématiquement
- 🚫 **Zéro débordement** : Impossible pour un enfant de sortir des limites
- ⚡ **Performance native** : Pas d'overhead SDL de clipping
- 🎛️ **Contrôle flexible** : Overflow configurable par élément
- 🔧 **Debug facile** : Logs détaillés des contraintes appliquées
- 🎯 **Positions réelles** : SET_POS devient coordonnées écran absolues

## 🎛️ Gestion des débordements (Overflow)

### Types d'overflow disponibles

```c
// Types de gestion des débordements
OVERFLOW_VISIBLE(element);   // Les enfants peuvent déborder (défaut)
OVERFLOW_HIDDEN(element);    // Les enfants sont contraints dans les limites
OVERFLOW_SCROLL(element);    // Avec scroll (futur développement)
OVERFLOW_AUTO(element);      // Comportement automatique

// Ou avec la fonction
ui_set_overflow(element, "visible");  // Débordement autorisé
ui_set_overflow(element, "hidden");   // Débordement contraint
ui_set_overflow(element, "scroll");   // Avec scroll
ui_set_overflow(element, "auto");     // Automatique
```

### Exemples d'utilisation

```c
// Container avec contraintes strictes
UINode* dialog = UI_CONTAINER(tree, "my-modal");
SET_SIZE(dialog, 400, 300);
OVERFLOW_HIDDEN(dialog); // Les enfants ne peuvent pas déborder

// Ajouter du contenu - sera automatiquement contraint
UINode* large_content = UI_DIV(tree, "content");
SET_SIZE(large_content, 500, 400); // Plus grand que le parent !
ui_container_add_content(dialog, large_content);

// Le contenu sera automatiquement redimensionné à 396x296 
// pour rester dans les limites du parent (400-4 pour padding)
```

### Contraintes intelligentes

```c
// Le système calcule automatiquement les positions optimales
UINode* container = UI_DIV(tree, "container");
SET_SIZE(container, 200, 150);
SET_POS(container, 50, 50);
OVERFLOW_HIDDEN(container);

UINode* child = UI_DIV(tree, "child");
SET_SIZE(child, 100, 80);
SET_POS(child, 180, 140); // Position qui déborderait

APPEND(container, child);

// Après update(), l'enfant sera repositionné à (148, 118)
// pour rester complètement dans le parent
ui_tree_update(tree, 0.0f);
```

### API de contrôle des débordements

```c
// Fonctions de contrôle
bool is_overflowing = ui_is_child_overflowing(parent, child);
ui_constrain_all_children(parent); // Force la contrainte immédiate

// Vérification et ajustement
if (ui_is_child_overflowing(dialog, button)) {
    printf("Le bouton déborde du dialogue !\n");
    ui_constrain_all_children(dialog); // Corriger immédiatement
}
```

### Logs des contraintes

```c
// Avec ui_set_event_logging(true), vous verrez :
// [UIComponent] [Style] [my-dialog] : Overflow set to hidden - children constrained within bounds
// [UIComponent] [Layout] [my-dialog] : All children positions constrained to parent bounds
// [UIComponent] [ConstraintApplied] [big-button] : Child position constrained from (250,200) to (196,146)
```

**📊 Avantages de cette approche :**
- 🧮 **Calculs précis** : Positions optimales calculées mathématiquement
- 🚫 **Zéro débordement** : Impossible pour un enfant de sortir des limites
- ⚡ **Performance native** : Pas d'overhead SDL de clipping
- 🎛️ **Contrôle flexible** : Overflow configurable par élément
- 🔧 **Debug facile** : Logs détaillés des contraintes appliquées

## 📦 Composant Container

### Style modal automatique avec contenu par défaut et align-self

```c
// Créer un container avec style modal + logo et sous-titre automatiques
UINode* modal = UI_CONTAINER(tree, "my-modal");
SET_SIZE(modal, 400, 300);
ALIGN_SELF_BOTH(modal); // Centrage automatique

// Le container inclut maintenant automatiquement :
// - Logo Fanorona à 9px DEPUIS L'INTÉRIEUR (bordure + padding + marge), centré avec align-self center-x
// - Texte "Stratégie et Tradition" à 94px DEPUIS L'INTÉRIEUR, centré avec align-self center-x, margin-bottom 2px
// - Fond noir transparent (alpha: 180) avec bordure orange
// - Padding interne de 2px - tous les éléments sont À L'INTÉRIEUR des bordures
// - Positionnement absolu (plus de flexbox interne)
```

### Ajout de contenu avec align-self automatique

```c
// Container centré avec contraintes strictes
UINode* dialog = UI_CONTAINER(tree, "my-modal");
SET_SIZE(dialog, 400, 300);
OVERFLOW_HIDDEN(dialog); // Les enfants ne peuvent pas déborder

// Ajouter un en-tête orange
ui_container_add_header(dialog, "CONFIGURATION");

// Ajouter du contenu
UINode* logo = UI_IMAGE(tree, "logo", logo_texture);
ui_container_add_content(dialog, logo);

UINode* text = UI_TEXT(tree, "subtitle", "Stratégie et Tradition");
ui_set_text_color(text, "rgb(255, 255, 255)");
ui_set_text_style(text, false, true); // Italique
ui_container_add_content(dialog, text);

// Le container organise automatiquement le contenu en colonne centrée
```

### Système align-self pour centrage intelligent

```c
// Nouveau système de centrage par axe
ALIGN_SELF_X(element);    // Centrage horizontal uniquement
ALIGN_SELF_Y(element);    // Centrage vertical uniquement  
ALIGN_SELF_BOTH(element); // Centrage horizontal + vertical

// Ou avec la fonction
ui_set_align_self(element, "center-x");     // Horizontal
ui_set_align_self(element, "center-y");     // Vertical
ui_set_align_self(element, "center-both");  // Les deux
ui_set_align_self(element, "auto");         // Désactiver

// 🆕 EXEMPLE D'USAGE DANS UN CONTAINER :
UINode* modal = UI_CONTAINER_CENTERED(tree, "dialog", 400, 300);
UINode* content = UI_DIV(tree, "content");
SET_SIZE(content, 300, 200);
ALIGN_SELF_Y(content);                     // Ajouter le centrage Y

ui_container_add_content(modal, content);  // Centrage X automatique

// Le contenu sera maintenant parfaitement centré dans le modal !
// Centrage X : align-self center-x (automatique)
// Centrage Y : align-self center-y (ajouté manuellement)
```

### Macros disponibles pour la construction UI

```c
// Macros de base
SET_POS(node, x, y);        // Position
SET_SIZE(node, w, h);       // Taille
SET_BG(node, color);        // Couleur de fond
CENTER(node);               // Centrage automatique

// Macros flexbox
FLEX_ROW(node);             // Direction row
FLEX_COLUMN(node);          // Direction column
APPEND(parent, child);      // Ajouter à la hiérarchie

// Macros align-self (nouveau)
ALIGN_SELF_X(node);         // Centrage horizontal
ALIGN_SELF_Y(node);         // Centrage vertical  
ALIGN_SELF_BOTH(node);      // Centrage complet

// Exemple d'usage avec le nouveau container :
UINode* dialog = UI_CONTAINER_CENTERED(tree, "dialog", 400, 300);
UINode* content = UI_DIV(tree, "content");
SET_SIZE(content, 300, 100);
ALIGN_SELF_X(content);      // Se centre automatiquement
ui_container_add_content(dialog, content);
```

### Utilisation avec contenu

```c
// Container centré avec taille définie
UINode* dialog = UI_CONTAINER_CENTERED(tree, "dialog", 500, 400);

// Ajouter un en-tête orange
ui_container_add_header(dialog, "CONFIGURATION");

// Ajouter du contenu
UINode* logo = UI_IMAGE(tree, "logo", logo_texture);
ui_container_add_content(dialog, logo);

UINode* text = UI_TEXT(tree, "subtitle", "Stratégie et Tradition");
ui_set_text_color(text, "rgb(255, 255, 255)");
ui_set_text_style(text, false, true); // Italique
ui_container_add_content(dialog, text);

// Le container organise automatiquement le contenu en colonne centrée
```

### Styles de container

```c
// Style modal complet (overlay sombre, z-index élevé)
ui_container_set_modal_style(container, true);

// Style normal (fond blanc, bordure grise)
ui_container_set_modal_style(container, false);

// Le container s'adapte automatiquement :
// Modal: z-index 1000, fond noir 200 alpha, bordure orange 3px
// Normal: z-index normal, fond blanc 230 alpha, bordure grise 1px
```

### Logs de container

```c
// Avec ui_set_event_logging(true), vous verrez :
// [UIComponent] [Create] [my-modal] : Container created with modal style (black overlay, orange border)
// [UIComponent] [Style] [my-modal] : Container size set
// [UIComponent] [Style] [my-modal] : Container centered
// [UIComponent] [ContainerHeader] [my-modal] : Header added to container
// [UIComponent] [ContainerContent] [my-modal] : Content added to container
```

## 🎨 Feedback visuel pour les boutons

### États interactifs automatiques

```c
// Les boutons réagissent automatiquement aux interactions :

// ✨ HOVER (Survol)
// - Effet de scale automatique (+5% de taille) avec transition fluide
// - Overlay blanc translucide (alpha: 30)
// - Animation fluide de 0.15 secondes

// 🎯 CLICK (Clic)
// - Effet de scale inverse (-3% de taille) 
// - Couleur de fond contextuelle (vert pour Play, rouge pour Quit)
// - Texte contrasté automatiquement

// 🔄 NORMAL (Repos)
// - Taille originale restaurée avec transition fluide
// - Background transparent pour montrer l'image PNG
// - Texte blanc pour contraste optimal
```

### Callbacks avec feedback visuel

```c
// Exemple de callback avec feedback automatique
static void play_button_clicked(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // 🎯 FEEDBACK VISUEL AUTOMATIQUE
    atomic_set_background_color(atomic_element, 100, 200, 100, 255); // Vert
    atomic_set_text_color_rgba(atomic_element, 0, 0, 0, 255);        // Noir
    
    // Effet d'enfoncement
    int width = atomic_get_width(atomic_element);
    int height = atomic_get_height(atomic_element);
    atomic_set_size(atomic_element, width - 4, height - 2);
    
    printf("🎮✨ Bouton Play avec feedback visuel !\n");
    
    // L'action utilisateur...
}
```

### Styles prédéfinis

```c
// Utiliser les styles prédéfinis pour différents types d'actions
BUTTON_SUCCESS(play_button);   // Vert - actions positives
BUTTON_DANGER(quit_button);    // Rouge - actions dangereuses  
BUTTON_INFO(help_button);      // Bleu - informations
BUTTON_WARNING(reset_button);  // Orange - avertissements

// Gestion manuelle des états
BUTTON_PRESSED(button);        // État pressé
BUTTON_RELEASED(button);       // État relâché
BUTTON_HOVER_ON(button);       // Survol activé
BUTTON_HOVER_OFF(button);      // Survol désactivé
BUTTON_RESET(button);          // Retour à l'état normal
```

### Logs de feedback visuel

Avec le système de logs activé, vous verrez :

```
[14:32:15] [UserCallback] [PlayButtonVisual] [home_scene.c] : play_button_clicked - VISUAL FEEDBACK APPLIED: green bg, black text, size reduced
[14:32:15] [UIComponent] [VisualState] [play-button] : Button pressed state applied
[14:32:16] [UserCallback] [ButtonHoverVisual] [home_scene.c] : button_hovered - VISUAL FEEDBACK: white overlay, size increased
[14:32:16] [UIComponent] [VisualState] [play-button] : Button hover state applied
[14:32:17] [UserCallback] [ButtonUnhoverVisual] [home_scene.c] : button_unhovered - VISUAL FEEDBACK RESET: normal appearance restored
[14:32:17] [UIComponent] [VisualState] [play-button] : Button normal state restored from hover
```

## 🚨 Diagnostic du problème d'événements - RÉSOLU ✅

### ✅ Problèmes identifiés et corrigés :

1. **🔧 Classification d'événements incorrecte** : Les mouvements de souris étaient classés comme des clics
2. **🔧 Éléments non enregistrés** : Les boutons n'étaient pas enregistrés dans l'EventManager
3. **🔧 Fonction inutilisée** : `count_elements` générait un warning
4. **🔧 Logs manquants** : Impossible de diagnostiquer les problèmes d'enregistrement

### ✅ Flux corrigé (MAINTENANT FONCTIONNEL !) :

```
🖱️ CLIC SOURIS → 🔧 Classification correcte (SDL_MOUSEBUTTONDOWN=1024) → 📡 Event Manager → ⚛️ Boutons enregistrés → 👤 Callbacks exécutés
    ↓                     ↓                                                 ↓                    ↓                        ↓
  Real Click         Correct typing                               Hit testing            Button found          play_button_clicked()
  (not motion)      (not motion=1026)                            (registered)         (callback exists)      quit_button_clicked()
```

### ✅ Tests de vérification ajoutés :

1. **🔍 Debug EventManager** : Compte et affiche tous les éléments enregistrés
2. **🔍 Debug hit testing** : Logs détaillés pour chaque test de collision  
3. **🔍 Debug connexion** : Vérification que les boutons existent avant enregistrement
4. **🔍 Debug callbacks** : Confirmation que les callbacks sont bien définis

### ✅ Logs maintenant attendus :

```
🔗 Connexion des événements pour la scène home...
✅ Boutons trouvés: play_button et quit_button
✅ Éléments atomiques trouvés
🔧 Enregistrement du bouton Play...
✅ Bouton Play enregistré
🔧 Enregistrement du bouton Quit...  
✅ Bouton Quit enregistré
🎯 Tous les événements connectés avec succès!
[DEBUG] Total registered elements: 2
[event.c] Hit testing against 2 registered elements at (150,200)
[event.c] Testing element #1: bounds(100,150,200,60) display=true
[event.c] Element hit - calling callback
```

Le système d'événements est maintenant complètement opérationnel ! 🎉

### 🎯 Changements apportés :

1. **✅ Classification correcte** : SDL_MOUSEMOTION (1026) vs SDL_MOUSEBUTTONDOWN (1024)
2. **✅ Debug complet** : Logs détaillés à chaque étape
3. **✅ Vérifications strictes** : NULL checks pour tous les pointeurs
4. **✅ Warning supprimé** : Fonction count_elements inutilisée enlevée

## Prochaines étapes

- Label component pour le texte
- Input component pour la saisie
- Panel component pour les conteneurs
- Layout automatique (flex, grid)
- Animations CSS-like
- Thèmes globaux
- Classes CSS-like avec sélecteurs

## 🆕 Architecture d'événements simplifiée

### ⚡ Boucle d'événements mono-thread classique

```
🔄 THREAD PRINCIPAL (Boucle de jeu - 60 FPS)
│
├── game_core_handle_events() → Traite directement SDL_PollEvent()
├── game_core_update() → Met à jour la logique
└── game_core_render() → Rendu graphique
```

### 🎯 Avantages de cette architecture

1. **🚀 Simplicité** : Code facile à comprendre et maintenir
2. **⚡ Stabilité** : Utilisation standard de SDL (mono-thread)
3. **🔒 Fiabilité** : Zero problèmes de thread-safety
4. **📊 Performance** : Latence minimale, 60 FPS constant
5. **🎮 Compatibilité** : Approche éprouvée par l'industrie

### 📦 Traitement direct des événements

```c
// Structure simplifiée - plus de buffer circulaire
void game_core_handle_events(GameCore* core) {
    SDL_Event event;
    
    // Simple boucle SDL standard
    while (SDL_PollEvent(&event)) {
        // Gestion directe des événements critiques
        if (event.type == SDL_QUIT) {
            core->running = false;
            return;
        }
        
        // Routage vers la scène active
        Scene* active_scene = scene_manager_get_active_scene(core->scene_manager);
        if (active_scene && active_scene->event_manager) {
            event_manager_handle_event(active_scene->event_manager, &event);
        }
    }
}
```

### 🔄 Flux simplifié et efficace

```
1. 🎯 SDL_PollEvent() (thread principal) :
   ├── Capture directe des événements SDL
   ├── Gestion immédiate des événements critiques
   └── Routage vers la scène active

2. 📦 Traitement immédiat :
   ├── Zero latence - pas de buffer
   ├── Zero problème de synchronisation
   └── Zero événements perdus

3. 🎮 Rendu synchrone (60 FPS) :
   ├── Événements → Mise à jour → Rendu
   ├── Séquence prévisible et stable
   └── Compatible avec tous les systèmes
```

### 📊 Logs générés (simplifiés)

```
[14:32:15] [CoreEvents] [ProcessedBatch] [core] : Processed 3 events in mono-thread
[14:32:15] [EventManager] [HitDetected] [event.c] : HIT! Element #1 at z-index 2 - calling callback
[14:32:15] [UserCallback] [PlayButton] [home_scene.c] : play_button_clicked callback executed
```

Cette architecture garantit une **simplicité maximale** tout en maintenant **toutes les fonctionnalités** ! 🎉

**🔧 Migration transparente :**
- ✅ **Même API** : Aucun changement dans le code utilisateur
- ✅ **Même performance** : 60 FPS garantis
- ✅ **Plus stable** : Élimination des problèmes de threading
- ✅ **Plus simple** : Code divisé par 10 en complexité

## 🎯 Animations de pièces du plateau (MAINTENANT IMPLÉMENTÉES)

Le système d'animation supporte maintenant les animations spécifiques aux pièces du jeu Fanorona avec **rendu complet du plateau** :

### ✅ Rendu du plateau

```c
// Le plateau trace automatiquement :
// - Les lignes du damier (horizontales, verticales, diagonales)
// - Les intersections (cercles indiquant les positions)
// - Les pions avec textures PNG (piece_black.png / piece_brun.png)

UINode* plateau = ui_plateau_container_with_players(tree, "plateau", player1, player2);
// Le rendu custom dessine tout automatiquement via plateau_custom_render()
```

### 🎨 Logique de tracé

**Conversion coordonnées logiques → écran :**
```c
plateau_logical_to_screen(data, row, col, &x, &y);
// Calcule la position pixel depuis la grille 5x9
```

**Dessin des lignes :**
```c
plateau_draw_line(data, r1, c1, r2, c2);
// Trace une ligne entre deux intersections
// Utilise LINE_THICKNESS pour épaisseur
```

**Dessin des intersections :**
```c
plateau_draw_intersection(data, r, c, is_strong);
// Cercle plus grand si strong (diagonales autorisées)
// Cercle plus petit si diamond (orthogonal uniquement)
```

**Dessin des pions :**
```c
plateau_draw_piece(data, r, c, owner);
// Texture noire/brune selon config_get_player_piece_colors()
// Fallback sur cercles colorés si textures manquantes
// 🆕 SYSTÈME DE SÉLECTION VISUELLE :
// - Cercle vert épais + croix blanche pour la sélection
// - Cercle doré fin pour le hover
// - Support sélection/déselection par clic
```

### 🎯 Système de sélection d'intersections

Le plateau supporte maintenant la sélection interactive des intersections avec **validation du tour selon le mode de jeu** :

**🎮 Règles d'interaction selon le mode :**

- **MODE MULTIJOUEUR LOCAL** : Les interactions alternent selon le tour
  - Tour du Joueur 1 : Seules les pièces du J1 peuvent être hover/sélectionnées
  - Tour du Joueur 2 : Seules les pièces du J2 peuvent être hover/sélectionnées
  - L'autre joueur ne peut pas interagir pendant ce temps

- **MODE VS IA** : Seul le joueur humain peut interagir
  - Tour du joueur humain : Peut hover/sélectionner ses pièces uniquement
  - Tour de l'IA : Aucune interaction possible (l'IA joue automatiquement)

- **MODE MULTIJOUEUR EN LIGNE** : Seul le joueur local peut interagir
  - Tour du joueur local : Peut hover/sélectionner ses pièces uniquement
  - Tour du joueur distant : Aucune interaction (en attente du coup réseau)

**🖱️ Interactions supportées :**
- **Clic sur intersection vide** : Sélectionne l'intersection (si autorisé)
- **Clic sur intersection avec pièce** : Sélectionne la pièce (si c'est le tour du propriétaire)
- **Clic sur intersection déjà sélectionnée** : Déselectionne (supprime les effets visuels)
- **Hover** : Effet doré temporaire (seulement si autorisé par le tour)

**🔒 Validation des interactions :**
```c
// Vérifier si un joueur peut interagir avec une pièce
bool can_interact = game_logic_can_player_interact(logic, piece_owner);

// Vérifier si c'est le tour d'un joueur local
bool is_turn = game_logic_is_local_player_turn(logic, player_number);

// Vérifier si on peut hover une pièce
bool can_hover = game_logic_can_hover_piece(logic, piece_owner);

// Vérifier si on peut sélectionner une pièce
bool can_select = game_logic_can_select_piece(logic, piece_owner);
```

**📊 Logs de validation :**
```