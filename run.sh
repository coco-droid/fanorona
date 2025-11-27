#!/bin/bash

# filepath: /home/t4x/Desktop/fanorona/run.sh
# Script de compilation pour Fanorona
echo "=== Compilation de Fanorona ==="

# Variables de configuration
PROJECT_NAME="fanorona"
SRC_DIR="src"
BUILD_DIR="build"

[ -x "check.sh" ] && ./check.sh && sleep 0.6

CC="gcc"
CFLAGS="-Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -g"
LIBS="-lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net -lm"

# Flags optionnels
ENABLE_LOG_CONSOLE=false

# === TRAITEMENT DES ARGUMENTS ===

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --logs, -l     Activer la console de logs séparée"
    echo "  --help, -h     Afficher cette aide"
    echo ""
    echo "Exemples:"
    echo "  $0              # Compilation normale"
    echo "  $0 --logs       # Compilation avec console de logs"
    echo "  $0 -l           # Version courte"
}

# Parser les arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --logs|-l)
            ENABLE_LOG_CONSOLE=true
            echo "Console de logs séparée : ACTIVÉE"
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            echo "Argument inconnu: $1"
            show_help
            exit 1
            ;;
    esac
done

# Ajouter le flag de compilation si nécessaire
if [ "$ENABLE_LOG_CONSOLE" = true ]; then
    CFLAGS="$CFLAGS -DENABLE_LOG_CONSOLE"
    echo "Flag de compilation ajouté : -DENABLE_LOG_CONSOLE"
fi

# Créer les répertoires nécessaires
mkdir -p "$BUILD_DIR"
mkdir -p "$BUILD_DIR/$SRC_DIR/core"
mkdir -p "$BUILD_DIR/$SRC_DIR/event"
mkdir -p "$BUILD_DIR/$SRC_DIR/window"
mkdir -p "$BUILD_DIR/$SRC_DIR/scene"
mkdir -p "$BUILD_DIR/$SRC_DIR/utils"
mkdir -p "$BUILD_DIR/$SRC_DIR/ui/native"
mkdir -p "$BUILD_DIR/$SRC_DIR/ui/components"  # Nouveau répertoire pour les composants UI

# Liste des fichiers sources mise à jour avec les nouveaux fichiers
SOURCES=(
    "main.c"
    "$SRC_DIR/core/core.c"
    "$SRC_DIR/event/event.c"
    "$SRC_DIR/window/window.c"
    "$SRC_DIR/config.c"
    "$SRC_DIR/scene/scene.c"
    "$SRC_DIR/scene/scene_manager.c"
    "$SRC_DIR/scene/home_scene.c"
    "$SRC_DIR/scene/choice_scene.c"
    "$SRC_DIR/scene/menu_scene.c"
    "$SRC_DIR/scene/profile_scene.c"
    "$SRC_DIR/scene/game_scene.c"
    "$SRC_DIR/scene/ai_scene.c"
    "$SRC_DIR/scene/wiki_scene.c"
    "$SRC_DIR/scene/pieces_scene.c"
    "$SRC_DIR/scene/net_start_scene.c"
    "$SRC_DIR/scene/lobby_scene.c"
    "$SRC_DIR/scene/player_list_scene.c"
    "$SRC_DIR/scene/setting_scene.c"
    "$SRC_DIR/scene/scene_registry.c"
    "$SRC_DIR/ui/ui_tree.c"
    "$SRC_DIR/ui/ui_components.c"
    "$SRC_DIR/ui/animation.c"
    "$SRC_DIR/ui/avatar.c"
    "$SRC_DIR/ui/cnt_ui.c"
    "$SRC_DIR/ui/cnt_playable.c"
    "$SRC_DIR/ui/sidebar.c"
    "$SRC_DIR/ui/plateau_cnt.c"
    "$SRC_DIR/ui/neon_btn.c"
    "$SRC_DIR/ui/text_input.c"
    "$SRC_DIR/ui/components/ui_link.c"
    "$SRC_DIR/ui/native/atomic.c"
    "$SRC_DIR/ui/native/optimum.c"
    "$SRC_DIR/plateau/plateau.c"
    "$SRC_DIR/pions/pions.c"
    "$SRC_DIR/logic/rules.c"
    "$SRC_DIR/logic/mode.c"
    "$SRC_DIR/ai/ai.c"
    "$SRC_DIR/ai/minimax.c"
    "$SRC_DIR/ai/markov.c"
    "$SRC_DIR/ai/mcts.c"  # 🆕 Ajouter MCTS
    "$SRC_DIR/stats/game_stats.c"
    "$SRC_DIR/types/plateau_types.c"
    "$SRC_DIR/net/network.c"
    "$SRC_DIR/sound/sound.c"
    "$SRC_DIR/net/protocol.c"
    "$SRC_DIR/serializer/serializer.c"
    "$SRC_DIR/utils/asset_manager.c"
    "$SRC_DIR/utils/log_console.c"
)

# Vérification préliminaire de l'existence des fichiers
missing_files=false
for source in "${SOURCES[@]}"; do
    if [ ! -f "$source" ]; then
        echo "Fichier manquant: $source"
        missing_files=true
    fi
done

if [ "$missing_files" = true ]; then
    echo "Certains fichiers sont manquants. Veuillez vérifier les chemins."
    echo "Compilation annulée."
    exit 1
fi

# VÉRIFICATION SPÉCIFIQUE pour ui_link.c
echo "Vérification spécifique de ui_link.c..."
if [ -f "$SRC_DIR/ui/components/ui_link.c" ]; then
    if grep -q "scene_manager_transition_to_scene_from_element" "$SRC_DIR/ui/components/ui_link.c"; then
        echo "   ui_link.c: Fonction de transition trouvée"
    else
        echo "   ui_link.c: Fonction de transition manquante"
        echo "   Vérifiez que scene_manager.c implémente cette fonction"
    fi
    
    if grep -q "ui_link_connect_to_manager" "$SRC_DIR/ui/components/ui_link.c"; then
        echo "   ui_link.c: Fonction de connexion trouvée"
    else
        echo "   ui_link.c: Fonction de connexion manquante"
    fi
else
    echo "   ui_link.c non trouvé - sera ignoré lors de la compilation"
    # Retirer ui_link.c de la liste si le fichier n'existe pas
    SOURCES=(${SOURCES[@]/*ui_link.c*/})
fi

# VÉRIFICATION SPÉCIFIQUE pour les conflits de définition
echo "Vérification des conflits de définition..."
if grep -n "ui_neon_button" "$SRC_DIR/ui/ui_components.c" | grep -q "^[0-9]*:.*{"; then
    echo "   ATTENTION: ui_neon_button défini dans ui_components.c"
    echo "   Assurez-vous qu'il n'y a pas de conflit avec neon_btn.c"
fi

if [ -f "$SRC_DIR/ui/neon_btn.c" ]; then
    if grep -q "ui_neon_button" "$SRC_DIR/ui/neon_btn.c"; then
        echo "   neon_btn.c: Implémentation complète trouvée"
    fi
else
    echo "   neon_btn.c non trouvé"
fi

# Compilation
echo "Compilation en cours..."
echo "$CC $CFLAGS ${SOURCES[*]} -o $BUILD_DIR/$PROJECT_NAME $LIBS"
$CC $CFLAGS "${SOURCES[@]}" -o "$BUILD_DIR/$PROJECT_NAME" $LIBS

# Vérifier le résultat de la compilation
if [ $? -eq 0 ]; then
    echo "Compilation réussie!"
    echo "L'exécutable se trouve dans: $BUILD_DIR/$PROJECT_NAME"
    
    if [ "$ENABLE_LOG_CONSOLE" = true ]; then
        echo ""
        echo "Console de logs activée :"
        echo "   - Une fenêtre séparée s'ouvrira pour les logs"
        echo "   - Les événements souris seront trackés dès l'entrée dans la fenêtre"
        echo "   - Utilisez Ctrl+C dans le terminal principal pour fermer"
        echo ""
    fi
    
    # Demander si on veut exécuter le programme
    read -p "Voulez-vous exécuter le programme maintenant? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "=== Lancement de $PROJECT_NAME ==="
        
        if [ "$ENABLE_LOG_CONSOLE" = true ]; then
            echo "Ouverture de la console d'événements..."
            echo "DEUX fenêtres vont s'ouvrir :"
            echo "   1. Fenêtre de jeu (Fanorona)"
            echo "   2. Console d'événements UI (debugging)"
            echo ""
            echo "Dans la console d'événements vous verrez :"
            echo "   • Mouvements de souris en temps réel"
            echo "   • Clics sur les boutons"
            echo "   • Tests de collision (hit testing)"
            echo "   • Événements de hover/unhover"
            echo "   • Debugging complet des interactions"
            echo ""
            # Vérifier que nous avons un serveur X
            if [ -z "$DISPLAY" ]; then
                echo "Attention: Variable DISPLAY non définie, la console d'événements pourrait ne pas s'ouvrir"
            fi
        fi
        
        # Exécuter depuis la racine du projet pour avoir le bon chemin vers assets/
        "./$BUILD_DIR/$PROJECT_NAME"
        
        if [ "$ENABLE_LOG_CONSOLE" = true ]; then
            echo "Nettoyage des processus de logs..."
            # Tuer les processus de terminal qui pourraient rester
            pkill -f "Fanorona.*Console.*Événements" 2>/dev/null || true
            pkill -f "Events Console" 2>/dev/null || true
        fi
    fi
else
    echo "Erreur de compilation!"
    exit 1
fi