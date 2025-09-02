#pragma once
#include "fanorona.h"
#include <stdbool.h>

typedef struct {
    Pos from, to;
    int captured_count;
    Pos captured_pieces[20]; // Max possible captures
} Move;

typedef struct {
    GameState state;
    Move *move_history;
    int move_count;
    int move_capacity;
    bool game_over;
    int winner; // 0=draw, 1=white, 2=black
    double time_per_player[2];
    double time_remaining[2];
} GameManager;

GameManager *game_manager_create(void);
void game_manager_destroy(GameManager *gm);
bool game_manager_make_move(GameManager *gm, Pos from, Pos to);
void game_manager_undo_move(GameManager *gm);
bool game_manager_can_undo(const GameManager *gm);
void game_manager_reset(GameManager *gm);
Move *game_manager_get_valid_moves(const GameManager *gm, int *count);
void game_manager_update_timers(GameManager *gm, double dt);
