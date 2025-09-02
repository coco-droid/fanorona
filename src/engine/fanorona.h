#pragma once
#include <stdbool.h>

typedef enum { EMPTY, WHITE, BLACK } Cell;
typedef struct { int x, y; } Pos;
typedef struct {
    Cell board[9][5];
    int  current_player;
} GameState;

bool game_move_valid(const GameState *g, Pos from, Pos to);
void game_apply_move(GameState *g, Pos from, Pos to);
bool game_is_terminal(const GameState *g, int *winner);