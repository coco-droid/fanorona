# Système UI DOM-like - Documentation

## 🎯 Vue d'ensemble

Le nouveau système UI de Fanorona offre une syntaxe **DOM-like** similaire à JavaScript, permettant de créer et manipuler des interfaces utilisateur de manière intuitive.

## 🌳 Architecture

```
UITree (Document)
├── UINode (Éléments DOM)
│   ├── AtomicElement (Rendu/Style)
│   ├── Component Data (Button, etc.)
│   └── Children (Hiérarchie)
└── Event Manager (Événements)
```

## 🚀 Syntaxe de base

### Création d'éléments

```c
// Créer l'arbre UI
UITree* ui_tree = ui_tree_create();
ui_set_global_tree(ui_tree); // Activer les fonctions $ et $$

// Créer des éléments
UINode* container = ui_div(ui_tree, "main-container");
UINode* button = ui_button(ui_tree, "save-btn", "Sauvegarder");
UINode* text = ui_text(ui_tree, "title", "Mon titre");
```

### Sélecteurs (comme jQuery/JavaScript)

```c
// Sélectionner par ID
UINode* element = $("#mon-id");

// Sélectionner par classe (à venir)
UINode* elements = $$(".ma-classe", &count);

// Navigation
UINode* parent = element->parent;
UINode* first_child = element->children[0];
```

### Style CSS-like avancé

```c
// Style direct
ui_set_position(element, 100, 50);
ui_set_size(element, 200, 40);
ui_set_z_index(element, 10);
ui_set_background(element, "rgb(70,130,180)");
ui_set_background_image(element, "assets/my_bg.png");

// Alignement et positionnement
ui_set_align(element, "center", "middle");
ui_center(element);        // Centre automatiquement
ui_center_x(element);      // Centre horizontalement
ui_center_y(element);      // Centre verticalement

// Flexbox complet
ui_set_display_flex(container);
ui_set_flex_direction(container, "row");         // row, column, row-reverse, column-reverse
ui_set_justify_content(container, "center");     // start, center, end, space-between, space-around, space-evenly
ui_set_align_items(container, "center");         // start, center, end, stretch
ui_set_flex_gap(container, 20);                 // Espacement entre éléments

// Texte et polices
ui_set_font(element, "assets/fonts/arial.ttf", 16);
ui_set_text_color(element, "rgb(255,255,255)");
ui_set_text_align(element, "center");           // left, center, right, justify
ui_set_text_style(element, true, false);        // bold, italic

// Style par propriétés
ui_node_set_style(element, "width", "200");
ui_node_set_style(element, "background-color", "rgb(255,0,0)");
ui_node_set_style(element, "display", "none");
```

### Événements

```c
// Ajouter des événements
ui_on_click(button, my_click_handler);
ui_on_hover(button, my_hover_handler);

// Ou avec addEventListener
ui_node_add_event_listener(button, "click", my_handler, user_data);
```

### Hiérarchie

```c
// Ajouter des enfants
ui_append(parent, child);
ui_append_to(child, parent); // Syntaxe inverse

// Manipulation
ui_tree_remove_child(parent, child);
ui_tree_insert_before(parent, new_child, reference);
```

## 📝 Exemple complet

```c
#include "src/ui/ui_components.h"

void create_interface(Scene* scene) {
    // Créer l'arbre UI
    UITree* ui_tree = ui_tree_create();
    ui_set_global_tree(ui_tree);
    
    // === CRÉATION DE L'INTERFACE ===
    
    // Container principal
    UINode* app = ui_div(ui_tree, "app");
    ui_set_position(app, 0, 0);
    ui_set_size(app, 800, 600);
    ui_set_background(app, "rgb(240,240,240)");
    
    // Header
    UINode* header = ui_div(ui_tree, "header");
    ui_set_position(header, 0, 0);
    ui_set_size(header, 800, 60);
    ui_set_background(header, "rgb(70,130,180)");
    
    UINode* title = ui_text(ui_tree, "title", "Mon Application");
    ui_set_position(title, 20, 15);
    ui_set_size(title, 300, 30);
    
    // Boutons
    UINode* save_btn = ui_button(ui_tree, "save", "Sauvegarder");
    ui_set_position(save_btn, 50, 100);
    ui_set_size(save_btn, 120, 40);
    ui_on_click(save_btn, on_save_click);
    
    UINode* load_btn = ui_button(ui_tree, "load", "Charger");
    ui_set_position(load_btn, 180, 100);
    ui_set_size(load_btn, 120, 40);
    ui_on_click(load_btn, on_load_click);
    
    // Status bar
    UINode* status = ui_div(ui_tree, "status");
    ui_set_position(status, 0, 560);
    ui_set_size(status, 800, 40);
    ui_set_background(status, "rgb(108,117,125)");
    
    // Construire la hiérarchie
    ui_append(ui_tree->root, app);
    ui_append(app, header);
    ui_append(header, title);
    ui_append(app, save_btn);
    ui_append(app, load_btn);
    ui_append(app, status);
    
    // Stocker dans la scène
    scene->ui_tree = ui_tree;
}

// Callbacks d'événements
void on_save_click(UINode* node, void* user_data) {
    printf("💾 Sauvegarde...\n");
    
    // Manipuler l'interface
    UINode* status = $("#status");
    ui_node_set_text(status, "Sauvegarde en cours...");
    ui_set_background(status, "rgb(40,167,69)");
    
    // Désactiver le bouton temporairement
    ui_node_set_style(node, "opacity", "0.5");
}

void on_load_click(UINode* node, void* user_data) {
    printf("📁 Chargement...\n");
    
    // Animation/effet
    UINode* status = $("#status");
    ui_node_set_text(status, "Chargement en cours...");
    ui_set_background(status, "rgb(255,193,7)");
}

// Dans la boucle de rendu de la scène
void scene_render(Scene* scene, GameWindow* window) {
    SDL_Renderer* renderer = window_get_renderer(window);
    
    // Fond
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    
    // Rendre l'arbre UI
    ui_tree_render(scene->ui_tree, renderer);
}

// Mise à jour
void scene_update(Scene* scene, float delta_time) {
    ui_tree_update(scene->ui_tree, delta_time);
}
```

## 🎨 Fonctionnalités avancées

### Macros pour syntaxe ultra-simple

```c
// Création rapide
UINode* btn = UI_BUTTON(tree, "save", "Sauvegarder");
UINode* box = UI_DIV(tree, "container");
UINode* txt = UI_TEXT(tree, "title", "Mon titre");

// Style rapide
SET_POS(btn, 100, 50);
SET_SIZE(btn, 120, 40);
SET_BG(btn, "rgb(70,130,180)");
SET_BG_IMG(btn, "assets/button_bg.png");
CENTER(btn);

// Flexbox rapide
FLEX_ROW(container);
FLEX_COLUMN(container);

// Événements rapides
ON_CLICK(btn, my_callback);

// Hiérarchie rapide
APPEND(parent, child);
APPEND_TO(child, parent);
```

### Système de styles complet

```c
// Exemple de style complexe
UIStyle my_style = {
    .x = 100, .y = 50,
    .width = 200, .height = 40,
    .z_index = 10,
    .background_color = "rgb(70,130,180)",
    .background_image = "assets/bg.png",
    .color = "rgb(255,255,255)",
    .align_horizontal = "center",
    .align_vertical = "middle",
    .flex_direction = "row",
    .justify_content = "space-between",
    .align_items = "center",
    .flex_gap = 20,
    .font_path = "assets/fonts/arial.ttf",
    .font_size = 16,
    .text_align = "center",
    .text_bold = true
};

ui_apply_style(element, &my_style);
```

### Flexbox complet

```c
// Container flexbox
UINode* container = ui_div(tree, "flex-container");
ui_set_display_flex(container);
ui_set_flex_direction(container, "row");        // row, column, row-reverse, column-reverse
ui_set_justify_content(container, "center");    // start, center, end, space-between, space-around, space-evenly  
ui_set_align_items(container, "center");        // start, center, end, stretch
ui_set_flex_gap(container, 20);

// Les enfants seront automatiquement positionnés
for (int i = 0; i < 4; i++) {
    UINode* item = ui_div(tree, NULL);
    ui_set_size(item, 80, 40);
    ui_append(container, item);
}
```

### Z-Index et superposition

```c
// Éléments avec différents z-index
UINode* background = ui_div(tree, "bg");
ui_set_z_index(background, 1);

UINode* content = ui_div(tree, "content");
ui_set_z_index(content, 10);

UINode* popup = ui_div(tree, "popup");
ui_set_z_index(popup, 100);

// Les éléments sont automatiquement triés par z-index lors du rendu
```

### Images de fond

```c
// Définir une image de fond
ui_set_background_image(element, "assets/button_bg.png");

// Ou directement dans le style
ui_node_set_style(element, "background-image", "assets/texture.png");

// L'image sera automatiquement chargée et redimensionnée
```

### Centrage intelligent

```c
// Centrage automatique
ui_center(element);          // Centre X et Y automatiquement
ui_center_x(element);        // Centre X seulement
ui_center_y(element);        // Centre Y seulement

// Alignement manuel
ui_set_align(element, "center", "middle");
ui_set_align(element, "right", "bottom");
ui_set_align(element, "left", "top");

// Le centrage est recalculé automatiquement lors des mises à jour
```

### Polices et texte avancé

```c
// Configurer la police
ui_set_font(element, "assets/fonts/roboto.ttf", 18);
ui_set_text_color(element, "rgb(33,37,41)");
ui_set_text_align(element, "center");       // left, center, right, justify
ui_set_text_style(element, true, false);    // bold, italic

// Le texte sera rendu avec la police spécifiée
```

### Classes CSS-like

```c
// Ajouter des classes
ui_add_class(element, "button");
ui_add_class(element, "primary");

// Vérifier les classes
if (ui_node_has_class(element, "active")) {
    // Faire quelque chose
}
```

### Recherche avancée

```c
// Sélecteurs CSS-like (à venir)
UINode* buttons = $$("button.primary", &count);
UINode* first_div = $("div");
UINode* nested = $("#container .button");
```

## 🔄 Comparaison avec JavaScript

| JavaScript DOM | Fanorona UI |
|----------------|-------------|
| `document.getElementById()` | `$("#id")` |
| `element.style.width = "100px"` | `ui_set_size(element, 100, height)` |
| `element.addEventListener()` | `ui_on_click(element, callback)` |
| `parent.appendChild(child)` | `ui_append(parent, child)` |
| `element.innerHTML = "text"` | `ui_node_set_text(element, "text")` |

## 📋 Exemples de scènes complètes

### Scène DOM de base
Voir `src/scene/dom_demo_scene.c` pour un exemple d'utilisation avec :
- ✅ Création d'interface complexe
- ✅ Gestion d'événements
- ✅ Manipulation DOM en temps réel
- ✅ Sélecteurs $ et $$
- ✅ Styles CSS-like

### Scène UI Avancée
Voir `src/scene/advanced_ui_scene.c` pour une démonstration complète avec :
- ✅ **Flexbox complet** avec gap, justify-content, align-items
- ✅ **Z-index dynamique** et superposition d'éléments
- ✅ **Centrage automatique** et alignement intelligent
- ✅ **Images de fond** et textures
- ✅ **Polices personnalisées** et styles de texte
- ✅ **Macros simplifiées** pour création rapide

Pour tester les scènes :
```c
// Scène DOM de base
Scene* demo = create_dom_demo_scene();
scene_manager_set_scene(scene_manager, demo);

// Scène UI avancée
Scene* advanced = create_advanced_ui_scene();
scene_manager_set_scene(scene_manager, advanced);
```

## 🆕 Nouvelles fonctionnalités ajoutées

### ✨ Z-Index
```c
ui_set_z_index(element, 10); // Plus le nombre est grand, plus l'élément est au premier plan
```

### 🖼️ Images de fond
```c
ui_set_background_image(element, "assets/my_texture.png");
// L'image est automatiquement chargée et adaptée à la taille de l'élément
```

### 🎯 Positionnement intelligent
```c
// Centrage automatique
ui_center(element);           // Centre complet
ui_center_x(element);         // Centre horizontal seulement
ui_center_y(element);         // Centre vertical seulement

// Alignement manuel
ui_set_align(element, "center", "middle");  // horizontal, vertical
ui_set_align(element, "right", "bottom");
```

### 📐 Flexbox CSS complet
```c
UINode* container = ui_div(tree, "flex-container");
ui_set_display_flex(container);
ui_set_flex_direction(container, "row");         // row, column, row-reverse, column-reverse
ui_set_justify_content(container, "center");     // start, center, end, space-between, space-around, space-evenly
ui_set_align_items(container, "center");         // start, center, end, stretch
ui_set_flex_gap(container, 20);                 // Espacement entre éléments

// Les enfants sont automatiquement positionnés !
```

### 🔤 Polices et texte avancé
```c
ui_set_font(element, "assets/fonts/roboto.ttf", 18);    // Police et taille
ui_set_text_color(element, "rgb(33,37,41)");           // Couleur du texte
ui_set_text_align(element, "center");                  // left, center, right, justify
ui_set_text_style(element, true, false);               // bold, italic
```

### 🏷️ Macros simplifiées
```c
// Création ultra-rapide
UINode* btn = UI_BUTTON(tree, "save", "Sauvegarder");
UINode* box = UI_DIV(tree, "container");

// Style ultra-rapide
SET_POS(btn, 100, 50);
SET_SIZE(btn, 120, 40);
SET_BG(btn, "rgb(70,130,180)");
SET_BG_IMG(btn, "assets/button.png");
CENTER(btn);

// Flexbox ultra-rapide
FLEX_ROW(container);
FLEX_COLUMN(container);

// Hiérarchie ultra-rapide
APPEND(parent, child);
```

Le système offre maintenant une syntaxe aussi simple et intuitive que le DOM JavaScript avec toutes les fonctionnalités CSS modernes ! 🎉