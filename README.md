# Fanorona

Un jeu Fanorona développé en C avec SDL2.

## Structure du projet

```
fanoron-sivy/
├── main.c              # Point d'entrée principal avec la boucle de jeu
├── run.sh              # Script de compilation et d'exécution
├── Makefile            # Makefile pour la compilation
├── build/              # Dossier de compilation (créé automatiquement)
├── assets/             # 🆕 Ressources graphiques (images, SVG)
│   ├── bg-bg.png       # 🆕 Fond de la sidebar
│   └── profile-card.svg # 🆕 Fond des cartes joueur
└── src/
    ├── core/
    ├── event/
    ├── scene/
    ├── ui/
    ├── sound/          # 🆕 Système de son (musique + effets)
    │   ├── sound.h
    │   └── sound.c
    ├── utils/
    └── window/
```

## Fonctionnalités

### ✨ Sidebar de jeu (NOUVEAU)
- **🎨 Fond graphique**: Image `bg-bg.png` pour un style immersif
- **👥 Cartes joueur**: Design SVG `profile-card.svg` (hauteur réduite à 80px)
- **📊 Informations en temps réel**: Avatars, noms, captures, chronomètres
- **🎮 Contrôles de jeu**: Grille 2×2 positionnée en bas avec `Rectangle.svg` pour le fond
  - Boutons normaux: `btn.svg`
  - Bouton QUIT: `btn-brun.svg` (style spécial)
  - Texte réduit (7px) pour meilleure lisibilité
- **⚡ Asset Manager**: Chargement optimisé des textures avec fallback couleur
- **📐 Dimensions fixes**: 230px de largeur (1/3 de 800px)

### 🎮 Zone de jeu responsive (NOUVEAU)
- **📏 Dimensions adaptatives**: 
  - Container: 2/3 de la fenêtre (≈533px sur 800px de large)
  - Game area: 90% du container (marges visuelles de 5%)
  - Plateau: 95% de game_area (marges minimales)
- **🖼️ Background board.png**: 
  - Appliqué sur le container principal
  - Mode `stretch` pour remplir l'espace
  - Visible à travers game_area et plateau **transparents**
- **🎯 Zéro padding sur container**: Dimensions exactes sans perte d'espace
- **📐 Calculs automatiques**: Redimensionnement en cascade lors de `ui_cnt_playable_with_size()`
- **🔧 Transparence complète**: Plateau avec alpha=0 pour révéler board.png

### ✨ Système d'animations (NOUVEAU)
- **🎬 Animations keyframe-based**: Système inspiré de CSS avec support complet des keyframes
- **🔄 Fonctions d'easing**: linear, ease-in, ease-out, ease-in-out
- **📊 Propriétés animables**: Position (X,Y), taille (W,H), opacité
- **♾️ Contrôles avancés**: Itérations, va-et-vient, modes de remplissage
- **🎭 Animations prédéfinies**: Fade-in/out, slide, shake, pulse, bounce
- **⚡ Performance optimisée**: Nettoyage automatique des animations terminées

### Event Manager
- Système de souscription d'éléments aux événements par scène
- **🔧 Architecture par scène**: Chaque scène a son EventManager dédié
- **🔧 Connexion à la demande**: Événements connectés UNIQUEMENT pour la scène active
- Gestion par position (x, y, largeur, hauteur) et z-index
- Attribut `display` pour contrôler la visibilité
- Fonctions pour souscrire, désinscrire et effacer les handlers
- **🆕 Debug intégré**: `event_manager_debug_elements()` pour diagnostiquer

### Scene Manager
- **🆕 Gestion par scène**: Chaque scène a son propre EventManager
- **🔧 Connexion isolée**: Seule la scène active a ses événements connectés
- **🆕 Transitions automatiques**: Via SDL_USEREVENT et ui_link
- **🆕 Multi-fenêtres**: Support MINI (700x500) et MAIN (800x600)
- **🆕 Debug**: `scene_manager_debug_active_scenes()` pour diagnostiquer
- **🔧 Reconnexion automatique**: Lors des transitions, la nouvelle scène connecte ses événements

### Core
- **🆕 Thread dédié**: Capture des événements en thread séparé
- **🆕 Buffer circulaire**: Queue thread-safe pour les événements
- **🆕 Routage intelligent**: Événements routés vers la bonne scène/fenêtre
- **🆕 Debug complet**: `game_core_debug_event_system()` pour diagnostiquer

### UI Links
- **🆕 Navigation entre scènes**: `ui_create_link()` pour créer des liens
- **🆕 Transitions automatiques**: Support de 4 types de transitions
- **🆕 Multi-fenêtres**: Transition entre MINI et MAIN window

### Profile Scene (NOUVEAU)
- **🎭 Sélection d'avatar**: 6 avatars (p1.png à p6.png) avec rond cliquable
- **🖱️ Callbacks fonctionnels**: Clic sur mini-avatar → mise à jour immédiate de l'avatar principal
- **📝 Saisie de nom**: Champ de texte avec registre global par ID de scène (`input_name`)
- **🔒 Persistance garantie**: Texte enregistré immédiatement à chaque frappe dans le registre global
- **🔑 Système d'IDs**: Chaque scène utilise des IDs uniques (ex: `input_name` pour profile_scene)
- **🎨 Avatar principal**: Grand aperçu avec bordure dorée, mis à jour en temps réel
- **✨ Animations**: Fade-in, pulse, et slide pour une expérience fluide
- **✅ Validation**: Bouton neon "CONFIRMER" pour sauvegarder le profil
- **🔧 Architecture**: File-scoped static globals (no heap, no corruption)

### Choice Scene (NOUVEAU)
- **🎮 Choix de mode de jeu**: Local ou En ligne
- **🔗 Navigation intelligente**: Lien vers menu_scene pour le mode local
- **🌐 Placeholder en ligne**: Bouton préparé pour la fonctionnalité future
- **✨ Animations**: Slide-in depuis les côtés pour chaque bouton
- **🎨 Neon buttons**: Vert pour local, bleu pour en ligne

### 🔊 Système de son (NOUVEAU)
- **🎵 Musique**: Canal dédié avec support loop et fade
- **🔊 Effets sonores**: 16 canaux pour jouer simultanément
- **🎚️ Contrôle volume**: Musique et effets séparés (0-100%)
- **⚙️ Configuration globale**: Activer/désactiver sons, musique, effets
- **🎮 Helpers UI**: Fonctions directes pour boutons, pièces, événements
- **📂 Format supporté**: WAV, OGG, MP3 via SDL_mixer
- **🔄 Fallback automatique**: Sons carrés 16-bit générés si fichiers manquants
- **✨ Intégration UI**: Sons automatiques sur boutons avec `ui_button_enable_sound_feedback()`
- **🎯 Effets contextuels**: Sons distincts pour succès, erreur, déplacement, capture

### Utilisation du système de son

```c
// Dans main.c ou core.c - initialisation
if (!sound_init()) {
    printf("❌ Impossible d'initialiser le son\n");
}

// Charger les ressources audio (avec fallback automatique)
sound_load_music(MUSIC_MENU, "assets/music/menu.ogg");
sound_load_effect(SOUND_CLICK, "assets/sfx/click.wav");
// Si le fichier n'existe pas, un son carré sera généré automatiquement

// Jouer la musique du menu (loop infini)
sound_play_music(MUSIC_MENU, -1);

// 🆕 NOUVEAU: Intégration automatique dans les boutons
UINode* button = ui_button(tree, "btn", "JOUER", on_click, NULL);
ui_button_enable_sound_feedback(button);  // Active sons click/hover automatiques

// Dans les callbacks UI
void on_button_click(UINode* node, void* data) {
    sound_play_button_click();  // Joue le son ou son fallback
    // ...reste du code...
}

// 🆕 NOUVEAU: Sons contextuels pour feedback visuel
void on_success(void) {
    sound_play_effect(SOUND_SUCCESS);  // Son aigu 1400Hz
}

void on_error(void) {
    sound_play_effect(SOUND_ERROR);  // Triple bip 300Hz
}

void on_piece_move(void) {
    sound_play_piece_move();  // Sweep 400-600Hz
}

void on_piece_capture(void) {
    sound_play_piece_capture();  // Double tonalité 300+500Hz
}

// Nettoyage à la fin
sound_cleanup();
```

**Caractéristiques des sons générés:**
- Format: 16-bit signé (AUDIO_S16LSB)
- Fréquence: 44.1kHz
- Canaux: Mono
- Type: Onde carrée (tonalités électroniques claires)
- Durée: 30ms à 600ms selon l'effet
- Effets spéciaux: Sweeps (balayages de fréquence), double tonalités

**Avantages du fallback:**
- ✅ Jeu fonctionnel même sans fichiers audio
- ✅ Sons distinctifs pour chaque action
- ✅ Faible latence (sons générés en mémoire)
- ✅ Pas de dépendance externe

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

## 🔄 Navigation entre scènes

### Flux de jeu complet (NOUVEAU)

1. **Home Scene** → "JOUER" → **Choice Scene** (MINI window)
2. **Choice Scene** → "JOUER EN LOCAL" → **Menu Scene** (MINI window)
3. **Choice Scene** → "JOUER EN LIGNE" → **NetStart Scene** (MINI window)
4a. **NetStart Scene** → "LANCER UNE PARTIE" (hôte) → **Profile Scene**
4b. **NetStart Scene** → "REJOINDRE UNE PARTIE" (invité) → **Profile Scene**
5. **Menu Scene** → "JOUER EN MULTIJOUEUR" → **Profile Scene** → **Pieces Scene** → **Game Scene**
6. **Menu Scene** → "JOUER CONTRE L'IA" → **Profile Scene** → **AI Configuration Scene** → **Game Scene**

```c
// Navigation fluide avec transitions animées
// Home → Choice : SCENE_TRANSITION_REPLACE (même fenêtre)
// Choice → Menu : SCENE_TRANSITION_REPLACE (même fenêtre)
// Menu → Profile : SCENE_TRANSITION_REPLACE (même fenêtre)
// Profile → AI Config (mode IA) : SCENE_TRANSITION_REPLACE (même fenêtre)
// Profile → Pieces (mode multi) : SCENE_TRANSITION_REPLACE (même fenêtre)
// AI Config → Game : SCENE_TRANSITION_CLOSE_AND_OPEN (changement de fenêtre)
// Pieces → Game : SCENE_TRANSITION_CLOSE_AND_OPEN (changement de fenêtre)
```

**🎯 Caractéristiques du flux complet :**
- ✅ **Mode IA activé dès le menu** : Le mode VS_AI est configuré avant la création de profil
- ✅ **Profil adaptatif** : 1 joueur en mode IA, 2 joueurs en multijoueur
- ✅ **Configuration séquentielle** : Profil → Difficulté IA → Couleurs → Jeu
- ✅ **Transitions fluides** : Animations pour chaque changement de scène
- ✅ **Fenêtres adaptées** : Config en MINI, jeu en MAIN

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
```   ANIMATE_SHAKE(button, 0.4f, 8.0f);  // Secousse 0.4s avec intensité 8px
}
### Animations d'erreur et feedback

```cnt) {
// Secouer un bouton en cas d'erreur
    ANIMATE_SHAKE(button, 0.4f, 8.0f);  // Secousse 0.4s avec intensité 8pxMATION_PROPERTY_WIDTH, 0.3f);
}
animation_add_keyframe(scale_up, 1.0f, 110.0f, "ease-out");    // Animation de succès
void show_success_feedback(UINode* element) {
    // Animation combinée : scale + fade   ui_node_add_animation(element, scale_up);
    Animation* scale_up = animation_create("success-scale", ANIMATION_PROPERTY_WIDTH, 0.3f);
    animation_add_keyframe(scale_up, 0.0f, 100.0f, "ease-out");```
    animation_add_keyframe(scale_up, 1.0f, 110.0f, "ease-out");
    ### Transitions entre scènes
    ANIMATE_FADE_IN(element, 0.2f);
    ui_node_add_animation(element, scale_up);
}
``` void (*on_complete_callback)(void)) {
elle
### Transitions entre scènesANIMATE_FADE_OUT(container, 0.5f);

```c
// Animation de sortie avant changement de scènee système)
void animate_scene_exit(UINode* container, void (*on_complete_callback)(void)) {   schedule_callback(on_complete_callback, 0.5f);
    // Faire disparaître la scène actuelle
    ANIMATE_FADE_OUT(container, 0.5f);```
    
    // Programmer le changement de scène après l'animation### Macros disponibles
    // (utilisez un timer ou un callback dans votre système)
    schedule_callback(on_complete_callback, 0.5f);
}
```ndu

### Macros disponibles
le
```c
ANIMATE_FADE_IN(node, duration)                   
printf("Animations actives: %d\n", active_count);
n élément
// Vérifier si un élément a des animationsnode_stop_animations(mon_bouton);
if (ui_node_has_active_animations(mon_bouton)) {```
    printf("Le bouton est en cours d'animation\n");}## Debug du système d'événements

// Arrêter toutes les animations d'un élément**🔧 Logs réduits** : Les logs verbeux ont été considérablement réduits pour une meilleure lisibilité.
ui_node_stop_animations(mon_bouton);
```Si aucun événement ne fonctionne, utiliser ces fonctions de debug dans le code:

## Debug du système d'événements

**🔧 Logs réduits** : Les logs verbeux ont été considérablement réduits pour une meilleure lisibilité.

Si aucun événement ne fonctionne, utiliser ces fonctions de debug dans le code:e_core_force_scene_event_registration(core);  // Force la re-connexion
```
```c
// Dans main.c ou dans une fonction de debug
game_core_debug_event_system(core);  // Debug complet du système
scene_manager_debug_active_scenes(scene_manager);  // Debug des scènesouvements souris)
game_core_force_scene_event_registration(core);  // Force la re-connexion
```- ❌ Logs de synchronisation verbeux supprimés

**📊 Console plus propre** :## Problèmes courants et solutions
- ✅ Logs critiques conservés (QUIT, fermeture fenêtre)
- ❌ Logs de routine supprimés (clics, mouvements souris)
- ❌ Logs de rendu périodiques supprimésupdate()
- ❌ Logs de synchronisation verbeux supprimés en millisecondes

## Problèmes courants et solutions4. **Mémoire insuffisante** : Le système peut refuser de nouvelles animations si la mémoire est limitée

### Animations qui ne fonctionnent pas
1. **Oublier ui_update_animations()** : Cette fonction DOIT être appelée dans chaque scene_update()
2. **Delta_time incorrect** : Vérifier que delta_time est en secondes, pas en millisecondess terminées
3. **Nœud détruit** : Ne pas détruire un nœud qui a des animations actives3. **Monitoring** : Utiliser `ui_get_active_animations_count()` pour surveiller
4. **Mémoire insuffisante** : Le système peut refuser de nouvelles animations si la mémoire est limitée

1. **Trop d'animations simultanées** : Limiter à ~20-30 animations actives maximum3. **Monitoring** : Utiliser `ui_get_active_animations_count()` pour surveiller4. **Vérifier la fenêtre**: L'événement doit venir de la bonne fenêtre
2. **Nettoyage automatique** : Le système nettoie automatiquement les animations terminéestrés
3. **Monitoring** : Utiliser `ui_get_active_animations_count()` pour surveiller4. **Vérifier la fenêtre**: L'événement doit venir de la bonne fenêtre
`oit être appelé
### Événements non détectés2. **Vérifier l'EventManager**: Chaque scène doit avoir son EventManager
1. **Vérifier l'initialisation**: La scène doit être `initialized = true`oit être appelé
2. **Vérifier l'EventManager**: Chaque scène doit avoir son EventManager








3. **Vérifier les fenêtres**: Les fenêtres cibles doivent être créées2. **Vérifier les IDs**: Les IDs de scène doivent correspondre1. **Vérifier la connexion**: `ui_link_connect_to_manager()` doit être appelé### Transitions qui ne fonctionnent pas4. **Vérifier la fenêtre**: L'événement doit venir de la bonne fenêtre3. **Vérifier l'enregistrement**: Les éléments UI doivent être enregistrés3. **Vérifier les fenêtres**: Les fenêtres cibles doivent être créées3. **Vérifier les fenêtres**: Les fenêtres cibles doivent être créées2. **Vérifier les IDs**: Les IDs de scène doivent correspondre1. **Vérifier la connexion**: `ui_link_connect_to_manager()` doit être appelé### Transitions qui ne fonctionnent pas4. **Vérifier la fenêtre**: L'événement doit venir de la bonne fenêtre3. **Vérifier l'enregistrement**: Les éléments UI doivent être enregistrés3. **Vérifier les fenêtres**: Les fenêtres cibles doivent être créées