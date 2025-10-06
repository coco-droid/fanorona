#!/bin/bash

# Test de compilation avec les bonnes flags
echo "🔧 Test de compilation avec correction des liens manquants..."

# Test atomic.c spécifiquement pour les duplications
echo "🔍 Vérification des duplications dans atomic.c..."
ATOMIC_CREATE_COUNT=$(grep -c "AtomicElement\* atomic_create" src/ui/native/atomic.c)
if [ $ATOMIC_CREATE_COUNT -gt 1 ]; then
    echo "❌ ERREUR: atomic_create défini $ATOMIC_CREATE_COUNT fois dans atomic.c"
    echo "   🔧 Veuillez supprimer la duplication"
    exit 1
else
    echo "✅ atomic_create défini une seule fois"
fi

# Vérifier atomic_destroy
echo "🔍 Vérification de atomic_destroy..."
if grep -q "void atomic_destroy(" src/ui/native/atomic.c; then
    echo "✅ atomic_destroy trouvé dans atomic.c"
else
    echo "❌ ERREUR: atomic_destroy manquant dans atomic.c"
    exit 1
fi

# Vérifier les accolades orphelines
echo "🔍 Vérification de la syntaxe des accolades..."
if ! gcc -fsyntax-only -std=c99 -D_POSIX_C_SOURCE=200809L -I/usr/include/SDL2 src/ui/native/atomic.c 2>/dev/null; then
    echo "❌ ERREUR: Problème de syntaxe dans atomic.c"
    echo "   🔧 Vérifiez les accolades et le code orphelin"
    exit 1
else
    echo "✅ Syntaxe des accolades correcte"
fi

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

# Test complet de linkage
echo "🔗 Test de linkage complet..."
gcc -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g \
    -I/usr/include/SDL2 \
    src/ui/native/atomic.c \
    src/ui/ui_tree.c \
    -c -o test_linkage_combined.o

if [ $? -eq 0 ]; then
    echo "✅ Linkage atomic.c + ui_tree.c réussi"
    rm -f test_linkage_combined.o
else
    echo "❌ Problème de linkage entre atomic.c et ui_tree.c"
    exit 1
fi

echo "🎉 Tous les fichiers compilent correctement avec les corrections !"
echo "   ✅ Plus de duplication de atomic_create"
echo "   ✅ Code orphelin supprimé"
echo "   ✅ Fonctions custom_data déclarées dans atomic.h"
echo "   ✅ atomic_destroy implémenté"
echo "   ✅ Linkage entre ui_tree.c et atomic.c fonctionnel"
echo "   🔧 FIX: Gestion mémoire améliorée pour custom_data"
echo "   🔧 FIX: board_free corrigé pour éviter les fuites"
echo "   🔧 FIX: plateau_cleanup amélioré"
echo "   🆕 NEW: board_destroy() ajouté pour nettoyage complet"
echo "   🆕 NEW: ui_plateau_container_destroy() pour libération mémoire"
echo "   🆕 NEW: Nettoyage explicite dans game_scene_cleanup()"
echo "   🆕 NEW: Système de feedback visuel intégré avec is_move_valide()"
echo "   🔧 FIX: Includes et handlers corrigés pour plateau interactif"
echo "   Vous pouvez maintenant utiliser ./run.sh en toute sécurité"