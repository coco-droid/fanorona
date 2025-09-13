#!/bin/bash

# Script de compilation pour Fanorona
echo "=== Compilation de Fanorona ==="

# Variables de configuration
PROJECT_NAME="fanorona"
SRC_DIR="src"
BUILD_DIR="build"
CC="gcc"
CFLAGS="-Wall -Wextra -std=c99 -g"
LIBS="-lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm"

# Créer le dossier build s'il n'existe pas
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
    echo "Dossier $BUILD_DIR créé"
fi

# Vérifier la présence du fichier background
if [ ! -f "assets/fix_bg.png" ]; then
    echo "Attention: Le fichier 'assets/fix_bg.png' est introuvable"
    echo "Le programme utilisera un background par défaut"
fi

# Liste des fichiers sources
SOURCES=(
    "main.c"
    "$SRC_DIR/core/core.c"
    "$SRC_DIR/event/event.c"
    "$SRC_DIR/window/window.c"
    "$SRC_DIR/scene/scene.c"
    "$SRC_DIR/scene/home_scene.c"
    "$SRC_DIR/utils/asset_manager.c"
    "$SRC_DIR/ui/native/atomic.c"
    "$SRC_DIR/ui/button.c"
)

# Vérifier que tous les fichiers sources existent
echo "Vérification des fichiers sources..."
for src in "${SOURCES[@]}"; do
    if [ ! -f "$src" ]; then
        echo "Erreur: Fichier source '$src' introuvable"
        exit 1
    fi
    echo "  ✓ $src"
done

# Compilation
echo "Compilation en cours..."
$CC $CFLAGS "${SOURCES[@]}" -o "$BUILD_DIR/$PROJECT_NAME" $LIBS

# Vérifier le résultat de la compilation
if [ $? -eq 0 ]; then
    echo "✓ Compilation réussie!"
    echo "L'exécutable se trouve dans: $BUILD_DIR/$PROJECT_NAME"
    
    # Demander si on veut exécuter le programme
    read -p "Voulez-vous exécuter le programme maintenant? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "=== Lancement de $PROJECT_NAME ==="
        # Exécuter depuis la racine du projet pour avoir le bon chemin vers assets/
        "./$BUILD_DIR/$PROJECT_NAME"
    fi
else
    echo "✗ Erreur de compilation!"
    exit 1
fi
