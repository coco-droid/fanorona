#!/bin/bash

echo "🔧 Analyse rapide des problèmes potentiels..."

echo "1. Vérification SDL_RenderPresent dans main.c:"
if grep -n "SDL_RenderPresent" main.c > /dev/null; then
    echo "   ✅ SDL_RenderPresent trouvé dans main.c"
    grep -n "SDL_RenderPresent" main.c
else
    echo "   ❌ SDL_RenderPresent MANQUANT dans main.c - PROBLÈME MAJEUR !"
fi

echo ""
echo "2. Vérification de l'initialisation des scènes:"
grep -n "create_home_scene" main.c

echo ""
echo "3. Structure de la boucle principale:"
grep -A 5 -B 5 "SDL_PollEvent\|SDL_Quit\|scene.*render" main.c

echo ""
echo "🎯 Solutions suggérées:"
echo "   - Ajouter SDL_RenderPresent() après scene_render() dans main.c"
echo "   - Vérifier que la scène est bien créée et initialisée"
echo "   - S'assurer que les événements SDL sont gérés correctement"
echo ""
echo "🚀 Exécutez ce diagnostic: chmod +x debug_segfault.sh && ./debug_segfault.sh"