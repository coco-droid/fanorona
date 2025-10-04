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

// 🆕 CONFIGURATION GLOBALE DU JEU
typedef struct {
    GameMode current_mode;
    AIDifficulty ai_difficulty;
    char player1_name[64];
    char player2_name[64];
    bool ai_plays_as_white;    // True si l'IA joue les blancs
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

// 🆕 FONCTIONS UTILITAIRES
const char* config_mode_to_string(GameMode mode);
const char* config_difficulty_to_string(AIDifficulty difficulty);

#endif // FANORONA_CONFIG_H