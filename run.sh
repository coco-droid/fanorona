#!/bin/bash

# filepath: /home/t4x/Desktop/fanorona/run.sh
# Script de compilation pour Fanorona
echo "=== Compilation de Fanorona ==="

# Variables de configuration
PROJECT_NAME="fanorona"
SRC_DIR="src"
BUILD_DIR="build"
CC="gcc"
CFLAGS="-Wall -Wextra -std=c99 -g"
LIBS="-lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm"

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
            echo "🖥️ Console de logs séparée : ACTIVÉE"
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
    echo "📝 Flag de compilation ajouté : -DENABLE_LOG_CONSOLE"
fi

# ...existing code jusqu'à la liste des sources...

# Liste des fichiers sources
SOURCES=(
    "main.c"
    "$SRC_DIR/core/core.c"
    "$SRC_DIR/event/event.c"
    "$SRC_DIR/window/window.c"
    "$SRC_DIR/scene/scene.c"
    "$SRC_DIR/scene/home_scene.c"
    "$SRC_DIR/utils/asset_manager.c"
    "$SRC_DIR/utils/log_console.c"
    "$SRC_DIR/ui/native/atomic.c"
    "$SRC_DIR/ui/ui_tree.c"
    "$SRC_DIR/ui/ui_components.c"
)

# ...existing code jusqu'à la compilation...

# Compilation
echo "Compilation en cours..."
$CC $CFLAGS "${SOURCES[@]}" -o "$BUILD_DIR/$PROJECT_NAME" $LIBS

# Vérifier le résultat de la compilation
if [ $? -eq 0 ]; then
    echo "✓ Compilation réussie!"
    echo "L'exécutable se trouve dans: $BUILD_DIR/$PROJECT_NAME"
    
    if [ "$ENABLE_LOG_CONSOLE" = true ]; then
        echo ""
        echo "🖥️ Console de logs activée :"
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
            echo "🖥️ Ouverture de la console de logs..."
            # Vérifier que nous avons un serveur X
            if [ -z "$DISPLAY" ]; then
                echo "⚠️ Attention: Variable DISPLAY non définie, la console de logs pourrait ne pas s'ouvrir"
            fi
        fi
        
        # Exécuter depuis la racine du projet pour avoir le bon chemin vers assets/
        "./$BUILD_DIR/$PROJECT_NAME"
        
        if [ "$ENABLE_LOG_CONSOLE" = true ]; then
            echo "🧹 Nettoyage des processus de logs..."
            # Tuer les processus de terminal qui pourraient rester
            pkill -f "Fanorona - Console de Logs" 2>/dev/null || true
        fi
    fi
else
    echo "✗ Erreur de compilation!"
    exit 1
fi