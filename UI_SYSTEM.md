# Système UI Atomique - Documentation

## Vue d'ensemble

Le système UI de Fanorona est basé sur une architecture atomique où tous les composants héritent d'un élément de base `AtomicElement` qui fournit des propriétés CSS-like et une gestion d'événements intégrée.

**🆕 Nouvelles fonctionnalités :**
- ✅ **Logs de traçage des événements** pour debugging
- ✅ **Z-index implicites** basés sur l'ordre d'ajout
- ✅ **Support des images PNG** pour les backgrounds de boutons
- ✅ **Correction de l'affichage du texte** sur les boutons
- 🆕 **Feedback visuel interactif** pour les clics et survols
- 🆕 **Composant Container** avec style modal
- 🔧 **API atomic simplifiée** avec fonctions unifiées

**🎯 Système de feedback visuel :**
- 🎨 **États visuels automatiques** : hover, pressed, normal
- 🌈 **Styles prédéfinis** : success, danger, info, warning
- ⚡ **Animations de clic** avec effets de taille et couleur
- 📊 **Logs détaillés** de tous les changements visuels

## 📦 Composant Container

### Style modal automatique avec API corrigée

```c
// Créer un container avec style modal (overlay noir, bordure orange)
UINode* modal = UI_CONTAINER(tree, "my-modal");
SET_SIZE(modal, 400, 300);
CENTER(modal);

// Le container utilise maintenant l'API atomic correcte :
// - atomic_set_border(element, width, r, g, b, a) pour bordures
// - Fond noir transparent (alpha: 180)
// - Bordure orange unifiée
// - Padding interne de 1px
// - Layout flexbox vertical centré
// - Gap de 20px entre les éléments
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
// - Taille légèrement augmentée (+2px largeur, +1px hauteur)
// - Overlay blanc translucide (alpha: 30)
// - Animation fluide

// 🎯 CLICK (Clic)
// - Taille réduite (-4px largeur, -2px hauteur) 
// - Couleur de fond contextuelle (vert pour Play, rouge pour Quit)
// - Texte contrasté automatiquement

// 🔄 NORMAL (Repos)
// - Apparence par défaut restaurée
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

## 🎯 Événements visuels connectés

Les boutons supportent maintenant 3 types d'événements avec feedback automatique :

```c
// Connexion complète des événements visuels
atomic_set_click_handler(button->element, button_clicked);      // Clic avec compression
atomic_set_hover_handler(button->element, button_hovered);      // Survol avec agrandissement
atomic_set_unhover_handler(button->element, button_unhovered);  // Sortie avec restauration
```

## ⚡ API de feedback visuel

### Fonctions principales

```c
// États de base
void ui_button_set_pressed_state(UINode* button, bool pressed);
void ui_button_set_hover_state(UINode* button, bool hovered);
void ui_button_reset_visual_state(UINode* button);

// Styles contextuels
void ui_button_apply_success_style(UINode* button);   // Vert
void ui_button_apply_danger_style(UINode* button);    // Rouge
void ui_button_apply_info_style(UINode* button);      // Bleu
void ui_button_apply_warning_style(UINode* button);   // Orange

// Animations (base pour extensions futures)
void ui_button_animate_click(UINode* button, int duration_ms);
void ui_button_animate_hover(UINode* button, bool entering);
```

### Exemple d'utilisation complète

```c
// Créer un bouton avec feedback visuel complet
UINode* save_btn = ui_button(tree, "save", "SAUVEGARDER", on_save_clicked, NULL);
SET_SIZE(save_btn, 200, 60);
ui_button_set_background_image(save_btn, "save_button.png");

// Connecter tous les événements visuels
atomic_set_click_handler(save_btn->element, save_button_clicked);
atomic_set_hover_handler(save_btn->element, save_button_hovered);
atomic_set_unhover_handler(save_btn->element, save_button_unhovered);

// Le bouton réagira automatiquement avec :
// - Agrandissement au survol
// - Compression au clic avec couleur verte
// - Restauration automatique à l'état normal
```

Cette amélioration rend les boutons beaucoup plus interactifs et donne un feedback visuel immédiat à l'utilisateur ! 🎨✨

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
[atomic.c] Click callback for element
🎮 Play button clicked with visual feedback
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

## 🆕 Architecture d'événements optimisée

### ⚡ Nouvelle boucle d'événements dédiée

```
🔄 THREAD PRINCIPAL (Boucle de jeu - 60 FPS)
│
├── game_core_handle_events() → Traite le buffer d'événements
├── game_core_update() → Met à jour la logique
└── game_core_render() → Rendu graphique

⚡ THREAD D'ÉVÉNEMENTS (Capture continue)
│
├── window_poll_events() → Capture SDL en continu
├── event_loop_push_event() → Ajoute au buffer thread-safe
└── window_update_focus() → Gestion du focus
```

### 🎯 Avantages de cette architecture

1. **🚀 Performance** : Capture d'événements non-bloquante
2. **⚡ Réactivité** : Événements capturés en temps réel
3. **🔒 Thread-safe** : Buffer protégé par mutex
4. **📊 Scalabilité** : Buffer circulaire de 256 événements
5. **🎮 Fluidité** : Boucle principale à 60 FPS constant

### 📦 Buffer d'événements circulaire

```c
// Structure du buffer thread-safe
typedef struct EventLoop {
    WindowEvent* event_buffer;  // Buffer circulaire
    int buffer_size;           // 256 événements
    int buffer_head;           // Tête d'écriture (thread événements)
    int buffer_tail;           // Queue de lecture (thread principal)
    int buffer_count;          // Nombre d'événements en attente
    
    SDL_mutex* event_mutex;    // Protection thread-safe
    SDL_cond* event_condition; // Signalement d'événements
} EventLoop;
```

### 🔄 Flux complet optimisé

```
1. 🎯 Thread d'événements (arrière-plan) :
   ├── window_poll_events() capture SDL en continu
   ├── Filtrage et logs par fenêtre source
   └── event_loop_push_event() → Buffer thread-safe

2. 📦 Buffer circulaire :
   ├── Stockage de 256 événements max
   ├── Protection par mutex
   └── Pas de perte d'événements

3. 🎮 Thread principal (60 FPS) :
   ├── event_loop_pop_event() → Lit le buffer
   ├── Filtrage par fenêtre active
   ├── event_manager_handle_event() → UI
   └── Callbacks utilisateur exécutés
```

### 📊 Logs générés avec threading

```
[14:32:15] [EventLoop] [ThreadStarted] [core.c] : ⚡ Event capture thread started - independent from main loop
[14:32:15] [EventLoop] [EventCaptured] [core.c] : 🔄 Event captured in dedicated thread: type=1025 from window='Fanorona - Mini Window'
[14:32:15] [WindowEvents] [MouseDown] [window.c] : Mouse button 1 down in window 'Fanorona - Mini Window' at (150,200)
[14:32:15] [CoreEvents] [BufferProcessing] [Mini_window] : 📦 Processing buffered event #1: type=1025
[14:32:15] [CoreEvents] [EventTransmission] [Mini_window] : Event from buffer → Transmitting to event_manager
[14:32:15] [EventManager] [HitDetected] [event.c] : HIT! Element #1 at z-index 2 - calling callback
[14:32:15] [UserCallback] [PlayButton] [home_scene.c] : play_button_clicked callback executed
[14:32:15] [CoreEvents] [FrameSummary] [core] : Processed 3 events from buffer this frame
```

Cette architecture garantit une capture d'événements ultra-réactive tout en maintenant une boucle principale fluide ! 🎉