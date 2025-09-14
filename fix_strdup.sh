#!/bin/bash

echo "🔧 Application du patch strdup..."

# Sauvegarder le fichier original
cp run.sh run.sh.backup

# Appliquer le patch pour ajouter les bonnes flags
sed -i 's/-std=c99 -g"/-std=c99 -D_POSIX_C_SOURCE=200809L -g"/g' run.sh

echo "✅ Patch appliqué à run.sh"
echo "   Flags ajoutées : -D_POSIX_C_SOURCE=200809L"
echo ""
echo "🚀 Vous pouvez maintenant compiler avec :"
echo "   ./run.sh"
echo ""
echo "📝 Résumé des corrections :"
echo "   - Ajout de #define _POSIX_C_SOURCE 200809L dans tous les fichiers"
echo "   - Création d'un fichier de compatibilité src/utils/compat.h"
echo "   - Flags de compilation mises à jour"
echo ""
echo "🎯 Tous les warnings strdup devraient être résolus !"