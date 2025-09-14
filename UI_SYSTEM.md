# Système UI Atomique - Documentation

## Vue d'ensemble

Le système UI de Fanorona est basé sur une architecture atomique où tous les composants héritent d'un élément de base `AtomicElement` qui fournit des propriétés CSS-like et une gestion d'événements intégrée.

**🆕 Nouvelles fonctionnalités :**
- ✅ **Logs de traçage des événements** pour debugging
- ✅ **Z-index implicites** basés sur l'ordre d'ajout
- ✅ **Support des images PNG** pour les backgrounds de boutons
- ✅ **Correction de l'affichage du texte** sur les boutons

## Architecture

```
AtomicElement (base)
├── Propriétés CSS-like (position, taille, margin, padding, etc.)
├── Gestion d'événements intégrée avec logs
├── Rendu personnalisable avec z-index automatique
└── Hiérarchie parent-enfant

Components (héritent d'AtomicElement)
├── Button (avec support PNG + logs)
├── Label (à venir)
├── Input (à venir)
└── Panel (à venir)
```

## 🔍 Système de logs d'événements

### Activation des logs

```c
// Activer les logs pour debugging
ui_set_event_logging(true);

// Les logs apparaîtront dans la console avec le format :
// [EVENT] [SOURCE] [TYPE] [ELEMENT_ID] : MESSAGE
```

### Traçage de la propagation

```c
// Exemple de logs générés automatiquement :
// [EVENT] [EventManager] [MouseClick] [play-button] : Event received at (150, 320)
// [EVENT] [AtomicElement] [MouseClick] [play-button] : Hit test passed
// [EVENT] [UIComponent] [Click] [play-button] : Button click callback triggered
// [EVENT] [Button] [StateChange] [play-button] : State changed to PRESSED
```

## 🎯 Z-Index automatique

### Gestion implicite

```c
// Les z-index sont calculés automatiquement dans l'ordre d'ajout :
UINode* element1 = ui_div(tree, "first");   // z-index: 1
UINode* element2 = ui_div(tree, "second");  // z-index: 2
UINode* element3 = ui_div(tree, "third");   // z-index: 3

// Calculer après construction de l'arbre
ui_calculate_implicit_z_index(tree);

// Z-index explicite prioritaire sur implicite
ui_set_z_index(element2, 100); // Sera rendu au-dessus des autres
```

### Obtenir le z-index effectif

```c
// Obtenir le z-index (explicite ou calculé automatiquement)
int z = ui_node_get_effective_z_index(element);
```

## 🎨 Boutons améliorés

### Images PNG de fond

```c
// Créer un bouton avec image PNG (sans couleur de fond)
UINode* btn = ui_button(tree, "my-btn", "Mon Bouton", callback, NULL);
ui_button_set_background_image(btn, "button_bg.png");

// Le PNG sera chargé automatiquement et redimensionné
// Idéal pour des boutons avec des designs personnalisés
// ✅ Correction des bugs de compilation - membres ajoutés dans AtomicStyle
```

### Correction de l'affichage du texte

```c
// Corriger les problèmes de rendu du texte
ui_button_fix_text_rendering(btn);

// Cette fonction :
// - Supprime les artefacts de rendu (carrés gris)
// - Optimise la position du texte avec text_x, text_y
// - Assure la lisibilité sur les backgrounds PNG
// ✅ Position du texte maintenant correctement gérée
```

### Configuration complète

```c
// Exemple de bouton optimisé (sans warnings de compilation)
UINode* save_btn = ui_button(tree, "save", "SAUVEGARDER", on_save, NULL);
ui_set_size(save_btn, 200, 60);
ui_button_set_background_image(save_btn, "home_bg_btn.png");
ui_set_text_color(save_btn, "rgb(255, 255, 255)");
ui_button_fix_text_rendering(save_btn);

// ✅ Toutes les énumérations ALIGN_START/ALIGN_END sont maintenant supportées
```

## 📊 Debugging et optimisation

### Logs d'événements détaillés

```c
// Activer dans votre scène
ui_set_event_logging(true);

// Observer la propagation :
// [EVENT] [EventManager] [MouseMove] [button-container] : Mouse at (150, 320)
// [EVENT] [AtomicElement] [MouseEnter] [play-button] : Element entered
// [EVENT] [Button] [StateChange] [play-button] : State NORMAL -> HOVERED
// [EVENT] [UIComponent] [Hover] [play-button] : Hover callback triggered
```

### Inspection des z-index

```c
// Vérifier les z-index calculés
void debug_z_index(UINode* node) {
    int z = ui_node_get_effective_z_index(node);
    printf("Element %s has z-index: %d\n", node->id, z);
}
```

## Exemple complet avec nouvelles fonctionnalités

```c
#include "src/ui/ui_components.h"

void create_modern_interface() {
    // Activer les logs pour debugging
    ui_set_event_logging(true);
    
    UITree* tree = ui_tree_create();
    ui_set_global_tree(tree);
    
    // Container principal
    UINode* app = UI_DIV(tree, "app");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 800, 600);
    ui_set_background_image(app, "background.png");
    
    // Logo (sera z-index: 1)
    UINode* logo = UI_IMAGE(tree, "logo", logo_texture);
    SET_SIZE(logo, 300, 100);
    CENTER_X(logo);
    SET_POS(logo, logo->x, 50);
    
    // Boutons avec PNG backgrounds (seront z-index: 2, 3)
    UINode* play_btn = ui_button(tree, "play", "JOUER", on_play, NULL);
    SET_SIZE(play_btn, 200, 60);
    SET_POS(play_btn, 300, 200);
    ui_button_set_background_image(play_btn, "button_play.png");
    ui_set_text_color(play_btn, "rgb(255,255,255)");
    ui_button_fix_text_rendering(play_btn);
    
    UINode* quit_btn = ui_button(tree, "quit", "QUITTER", on_quit, NULL);
    SET_SIZE(quit_btn, 200, 60);
    SET_POS(quit_btn, 300, 280);
    ui_button_set_background_image(quit_btn, "button_quit.png");
    ui_set_text_color(quit_btn, "rgb(255,255,255)");
    ui_button_fix_text_rendering(quit_btn);
    
    // Popup (z-index explicite prioritaire)
    UINode* popup = UI_DIV(tree, "popup");
    SET_SIZE(popup, 400, 200);
    CENTER(popup);
    ui_set_z_index(popup, 100); // Sera au-dessus de tout
    SET_BG(popup, "rgba(0,0,0,0.8)");
    
    // Construire la hiérarchie
    APPEND(tree->root, app);
    APPEND(app, logo);
    APPEND(app, play_btn);
    APPEND(app, quit_btn);
    APPEND(app, popup);
    
    // Calculer les z-index automatiques
    ui_calculate_implicit_z_index(tree);
    
    printf("✅ Interface créée avec z-index automatiques et logs activés\n");
}

// Les logs montreront :
// [EVENT] [UIComponent] [ZIndexCalculation] [app] : Implicit z-index set to 1
// [EVENT] [UIComponent] [ZIndexCalculation] [logo] : Implicit z-index set to 2  
// [EVENT] [UIComponent] [ZIndexCalculation] [play] : Implicit z-index set to 3
// [EVENT] [UIComponent] [ZIndexCalculation] [quit] : Implicit z-index set to 4
// [EVENT] [UIComponent] [ZIndexCalculation] [popup] : Explicit z-index 100 kept
```

## 🆕 Fonctionnalités ajoutées

### ✅ Logs de traçage complets
- Propagation des événements étape par étape
- Sources identifiées (EventManager, AtomicElement, Button, etc.)
- Messages descriptifs pour debugging

### ✅ Z-Index automatique
- Calcul basé sur l'ordre d'ajout dans l'arbre
- Les éléments ajoutés en dernier sont au premier plan
- Z-index explicites prioritaires

### ✅ Boutons PNG optimisés
- Support natif des images de fond PNG
- Correction des artefacts de texte (carrés gris)
- Positionnement optimisé du texte

### ✅ Debugging avancé
- Activation/désactivation des logs
- Inspection des z-index effectifs
- Traçage des changements d'état

Ces améliorations rendent le système UI plus robuste, plus facile à déboguer et plus flexible pour créer des interfaces modernes ! 🎉

## 🆕 Corrections de bugs apportées

### ✅ Structure AtomicStyle étendue
- Ajout des membres `text_x`, `text_y` pour la position du texte
- Ajout du membre `font` pour la police directe
- Ajout du membre `font_size` pour la taille de police directe

### ✅ Énumérations corrigées
- Ajout de `ALIGN_START` et `ALIGN_END` dans `AlignType`
- Mapping correct des valeurs "start" → ALIGN_TOP et "end" → ALIGN_BOTTOM

### ✅ Warnings supprimés
- Paramètres non utilisés dans `ui_button()` correctement gérés
- Signature de `ui_tree_create_node()` corrigée

### ✅ Compatibilité maintenue
- Les nouvelles propriétés sont synchronisées avec les anciennes
- Pas de rupture de l'API existante

Ces améliorations rendent le système UI plus robuste et sans erreurs de compilation ! 🎉

## Prochaines étapes

- Label component pour le texte
- Input component pour la saisie
- Panel component pour les conteneurs
- Layout automatique (flex, grid)
- Animations CSS-like
- Thèmes globaux
- Classes CSS-like avec sélecteurs