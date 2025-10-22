#include "config.h"
#include <stdio.h>
#include <string.h>

// 🆕 CONFIGURATION GLOBALE STATIQUE
static GameConfig g_config = {
    .current_mode = GAME_MODE_NONE,
    .ai_difficulty = AI_DIFFICULTY_MEDIUM,
    .player1_name = "Joueur 1",
    .player2_name = "Joueur 2",
    .ai_plays_as_white = false,  // IA joue les noirs par défaut
    .sound_enabled = true,
    .animations_enabled = true
};

// 🆕 INITIALISATION DE LA CONFIGURATION
void config_init(void) {
    printf("⚙️ Initialisation de la configuration du jeu\n");
    
    // Valeurs par défaut déjà définies dans la structure statique
    printf("   📋 Mode par défaut: %s\n", config_mode_to_string(g_config.current_mode));
    printf("   🤖 Difficulté IA par défaut: %s\n", config_difficulty_to_string(g_config.ai_difficulty));
    printf("   👤 Joueur 1: %s\n", g_config.player1_name);
    printf("   👤 Joueur 2: %s\n", g_config.player2_name);
    printf("   🎨 IA joue en: %s\n", g_config.ai_plays_as_white ? "Blanc" : "Noir");
    
    printf("✅ Configuration initialisée\n");
}

// 🆕 GESTION DU MODE DE JEU
void config_set_mode(GameMode mode) {
    GameMode old_mode = g_config.current_mode;
    g_config.current_mode = mode;
    
    printf("🎮 Mode de jeu changé: %s → %s\n", 
           config_mode_to_string(old_mode), 
           config_mode_to_string(mode));
    
    // Ajustements automatiques selon le mode
    switch (mode) {
        case GAME_MODE_LOCAL_MULTIPLAYER:
            printf("   👥 Mode multijoueur local activé\n");
            printf("   ⌨️ Deux joueurs humains sur le même PC\n");
            break;
            
        case GAME_MODE_ONLINE_MULTIPLAYER:
            printf("   🌐 Mode multijoueur en ligne activé\n");
            printf("   📡 Connexion réseau requise\n");
            break;
            
        case GAME_MODE_VS_AI:
            printf("   🤖 Mode contre IA activé\n");
            printf("   🧠 Difficulté: %s\n", config_difficulty_to_string(g_config.ai_difficulty));
            printf("   🎨 IA joue en: %s\n", g_config.ai_plays_as_white ? "Blanc" : "Noir");
            break;
            
        case GAME_MODE_NONE:
            printf("   ❌ Aucun mode sélectionné\n");
            break;
    }
}

GameMode config_get_mode(void) {
    return g_config.current_mode;
}

// 🆕 GESTION DE LA DIFFICULTÉ IA
void config_set_ai_difficulty(AIDifficulty difficulty) {
    g_config.ai_difficulty = difficulty;
    printf("🤖 Difficulté IA définie à: %s\n", config_difficulty_to_string(difficulty));
}

AIDifficulty config_get_ai_difficulty(void) {
    return g_config.ai_difficulty;
}

// 🆕 GESTION DES NOMS DE JOUEURS
void config_set_player_names(const char* player1, const char* player2) {
    if (player1) {
        strncpy(g_config.player1_name, player1, sizeof(g_config.player1_name) - 1);
        g_config.player1_name[sizeof(g_config.player1_name) - 1] = '\0';
    }
    
    if (player2) {
        strncpy(g_config.player2_name, player2, sizeof(g_config.player2_name) - 1);
        g_config.player2_name[sizeof(g_config.player2_name) - 1] = '\0';
    }
    
    printf("👤 Noms des joueurs mis à jour: '%s' vs '%s'\n", 
           g_config.player1_name, g_config.player2_name);
}

const char* config_get_player1_name(void) {
    return g_config.player1_name;
}

const char* config_get_player2_name(void) {
    return g_config.player2_name;
}

// 🆕 GESTION DE LA COULEUR DE L'IA
void config_set_ai_color(bool ai_plays_white) {
    g_config.ai_plays_as_white = ai_plays_white;
    printf("🤖 IA configurée pour jouer en: %s\n", ai_plays_white ? "Blanc" : "Noir");
}

bool config_is_ai_white(void) {
    return g_config.ai_plays_as_white;
}

// 🆕 ACCÈS À LA CONFIGURATION COMPLÈTE
GameConfig* config_get_current(void) {
    return &g_config;
}

// 🆕 FONCTIONS UTILITAIRES DE CONVERSION
const char* config_mode_to_string(GameMode mode) {
    switch (mode) {
        case GAME_MODE_NONE:              return "Aucun mode";
        case GAME_MODE_LOCAL_MULTIPLAYER: return "Multijoueur local";
        case GAME_MODE_ONLINE_MULTIPLAYER:return "Multijoueur en ligne";
        case GAME_MODE_VS_AI:             return "Contre IA";
        default:                          return "Mode inconnu";
    }
}

const char* config_difficulty_to_string(AIDifficulty difficulty) {
    switch (difficulty) {
        case AI_DIFFICULTY_EASY:   return "Facile";
        case AI_DIFFICULTY_MEDIUM: return "Moyen";
        case AI_DIFFICULTY_HARD:   return "Difficile";
        default:                   return "Inconnue";
    }
}

// 🆕 FONCTION RAPIDE pour activer le mode IA
void config_enable_ai_mode(void) {
    config_set_mode(GAME_MODE_VS_AI);
    printf("⚡ Mode IA activé rapidement avec configuration par défaut\n");
}
