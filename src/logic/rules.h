#ifndef RULES_H
#define RULES_H

#include "../plateau/plateau.h"

#define MAX_CAPTURE_LIST 45
#define MAX_MOVES 512

// Move representation
typedef struct Move {
    int from_id;
    int to_id;
    int capture_count;
    int captured_ids[MAX_CAPTURE_LIST]; // list of node ids of captured pieces
    int is_capture; // 0 = paika, 1 = capture
} Move;

// Game logic functions
int collect_captures_along(Board *b, int start_id, int vr, int vc, Player enemy, int out_ids[], int *out_count);
int detect_capture(Board *b, int from_id, int to_id, Move *mv);
int generate_moves(Board *b, Player player, Move out_moves[], int max_moves);
void apply_move(Board *b, const Move *mv);
void print_move(Board *b, const Move *m);

#endif
