#ifndef FANORONA_CONFIG_H
#define FANORONA_CONFIG_H

#include <stdbool.h>
#include "types.h"  // 🔧 FIX: Import types instead of redefining

// Tailles par défaut centralisées
#define DEFAULT_MAIN_WINDOW_WIDTH  950
#define DEFAULT_MAIN_WINDOW_HEIGHT 500
#define DEFAULT_MINI_WINDOW_WIDTH  700
#define DEFAULT_MINI_WINDOW_HEIGHT 500

// 🆕 CONFIGURATION GLOBALE DU JEU
typedef struct {
    GameMode current_mode;
    AIDifficulty ai_difficulty;
    char player1_name[128];
    char player2_name[128];
    AvatarID player1_avatar;
    AvatarID player2_avatar;
    PieceColor player1_piece_color;    // 🆕 Couleur des pièces J1
    PieceColor player2_piece_color;    // 🆕 Couleur des pièces J2
    bool player1_configured;
    bool player2_configured;
    bool ai_plays_as_white;
    bool sound_enabled;
    bool animations_enabled;
} GameConfig;

// 🆕 FONCTIONS DE CONFIGURATION
void config_init(void);
void config_set_mode(GameMode mode);
GameMode config_get_mode(void);
void config_set_ai_difficulty(AIDifficulty difficulty);
AIDifficulty config_get_ai_difficulty(void);
void config_set_player_names(const char* player1, const char* player2);
const char* config_get_player1_name(void);
const char* config_get_player2_name(void);
void config_set_ai_color(bool ai_plays_white);
bool config_is_ai_white(void);
GameConfig* config_get_current(void);

// 🆕 FONCTIONS POUR LES AVATARS
void config_set_player_avatars(AvatarID avatar1, AvatarID avatar2);
AvatarID config_get_player1_avatar(void);
AvatarID config_get_player2_avatar(void);
void config_set_player1_full_profile(const char* name, AvatarID avatar);
void config_set_player2_full_profile(const char* name, AvatarID avatar);

// 🆕 FONCTIONS POUR LES COULEURS DE PIÈCES
void config_set_player_piece_colors(PieceColor player1_color, PieceColor player2_color);
PieceColor config_get_player1_piece_color(void);
PieceColor config_get_player2_piece_color(void);

// 🆕 FONCTIONS UTILITAIRES
const char* config_mode_to_string(GameMode mode);
const char* config_difficulty_to_string(AIDifficulty difficulty);

// 🔧 FIX: Forward declare, implemented in pions.c
extern const char* piece_color_to_string(PieceColor color);

// 🆕 FONCTION RAPIDE pour activer le mode IA
void config_enable_ai_mode(void);

// 🆕 NOUVELLES FONCTIONS pour vérifier l'état
bool config_is_player1_configured(void);
bool config_is_player2_configured(void);

// 🔧 FIX: Check if profile scene should show J2 form (NOT game turn)
bool config_is_profile_player2_turn(void);
void config_reset_player_configs(void);

// 🆕 Mapper ID → Nom de fichier
const char* avatar_id_to_filename(AvatarID id);

#endif // FANORONA_CONFIG_H