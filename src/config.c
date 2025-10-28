#include "config.h"
#include <string.h>
#include <stdio.h>

// 🆕 CONFIGURATION GLOBALE STATIQUE
static GameConfig g_config = {
    .current_mode = GAME_MODE_NONE,
    .ai_difficulty = AI_DIFFICULTY_MEDIUM,
    .player1_name = "Joueur 1",
    .player2_name = "Joueur 2",
    .player1_avatar = AVATAR_WARRIOR,
    .player2_avatar = AVATAR_STRATEGIST,
    .player1_piece_color = PIECE_COLOR_WHITE,  // 🆕 Défaut: blanc
    .player2_piece_color = PIECE_COLOR_BLACK,  // 🆕 Défaut: noir
    .player1_configured = false,
    .player2_configured = false,
    .ai_plays_as_white = false,
    .sound_enabled = true,
    .animations_enabled = true
};

// 🔧 FIX: Move utility functions BEFORE first use
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

// 🔧 REMOVED: piece_color_to_string - now in pions.c only

// 🆕 INITIALISATION DE LA CONFIGURATION
void config_init(void) {
    printf("Initialisation de la configuration du jeu\n");
    
    printf("   Mode par defaut: %s\n", config_mode_to_string(g_config.current_mode));
    printf("   Difficulte IA par defaut: %s\n", config_difficulty_to_string(g_config.ai_difficulty));
    printf("   Joueur 1: %s\n", g_config.player1_name);
    printf("   Joueur 2: %s\n", g_config.player2_name);
    printf("   IA joue en: %s\n", g_config.ai_plays_as_white ? "Blanc" : "Noir");
    
    printf("Configuration initialisee\n");
}

// 🆕 GESTION DU MODE DE JEU
void config_set_mode(GameMode mode) {
    GameMode old_mode = g_config.current_mode;
    g_config.current_mode = mode;
    
    printf("Mode de jeu change: %s -> %s\n", 
           config_mode_to_string(old_mode), 
           config_mode_to_string(mode));
    
    // Ajustements automatiques selon le mode
    switch (mode) {
        case GAME_MODE_LOCAL_MULTIPLAYER:
            printf("   Mode multijoueur local active\n");
            printf("   Deux joueurs humains sur le meme PC\n");
            break;
            
        case GAME_MODE_ONLINE_MULTIPLAYER:
            printf("   Mode multijoueur en ligne active\n");
            printf("   Connexion reseau requise\n");
            break;
            
        case GAME_MODE_VS_AI:
            printf("   Mode contre IA active\n");
            printf("   Difficulte: %s\n", config_difficulty_to_string(g_config.ai_difficulty));
            printf("   IA joue en: %s\n", g_config.ai_plays_as_white ? "Blanc" : "Noir");
            break;
            
        case GAME_MODE_NONE:
            printf("   Aucun mode selectionne\n");
            break;
    }
}

GameMode config_get_mode(void) {
    return g_config.current_mode;
}

// 🆕 GESTION DE LA DIFFICULTÉ IA
void config_set_ai_difficulty(AIDifficulty difficulty) {
    AIDifficulty old_difficulty = g_config.ai_difficulty;
    g_config.ai_difficulty = difficulty;
    
    printf("Difficulte IA changee: %s -> %s\n", 
           config_difficulty_to_string(old_difficulty),
           config_difficulty_to_string(difficulty));
    printf("   Niveau enregistre dans la configuration globale\n");
}

AIDifficulty config_get_ai_difficulty(void) {
    return g_config.ai_difficulty;
}

// 🆕 GESTION DES NOMS DE JOUEURS
void config_set_player_names(const char* player1, const char* player2) {
    if (!player1 || !player2) return;
    
    // 🔧 FIX: Safe bounded copy with null termination guarantee
    strncpy(g_config.player1_name, player1, sizeof(g_config.player1_name) - 1);
    g_config.player1_name[sizeof(g_config.player1_name) - 1] = '\0';
    
    strncpy(g_config.player2_name, player2, sizeof(g_config.player2_name) - 1);
    g_config.player2_name[sizeof(g_config.player2_name) - 1] = '\0';
    
    // 🆕 LOG: Verify sizes
    printf("📏 [CONFIG] player1_name: '%s' (len=%zu, max=%zu)\n", 
           g_config.player1_name, strlen(g_config.player1_name), sizeof(g_config.player1_name));
    printf("📏 [CONFIG] player2_name: '%s' (len=%zu, max=%zu)\n", 
           g_config.player2_name, strlen(g_config.player2_name), sizeof(g_config.player2_name));
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
    printf("IA configuree pour jouer en: %s\n", ai_plays_white ? "Blanc" : "Noir");
}

bool config_is_ai_white(void) {
    return g_config.ai_plays_as_white;
}

// 🆕 ACCÈS À LA CONFIGURATION COMPLÈTE
GameConfig* config_get_current(void) {
    return &g_config;
}

// 🆕 FONCTION RAPIDE pour activer le mode IA
void config_enable_ai_mode(void) {
    config_set_mode(GAME_MODE_VS_AI);
    printf("Mode IA active rapidement avec configuration par defaut\n");
}

// 🆕 IMPLÉMENTATION DU SYSTÈME D'AVATARS
// 🆕 Mapper ID → Nom de fichier
const char* avatar_id_to_filename(AvatarID id) {
    switch (id) {
        case AVATAR_WARRIOR: return "p1.png";
        case AVATAR_STRATEGIST: return "p2.png";
        case AVATAR_DIPLOMAT: return "p3.png";
        case AVATAR_EXPLORER: return "p4.png";
        case AVATAR_MERCHANT: return "p5.png";
        case AVATAR_SAGE: return "p6.png";
        default: return "p1.png";
    }
}

AvatarID avatar_filename_to_id(const char* filename) {
    if (!filename) return AVATAR_WARRIOR;
    
    if (strcmp(filename, "p1.png") == 0) return AVATAR_WARRIOR;
    if (strcmp(filename, "p2.png") == 0) return AVATAR_STRATEGIST;
    if (strcmp(filename, "p3.png") == 0) return AVATAR_DIPLOMAT;
    if (strcmp(filename, "p4.png") == 0) return AVATAR_EXPLORER;
    if (strcmp(filename, "p5.png") == 0) return AVATAR_MERCHANT;
    if (strcmp(filename, "p6.png") == 0) return AVATAR_SAGE;
    
    return AVATAR_WARRIOR;
}

// 🆕 GESTION DES AVATARS
void config_set_player_avatars(AvatarID avatar1, AvatarID avatar2) {
    g_config.player1_avatar = avatar1;
    g_config.player2_avatar = avatar2;
    printf("🎭 Avatars configurés: J1=%d, J2=%d\n", avatar1, avatar2);
}

AvatarID config_get_player1_avatar(void) {
    return g_config.player1_avatar;
}

AvatarID config_get_player2_avatar(void) {
    return g_config.player2_avatar;
}

void config_set_player1_full_profile(const char* name, AvatarID avatar) {
    if (name) {
        strncpy(g_config.player1_name, name, sizeof(g_config.player1_name) - 1);
        g_config.player1_name[sizeof(g_config.player1_name) - 1] = '\0';
    }
    g_config.player1_avatar = avatar;
    g_config.player1_configured = true;
    printf("Profil Joueur 1 configure: '%s' (Avatar %d)\n", name, avatar);
}

void config_set_player2_full_profile(const char* name, AvatarID avatar) {
    if (name) {
        strncpy(g_config.player2_name, name, sizeof(g_config.player2_name) - 1);
        g_config.player2_name[sizeof(g_config.player2_name) - 1] = '\0';
    }
    g_config.player2_avatar = avatar;
    g_config.player2_configured = true;
    printf("Profil Joueur 2 configure: '%s' (Avatar %d)\n", name, avatar);
}

// 🆕 NOUVELLES FONCTIONS pour vérifier l'état
bool config_is_player1_configured(void) {
    return g_config.player1_configured;
}

bool config_is_player2_configured(void) {
    return g_config.player2_configured;
}

// 🔧 FIX: Return true if profile form should show J2 (J1 done + local multiplayer)
// ⚠️ NOTE: This is for PROFILE SCENE ONLY, not game turn logic
bool config_is_profile_player2_turn(void) {
    bool result = g_config.current_mode == GAME_MODE_LOCAL_MULTIPLAYER &&
                  g_config.player1_configured && !g_config.player2_configured;
    printf("🔍 [CONFIG] is_profile_player2_turn() = %s (mode=%d, J1=%s, J2=%s)\n",
           result ? "TRUE" : "FALSE",
           g_config.current_mode,
           g_config.player1_configured ? "conf" : "NOT",
           g_config.player2_configured ? "conf" : "NOT");
    return result;
}

void config_reset_player_configs(void) {
    g_config.player1_configured = false;
    g_config.player2_configured = false;
    printf("Flags de configuration J1/J2 reinitialises\n");
}

// 🆕 GESTION DES COULEURS DE PIÈCES
void config_set_player_piece_colors(PieceColor player1_color, PieceColor player2_color) {
    g_config.player1_piece_color = player1_color;
    g_config.player2_piece_color = player2_color;
    
    printf("Couleurs de pieces configurees:\n");
    printf("   Joueur 1 (%s): %s\n", g_config.player1_name, piece_color_to_string(player1_color));
    printf("   Joueur 2 (%s): %s\n", g_config.player2_name, piece_color_to_string(player2_color));
}

PieceColor config_get_player1_piece_color(void) {
    return g_config.player1_piece_color;
}

PieceColor config_get_player2_piece_color(void) {
    return g_config.player2_piece_color;
}

// 🆕 FONCTIONS POUR LE MODE RÉSEAU
void config_set_network_role(bool is_invite) {
    g_config.invite_on_game = is_invite;
    printf("Role reseau defini: %s\n", is_invite ? "INVITE" : "HOTE");
}

bool config_is_network_invite(void) {
    return g_config.invite_on_game;
}
