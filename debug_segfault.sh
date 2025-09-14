#!/bin/bash

echo "🔍 Diagnostic du segmentation fault..."

# Compiler avec des flags de débogage
echo "📦 Compilation avec débogage..."
gcc -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g -O0 \
    -I/usr/include/SDL2 \
    main.c \
    src/core/core.c \
    src/event/event.c \
    src/window/window.c \
    src/scene/scene.c \
    src/scene/simple_home_scene.c \
    src/utils/asset_manager.c \
    -lSDL2 -lSDL2_image -lm \
    -o debug_fanorona

if [ $? -ne 0 ]; then
    echo "❌ Erreur de compilation"
    exit 1
fi

echo "✅ Compilation réussie"

# Vérifier si le programme crash au lancement
echo "🚀 Test de lancement..."
timeout 3s ./debug_fanorona

if [ $? -eq 139 ]; then
    echo "❌ SEGFAULT détecté !"
    echo "🔧 Lancement avec gdb pour plus d'infos..."
    
    # Créer un script gdb pour l'automatiser
    cat > gdb_script.txt << EOF
set confirm off
run
bt
quit
EOF
    
    gdb -batch -x gdb_script.txt ./debug_fanorona
    rm gdb_script.txt
    
elif [ $? -eq 124 ]; then
    echo "✅ Programme fonctionne (timeout normal)"
else
    echo "⚠️  Programme s'est fermé avec code: $?"
fi

# Nettoyage
rm -f debug_fanorona

echo "📋 Diagnostic terminé"