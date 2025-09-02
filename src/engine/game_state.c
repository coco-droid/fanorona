#include "game_state.h"
#include <stdlib.h>
#include <string.h>

GameManager *game_manager_create(void) {
    GameManager *gm = malloc(sizeof(GameManager));
    memset(gm, 0, sizeof(GameManager));
    
    gm->move_capacity = 100;
    gm->move_history = malloc(sizeof(Move) * gm->move_capacity);
    gm->move_count = 0;
    gm->game_over = false;
    gm->winner = 0;
    
    // Initialize timers (10 minutes per player)
    gm->time_per_player[0] = gm->time_per_player[1] = 600.0;
    gm->time_remaining[0] = gm->time_remaining[1] = 600.0;
    
    return gm;
}

void game_manager_destroy(GameManager *gm) {
    if (!gm) return;
    free(gm->move_history);
    free(gm);
}

bool game_manager_make_move(GameManager *gm, Pos from, Pos to) {
    if (!gm || gm->game_over) return false;
    
    if (!game_move_valid(&gm->state, from, to)) return false;
    
    // Store move in history
    if (gm->move_count >= gm->move_capacity) {
        gm->move_capacity *= 2;
        gm->move_history = realloc(gm->move_history, sizeof(Move) * gm->move_capacity);
    }
    
    Move *move = &gm->move_history[gm->move_count++];
    move->from = from;
    move->to = to;
    move->captured_count = 0; // TODO: Calculate captures
    
    game_apply_move(&gm->state, from, to);
    
    // Check for game end
    gm->game_over = game_is_terminal(&gm->state, &gm->winner);
    
    return true;
}

void game_manager_undo_move(GameManager *gm) {
    if (!gm || gm->move_count == 0) return;
    // TODO: Implement move undo
    gm->move_count--;
}

bool game_manager_can_undo(const GameManager *gm) {
    return gm && gm->move_count > 0;
}

void game_manager_reset(GameManager *gm) {
    if (!gm) return;
    memset(&gm->state, 0, sizeof(GameState));
    gm->move_count = 0;
    gm->game_over = false;
    gm->winner = 0;
    gm->time_remaining[0] = gm->time_per_player[0];
    gm->time_remaining[1] = gm->time_per_player[1];
}

Move *game_manager_get_valid_moves(const GameManager *gm, int *count) {
    if (!gm || !count) return NULL;
    *count = 0;
    // TODO: Generate valid moves
    return NULL;
}

void game_manager_update_timers(GameManager *gm, double dt) {
    if (!gm || gm->game_over) return;
    
    int current = gm->state.current_player - 1;
    if (current >= 0 && current < 2) {
        gm->time_remaining[current] -= dt;
        if (gm->time_remaining[current] <= 0) {
            gm->game_over = true;
            gm->winner = (current == 0) ? 2 : 1; // Other player wins
        }
    }
}
