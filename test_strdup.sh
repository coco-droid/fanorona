#!/bin/bash

# Test de compilation avec les bonnes flags
echo "🔧 Test de compilation avec correction strdup..."

gcc -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g \
    -I/usr/include/SDL2 \
    -c src/ui/native/atomic.c \
    -o test_atomic.o

if [ $? -eq 0 ]; then
    echo "✅ atomic.c compile correctement"
    rm -f test_atomic.o
else
    echo "❌ Problème de compilation avec atomic.c"
    exit 1
fi

gcc -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g \
    -I/usr/include/SDL2 \
    -c src/ui/ui_tree.c \
    -o test_tree.o

if [ $? -eq 0 ]; then
    echo "✅ ui_tree.c compile correctement"
    rm -f test_tree.o
else
    echo "❌ Problème de compilation avec ui_tree.c"
    exit 1
fi

echo "🎉 Tous les fichiers compilent correctement avec strdup !"
echo "   Vous pouvez maintenant utiliser ./run.sh en toute sécurité"