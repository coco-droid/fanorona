#include "fanorona.h"
#include <stdbool.h>

bool game_move_valid(const GameState *g, Pos from, Pos to) {
    // Basic bounds checking
    if (from.x < 0 || from.x >= 9 || from.y < 0 || from.y >= 5) return false;
    if (to.x < 0 || to.x >= 9 || to.y < 0 || to.y >= 5) return false;
    
    // Check if starting position has current player's piece
    if (g->board[from.x][from.y] != (g->current_player == 1 ? WHITE : BLACK)) return false;
    
    // Check if destination is empty
    if (g->board[to.x][to.y] != EMPTY) return false;
    
    // TODO: Add proper move validation logic
    return true;
}

void game_apply_move(GameState *g, Pos from, Pos to) {
    if (!game_move_valid(g, from, to)) return;
    
    // Move the piece
    g->board[to.x][to.y] = g->board[from.x][from.y];
    g->board[from.x][from.y] = EMPTY;
    
    // Switch player
    g->current_player = (g->current_player == 1) ? 2 : 1;
}

bool game_is_terminal(const GameState *g, int *winner) {
    // TODO: Implement proper game end detection
    (void)g; // Suppress unused warning
    *winner = 0; // Draw by default
    return false;
}
