#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>
#include "../plateau/plateau.h"
#include "../config.h"
#include "../pions/pions.h"

// 🆕 ÉTAT DE JEU
typedef enum {
    GAME_STATE_MENU,           // Dans les menus
    GAME_STATE_WAITING_INPUT,  // En attente d'un coup du joueur
    GAME_STATE_AI_THINKING,    // IA en train de réfléchir
    GAME_STATE_ONLINE_WAITING, // En attente du joueur distant
    GAME_STATE_GAME_OVER,      // Partie terminée
    GAME_STATE_PAUSED          // Partie en pause
} GameState;

// 🔧 FIX: Use GamePlayer structure directly from pions.h
typedef struct {
    GameState state;
    GameMode mode;
    GamePlayer* white_player;  // Pointer vers GamePlayer
    GamePlayer* black_player;  // Pointer vers GamePlayer
    Player current_player;
    Board* board;
    int turn_number;
    bool game_finished;
    Player winner;
    float total_game_time;
} GameLogic;

// 🆕 FONCTIONS DE GESTION DE LA LOGIQUE DE JEU
GameLogic* game_logic_create(void);
void game_logic_destroy(GameLogic* logic);
void game_logic_initialize_for_mode(GameLogic* logic, GameMode mode);
void game_logic_start_new_game(GameLogic* logic);

// 🆕 FONCTIONS DE GESTION DES TOURS
bool game_logic_is_human_turn(GameLogic* logic);
bool game_logic_is_ai_turn(GameLogic* logic);
bool game_logic_is_remote_turn(GameLogic* logic);
GamePlayer* game_logic_get_current_player_info(GameLogic* logic);
GamePlayer* game_logic_get_other_player_info(GameLogic* logic);
void game_logic_switch_turn(GameLogic* logic);

// 🆕 FONCTIONS DE VALIDATION ET ÉTAT
bool game_logic_can_player_move(GameLogic* logic, Player player);
bool game_logic_is_game_over(GameLogic* logic);
Player game_logic_check_winner(GameLogic* logic);
void game_logic_update(GameLogic* logic, float delta_time);

// 🆕 FONCTIONS UTILITAIRES
const char* game_logic_state_to_string(GameState state);
const char* game_logic_player_type_to_string(PlayerType type);
void game_logic_debug_print(GameLogic* logic);

#endif // LOGIC_H
bool game_logic_is_remote_turn(GameLogic* logic);
GamePlayer* game_logic_get_current_player_info(GameLogic* logic);
GamePlayer* game_logic_get_other_player_info(GameLogic* logic);
void game_logic_switch_turn(GameLogic* logic);

// 🆕 FONCTIONS DE VALIDATION ET ÉTAT
bool game_logic_can_player_move(GameLogic* logic, Player player);
bool game_logic_is_game_over(GameLogic* logic);
Player game_logic_check_winner(GameLogic* logic);
void game_logic_update(GameLogic* logic, float delta_time);

// 🆕 FONCTIONS UTILITAIRES
const char* game_logic_state_to_string(GameState state);
const char* game_logic_player_type_to_string(PlayerType type);
void game_logic_debug_print(GameLogic* logic);


