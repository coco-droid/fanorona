#ifndef FANORONA_CONFIG_H
#define FANORONA_CONFIG_H

#include <stdbool.h>

// Tailles par défaut centralisées
#define DEFAULT_MAIN_WINDOW_WIDTH  800
#define DEFAULT_MAIN_WINDOW_HEIGHT 600
#define DEFAULT_MINI_WINDOW_WIDTH  700
#define DEFAULT_MINI_WINDOW_HEIGHT 500

// 🆕 MODES DE JEU
typedef enum {
    GAME_MODE_NONE = 0,           // Aucun mode sélectionné
    GAME_MODE_LOCAL_MULTIPLAYER,  // Multijoueur sur le même PC
    GAME_MODE_ONLINE_MULTIPLAYER, // Multijoueur en ligne
    GAME_MODE_VS_AI               // Contre l'IA
} GameMode;

// 🆕 NIVEAUX DE DIFFICULTÉ IA
typedef enum {
    AI_DIFFICULTY_EASY = 1,    // Facile
    AI_DIFFICULTY_MEDIUM = 2,  // Moyen
    AI_DIFFICULTY_HARD = 3     // Difficile
} AIDifficulty;

// 🆕 SYSTÈME D'AVATARS AVEC IDS FIXES
typedef enum {
    AVATAR_WARRIOR = 1,      // p1.png - Guerrier
    AVATAR_STRATEGIST = 2,   // p2.png - Stratège
    AVATAR_DIPLOMAT = 3,     // p3.png - Diplomate
    AVATAR_EXPLORER = 4,     // p4.png - Explorateur
    AVATAR_MERCHANT = 5,     // p5.png - Marchand
    AVATAR_SAGE = 6          // p6.png - Sage
} AvatarID;

// 🆕 Mapper ID → Nom de fichier
const char* avatar_id_to_filename(AvatarID id);

// 🆕 Mapper Nom de fichier → ID
AvatarID avatar_filename_to_id(const char* filename);

// 🆕 CONFIGURATION GLOBALE DU JEU
typedef struct {
    GameMode current_mode;
    AIDifficulty ai_difficulty;
    char player1_name[128];
    char player2_name[128];
    AvatarID player1_avatar;
    AvatarID player2_avatar;
    bool player1_configured;  // 🆕 J1 a terminé sa config
    bool player2_configured;  // 🆕 J2 a terminé sa config
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

// 🆕 FONCTIONS UTILITAIRES
const char* config_mode_to_string(GameMode mode);
const char* config_difficulty_to_string(AIDifficulty difficulty);

// 🆕 FONCTION RAPIDE pour activer le mode IA
void config_enable_ai_mode(void);

// 🆕 NOUVELLES FONCTIONS pour gérer les flags
bool config_is_player1_configured(void);
bool config_is_player2_configured(void);
bool config_is_profile_player2_turn(void);  // 🔧 FIX: Check if profile scene should show J2 form (NOT game turn)
void config_reset_player_configs(void);

#endif // FANORONA_CONFIG_H