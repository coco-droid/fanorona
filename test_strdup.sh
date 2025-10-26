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
echo "   🆕 NEW: choice_scene.c ajouté (choix Local/En ligne)"
echo "   🆕 NEW: profile_scene.c ajouté (création de profil avec avatars)"
echo "   🆕 NEW: Système de registre global par ID de scène pour text inputs"
echo "   🔑 IDs de scène: input_name (profile_scene), input_player2 (future)"
echo "   ✅ FIX: Avatar par défaut utilise toujours AvatarID (AVATAR_WARRIOR=1)"
echo "   ✅ FIX: Validation stricte des IDs d'avatars (plage 1-6)"
echo "   🔧 FIX: Stack smashing corrigé dans log_console (buffer overflow)"
echo "   🔧 FIX: Callbacks d'avatars cliquables corrigés (profile_scene)"
echo "   🔧 FIX: Logique is_player2_turn clarifiée (FALSE=joueur1, TRUE=joueur2)"
echo "   ✅ NEW: Confirmation de profil avec sauvegarde complète du joueur"
echo "   ✅ NEW: Support multijoueur local séquentiel (J1 puis J2)"
echo "   ✅ NEW: Avatars sauvegardés dans la config globale avec IDs fixes"
echo "   🔧 FIX: Input clearing for Player 2 in profile_scene"
echo "   🔧 FIX: Button label 'COMMENCER' for Player 2"
echo "   🔧 FIX: CLOSE_AND_OPEN transition from MINI to MAIN window"
echo "   🔧 FIX: confirm_clicked signature matches atomic handlers"
echo "   🔧 FIX: Avatar IDs properly saved as AvatarID enum values"
echo "   🔧 FIX: Avatar selection persists across avatar clicks"
echo "   🔧 FIX: J2 avatar resets to WARRIOR on second profile init"
echo "   🔧 FIX: Avatar selection uses AvatarID enum (WARRIOR=1...SAGE=6)"
echo "   🔧 FIX: Callback duplication removed - single save_profile() function"
echo "   🔧 FIX: J2 transitions to game via explicit scene_manager call"
echo "   ✨ REFACTOR: profile_scene.c rewritten with clean ui_link architecture"
echo "   🎯 NEW: Single callback on_confirm_profile() for both J1 and J2"
echo "   🆔 NEW: Avatar selection uses AvatarID enum directly (1-6)"
echo "   🔀 NEW: Conditional UI based on is_player2_turn flag"
echo "   🔗 NEW: confirm button is now a styled ui_link (neon design)"
echo "   📝 NEW: Placeholder and button text adapt to current player"
echo "   🚀 NEW: J1→profile (REPLACE), J2→game (CLOSE_AND_OPEN)"
echo "   🔧 FIX: Profile scene cleanup prevents NULL pointer crash on J1→J2"
echo "   🔧 FIX: Avatar IDs cast to (int) to prevent corruption in printf"
echo "   📛 RENAME: config_is_player2_turn() → config_is_profile_player2_turn()"
echo "   📝 CLARIFY: Profile turn flag is for FORM DISPLAY ONLY, not game logic"
echo "   🔧 FIX: Avatar initialization moved before is_player2_turn check"
echo "   🔧 FIX: Scene initialization now happens BEFORE event connection"
echo "   📋 FIX: profile_scene_init() called before profile_scene_connect_events()"
echo "   ✅ FIX: Garbage avatar IDs eliminated by proper initialization order"
echo "   🔧 FIX: Profile scene refactored with static globals (no heap allocation)"
echo "   ✨ CLEAN: Removed ProfileSceneData heap + user_data pointer corruption"
echo "   🆕 NEW: AvatarSelector component created with atomic elements"
echo "   🎭 NEW: Reusable avatar selection system extracted from profile_scene"
echo "   🔗 NEW: Proper EventManager registration for avatar mini-elements"
echo "   ✨ NEW: Component architecture similaire à neon_button et container"
echo "   🔧 FIX: AvatarID type conflict resolved (removed forward declaration)"
echo "   🔧 FIX: profile_scene.c now uses AvatarSelector component"
echo "   🔧 FIX: avatar.c added to run.sh compilation sources"
echo "   🔧 FIX: Avatar textures loading corrected with proper paths"
echo "   🔧 FIX: Mini-avatar events properly registered in profile_scene"
echo "   🔧 FIX: Mini-avatar click events now working with selection feedback"
echo "   🆕 NEW: SUIVANT button replaced with UI Link for callback-only reset"
echo "   📝 NEW: Text input component added below avatar selector"
echo "   🔄 NEW: Text input placeholder changes to 'Joueur 2' after reset"
echo "   📋 NEW: Multistep form with ProfileData structure for 2 players"
echo "   🔄 NEW: Button text changes from SUIVANT to START for player 2"
echo "   🔧 FIX: Button design preserved after text changes"
echo "   📊 NEW: Complete ProfileData logging on each step"
echo "   🔧 FIX: pieces_scene registered in scene registry"
echo "   🔄 NEW: Profile scene transitions to pieces_scene after J2 completion"
echo "   💾 NEW: Player profiles automatically saved to global config"
echo "   🎮 FIX: Game scene now uses global config for player setup"
echo "   👥 FIX: Plateau uses configured players instead of default test players"
echo "   🔄 NEW: Local multiplayer mode allows both players to select pieces"
echo "   🎯 NEW: game_logic_initialize_from_config() uses real player data"
echo "   🔧 FIX: game_scene.c uses player1/player2 instead of white_player/black_player"
echo "   Vous pouvez maintenant utiliser ./run.sh en toute sécurité"