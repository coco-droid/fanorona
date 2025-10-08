# Système UI DOM-like - Documentation

## 🎯 Vue d'ensemble

Le nouveau système UI de Fanorona offre une syntaxe **DOM-like** similaire à JavaScript, permettant de créer et manipuler des interfaces utilisateur de manière intuitive.

**🆕 Nouvelles fonctionnalités :**
- ✅ **Logs de traçage des événements** pour debugging avancé
- ✅ **Z-index implicites** calculés automatiquement
- ✅ **Support complet des images PNG** pour les backgrounds
- ✅ **Correction de l'affichage du texte** sur tous les composants

## 📦 Architecture Container Modulaire - NOUVEAU ! 

### 🎯 Containers avec sections spécialisées

La nouvelle architecture divise les containers en **3 sections spécialisées** :

```c
// Container principal avec sections automatiques
UINode* modal = UI_CONTAINER_CENTERED(tree, "game-menu", 500, 400);

// Architecture créée automatiquement :
// ┌─── Container Principal (flexbox column) ───┐
// │  ┌─── Section Logo (400x100) ───┐         │
// │  │     Logo centré               │         │
// │  └─────────────────────────────┘         │
// │  ┌─── Section Sous-titre (400x50) ───┐   │
// │  │     "Stratégie et Tradition"      │   │
// │  └─────────────────────────────────┘     │
// │  ┌─── Section Contenu (450x250) ───┐     │
// │  │     Vos boutons/contenu         │     │
// │  │     (centré automatiquement)    │     │
// │  └─────────────────────────────────┘     │
// └─────────────────────────────────────────┘
```

### ✨ Avantages de cette architecture

1. **🎯 Centrage automatique** : Chaque section centre son contenu
2. **📏 Tailles dédiées** : Chaque section a sa taille optimisée
3. **🔧 Sans margin/padding** : Plus besoin de calculs complexes
4. **🎨 Flexbox natif** : Utilisation pure de CSS Flexbox
5. **📦 Modulaire** : Chaque section est indépendante

### 🚀 Utilisation simplifiée

```c
// 1. Créer le container (logo et sous-titre automatiques)
UINode* dialog = UI_CONTAINER_CENTERED(tree, "my-dialog", 500, 400);

// 2. Ajouter votre contenu (sera centré automatiquement)
UINode* button_group = UI_DIV(tree, "buttons");
ui_set_display_flex(button_group);
FLEX_COLUMN(button_group);
ui_set_flex_gap(button_group, 15);

// Ajouter des boutons au groupe
UINode* play = ui_button(tree, "play", "JOUER", on_play, NULL);
UINode* quit = ui_button(tree, "quit", "QUITTER", on_quit, NULL);
APPEND(button_group, play);
APPEND(button_group, quit);

// 3. Le contenu va dans la section dédiée (centré automatiquement)
ui_container_add_content(dialog, button_group);

// Résultat : Interface parfaitement centrée sans calculs manuels !
```

### 🔄 Migration depuis l'ancien système

```c
// ANCIEN (calculs manuels) :
ui_set_position(logo, calculated_x, 23);
ui_set_position(subtitle, calculated_x, 111);
atomic_set_margin(subtitle, 0, 0, 20, 0);
// ... calculs complexes de positions ...

// NOUVEAU (automatique) :
UINode* container = UI_CONTAINER_CENTERED(tree, "dialog", 500, 400);
ui_container_add_content(container, my_buttons);
// Logo et sous-titre positionnés automatiquement !
// Boutons centrés automatiquement dans leur section !
```

## 🔍 Système de logs pour debugging

### Activation et utilisation

```c
// Activer les logs d'événements (maintenant plus discrets)
ui_set_event_logging(true);

// Les logs critiques apparaissent toujours :
// [EVENT] [UIComponent] [Click] [save-btn] : Button click callback triggered
// [EVENT] [Button] [StateChange] [save-btn] : State NORMAL -> PRESSED

// 🔧 AMÉLIORATION : Logs verbeux supprimés pour une console plus propre
// ❌ Plus de logs de mouvement souris en continu
// ❌ Plus de logs de hit testing détaillés  
// ❌ Plus de logs de rendu périodiques
```

### Logs personnalisés

```c
// Ajouter vos propres logs (toujours disponible)
ui_log_event("MyComponent", "CustomEvent", "my-element", "Mon message personnalisé");
```

**📊 Console allégée** :
- ✅ Événements utilisateur importants (clics boutons)
- ✅ Changements d'état visuels
- ✅ Erreurs et avertissements
- ❌ Mouvements souris répétitifs supprimés
- ❌ Logs de synchronisation technique supprimés

## 🎯 Z-Index automatique

### Gestion intelligente

```c
// Les z-index sont calculés automatiquement dans l'ordre d'ajout
UINode* background = ui_div(tree, "bg");      // z-index automatique: 1
UINode* content = ui_div(tree, "content");    // z-index automatique: 2
UINode* overlay = ui_div(tree, "overlay");    // z-index automatique: 3

// Calculer après construction de l'interface
ui_calculate_implicit_z_index(tree);

// Z-index explicite prioritaire
ui_set_z_index(overlay, 100); // Sera au-dessus de tous les autres
```

### Inspection des z-index

```c
// Vérifier le z-index effectif (explicite ou calculé)
int effective_z = ui_node_get_effective_z_index(element);
printf("Z-index de %s : %d\n", element->id, effective_z);
```

## 🎨 Images PNG avancées

### Background images optimisées

```c
// Utiliser des images PNG comme fond
ui_set_background_image(element, "assets/my_background.png");

// Pour les boutons spécifiquement
ui_button_set_background_image(button, "assets/button_bg.png");

// L'image sera automatiquement :
// - Chargée en mémoire
// - Redimensionnée à la taille de l'élément
// - Rendue avec transparence préservée
```

### Correction des artefacts de texte

```c
// Corriger les problèmes d'affichage de texte sur PNG
ui_button_fix_text_rendering(button);

// Cette fonction résout :
// - Les carrés gris autour du texte
// - Le positionnement incorrect
// - Les problèmes de contraste
```

## 🎨 Feedback visuel interactif

### Boutons avec états visuels

```c
// Créer des boutons avec feedback automatique
UINode* confirm_btn = ui_button(tree, "confirm", "CONFIRMER", on_confirm, NULL);
ui_button_set_background_image(confirm_btn, "btn_confirm.png");

// États visuels automatiques :
// HOVER: Agrandissement (+2px) + overlay blanc translucide
// CLICK: Réduction (-4px) + fond vert + texte noir
// NORMAL: Taille normale + fond transparent + texte blanc

// Connexion des événements avec feedback
atomic_set_click_handler(confirm_btn->element, confirm_clicked);
atomic_set_hover_handler(confirm_btn->element, button_hovered);
atomic_set_unhover_handler(confirm_btn->element, button_unhovered);
```

### Styles prédéfinis pour feedback

```c
// Appliquer des styles contextuels instantanément
BUTTON_SUCCESS(play_btn);    // Vert pour actions positives
BUTTON_DANGER(delete_btn);   // Rouge pour actions dangereuses
BUTTON_INFO(help_btn);       // Bleu pour informations
BUTTON_WARNING(reset_btn);   // Orange pour avertissements

// Contrôle manuel des états
BUTTON_PRESSED(btn);         // Simuler un clic
BUTTON_HOVER_ON(btn);        // Simuler un survol
BUTTON_RESET(btn);           // Retour à l'état normal
```

### Logs de feedback en temps réel

```c
// Avec ui_set_event_logging(true), observez le feedback :

// Survol d'un bouton :
// [EVENT] [UIComponent] [VisualState] [confirm] : Button hover state applied
// [EVENT] [UserCallback] [ButtonHoverVisual] [home_scene.c] : button_hovered - VISUAL FEEDBACK: white overlay, size increased

// Clic sur un bouton :
// [EVENT] [UIComponent] [VisualState] [confirm] : Button pressed state applied  
// [EVENT] [UserCallback] [ConfirmButtonVisual] [home_scene.c] : confirm_button_clicked - VISUAL FEEDBACK APPLIED: green bg, black text, size reduced
// [EVENT] [UIComponent] [VisualStyle] [confirm] : Success style applied (green)

// Restauration automatique :
// [EVENT] [UIComponent] [VisualState] [confirm] : Button normal state restored from hover
```

## 💡 Exemple d'interface moderne

```c
#include "src/ui/ui_components.h"

void create_game_menu() {
    // === SETUP INITIAL ===
    ui_set_event_logging(true); // Debugging activé
    
    UITree* tree = ui_tree_create();
    ui_set_global_tree(tree);
    
    // === INTERFACE PRINCIPALE ===
    
    // Fond principal avec image
    UINode* app = UI_DIV(tree, "game-menu");
    SET_POS(app, 0, 0);
    SET_SIZE(app, 800, 600);
    ui_set_background_image(app, "menu_background.png");
    
    // Logo du jeu
    UINode* logo = UI_IMAGE(tree, "logo", logo_texture);
    SET_SIZE(logo, 400, 150);
    CENTER_X(logo);
    SET_POS(logo, logo->x, 50);
    
    // Container pour les boutons avec flexbox
    UINode* menu_buttons = UI_DIV(tree, "menu-buttons");
    SET_SIZE(menu_buttons, 300, 400);
    CENTER_X(menu_buttons);
    SET_POS(menu_buttons, menu_buttons->x, 220);
    ui_set_display_flex(menu_buttons);
    FLEX_COLUMN(menu_buttons);
    ui_set_justify_content(menu_buttons, "center");
    ui_set_align_items(menu_buttons, "center");
    ui_set_flex_gap(menu_buttons, 20);
    
    // Boutons avec images PNG personnalisées
    UINode* play_btn = ui_button(tree, "play", "NOUVELLE PARTIE", on_new_game, NULL);
    SET_SIZE(play_btn, 280, 60);
    ui_button_set_background_image(play_btn, "btn_play.png");
    ui_set_text_color(play_btn, "rgb(255,255,255)");
    ui_button_fix_text_rendering(play_btn);
    
    UINode* load_btn = ui_button(tree, "load", "CHARGER PARTIE", on_load_game, NULL);
    SET_SIZE(load_btn, 280, 60);
    ui_button_set_background_image(load_btn, "btn_load.png");
    ui_set_text_color(load_btn, "rgb(255,255,255)");
    ui_button_fix_text_rendering(load_btn);
    
    UINode* options_btn = ui_button(tree, "options", "OPTIONS", on_options, NULL);
    SET_SIZE(options_btn, 280, 60);
    ui_button_set_background_image(options_btn, "btn_options.png");
    ui_set_text_color(options_btn, "rgb(255,255,255)");
    ui_button_fix_text_rendering(options_btn);
    
    UINode* quit_btn = ui_button(tree, "quit", "QUITTER", on_quit, NULL);
    SET_SIZE(quit_btn, 280, 60);
    ui_button_set_background_image(quit_btn, "btn_quit.png");
    ui_set_text_color(quit_btn, "rgb(255,255,255)");
    ui_button_fix_text_rendering(quit_btn);
    
    // Popup de confirmation (z-index élevé)
    UINode* confirm_popup = UI_DIV(tree, "confirm-popup");
    SET_SIZE(confirm_popup, 400, 200);
    CENTER(confirm_popup);
    ui_set_z_index(confirm_popup, 1000); // Au-dessus de tout
    ui_set_background_image(confirm_popup, "popup_bg.png");
    ui_node_set_style(confirm_popup, "display", "none"); // Caché par défaut
    
    // === CONSTRUCTION DE LA HIÉRARCHIE ===
    
    APPEND(tree->root, app);
    APPEND(app, logo);
    APPEND(app, menu_buttons);
    APPEND(menu_buttons, play_btn);
    APPEND(menu_buttons, load_btn);
    APPEND(menu_buttons, options_btn);
    APPEND(menu_buttons, quit_btn);
    APPEND(app, confirm_popup);
    
    // Calculer les z-index automatiques
    ui_calculate_implicit_z_index(tree);
    
    printf("✅ Menu de jeu créé avec images PNG et z-index automatiques\n");
    
    // Les logs montreront :
    // [EVENT] [UIComponent] [ZIndexCalculation] [game-menu] : Implicit z-index set to 1
    // [EVENT] [UIComponent] [ZIndexCalculation] [logo] : Implicit z-index set to 2
    // [EVENT] [UIComponent] [ZIndexCalculation] [menu-buttons] : Implicit z-index set to 3
    // [EVENT] [UIComponent] [ZIndexCalculation] [play] : Implicit z-index set to 4
    // [EVENT] [UIComponent] [ZIndexCalculation] [load] : Implicit z-index set to 5
    // [EVENT] [UIComponent] [ZIndexCalculation] [options] : Implicit z-index set to 6
    // [EVENT] [UIComponent] [ZIndexCalculation] [quit] : Implicit z-index set to 7
    // [EVENT] [UIComponent] [ZIndexCalculation] [confirm-popup] : Explicit z-index 1000 kept
}

// Callbacks avec logs automatiques
void on_new_game(UINode* node, void* user_data) {
    // Les logs apparaîtront automatiquement :
    // [EVENT] [EventManager] [MouseClick] [play] : Event received at (400, 280)
    // [EVENT] [AtomicElement] [MouseClick] [play] : Hit test passed
    // [EVENT] [UIComponent] [Click] [play] : Button click callback triggered
    
    printf("🎮 Nouvelle partie démarrée\n");
    
    // Afficher popup de confirmation
    UINode* popup = $("#confirm-popup");
    ui_node_set_style(popup, "display", "block");
}

void on_quit(UINode* node, void* user_data) {
    // [EVENT] [Button] [StateChange] [quit] : State NORMAL -> PRESSED
    printf("🚪 Fermeture du jeu\n");
    exit(0);
}
```

## 🔍 Debugging avancé

### Traçage complet des événements

```c
// Avec ui_set_event_logging(true), vous verrez :

// Survol d'un bouton :
// [EVENT] [EventManager] [MouseMove] [menu-buttons] : Mouse at (400, 280)
// [EVENT] [AtomicElement] [MouseEnter] [play] : Element entered, bounds (260,250,280,60)
// [EVENT] [Button] [StateChange] [play] : State NORMAL -> HOVERED
// [EVENT] [UIComponent] [Hover] [play] : Hover callback triggered

// Clic sur un bouton :
// [EVENT] [EventManager] [MouseDown] [play] : Button pressed at (400, 280)
// [EVENT] [AtomicElement] [MouseDown] [play] : Hit test passed
// [EVENT] [Button] [StateChange] [play] : State HOVERED -> PRESSED
// [EVENT] [EventManager] [MouseUp] [play] : Button released at (400, 280)
// [EVENT] [UIComponent] [Click] [play] : Button click callback triggered
// [EVENT] [Button] [StateChange] [play] : State PRESSED -> HOVERED
```

### Inspection des z-index en temps réel

```c
// Fonction de debug pour visualiser les z-index
void debug_interface_z_index(UITree* tree) {
    printf("=== Z-INDEX DEBUG ===\n");
    
    // Parcourir tous les éléments
    ui_tree_traverse(tree->root, print_z_index, NULL);
}

void print_z_index(UINode* node, void* user_data) {
    int z = ui_node_get_effective_z_index(node);
    printf("Element '%s' : z-index = %d %s\n", 
           node->id, 
           z,
           ui_node_has_explicit_z_index(node) ? "(explicite)" : "(automatique)");
}
```

## 🆕 Nouvelles fonctionnalités complètes

### ✨ Logs intelligents
- **Traçage complet** de la propagation des événements
- **Sources identifiées** (EventManager, AtomicElement, Button, etc.)
- **Messages descriptifs** pour debugging rapide
- **Activation/désactivation** à la volée

### 🎯 Z-Index automatique
- **Calcul intelligent** basé sur l'ordre d'ajout
- **Éléments récents** au premier plan automatiquement
- **Z-index explicites** prioritaires sur automatiques
- **Inspection en temps réel** des valeurs

### 🖼️ Images PNG optimisées
- **Support natif** des backgrounds PNG avec transparence
- **Chargement automatique** et cache des textures
- **Redimensionnement intelligent** selon l'élément
- **Correction des artefacts** de rendu de texte

### 🔧 Debugging avancé
- **Inspection complète** de l'état des éléments
- **Traçage des changements** d'état en temps réel
- **Messages colorés** et structurés
- **Navigation** dans la hiérarchie UI

### 🎉 Feedback visuel interactif
- **États visuels automatiques** pour les boutons (hover, click, normal)
- **Styles prédéfinis** pour actions contextuelles (succès, danger, info, avertissement)
- **Logs de feedback** en temps réel pour chaque interaction

Cette version du système DOM-like offre maintenant tous les outils nécessaires pour créer et déboguer des interfaces modernes et robustes ! 🎉