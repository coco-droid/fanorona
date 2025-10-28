#ifndef RULES_H
#define RULES_H

#include <stdbool.h>
#include "../plateau/plateau.h"
#include "../types.h"

// 🔧 FIX: Define MAX_CAPTURE_LIST before any usage
#define MAX_CAPTURE_LIST 22  // Maximum pieces capturable in one move

// 🆕 FIX: Add missing macro definitions
#define MAX_MOVES 512  // Maximum possible moves in a position
#define MAX_VISITED_POSITIONS 32  // Maximum positions in capture chain

// Move representation
typedef struct Move {
    int from_id;
    int to_id;
    bool is_capture;
    int capture_count;
    int captured_ids[MAX_CAPTURE_LIST];
} Move;

// Direction vector for capture chaining restrictions
typedef struct Direction {
    int vr, vc; // direction vector (row, col)
} Direction;

// Game logic functions
int collect_captures_along(Board *b, int start_id, int vr, int vc, Player enemy, int out_ids[], int *out_count);
int detect_capture(Board *b, int from_id, int to_id, Move *mv);
int generate_moves(Board *b, Player player, Move out_moves[], int max_moves);
void apply_move(Board *b, const Move *mv);
void print_move(Board *b, const Move *m);

// 🆕 NEW: Move validation function
int is_move_valide(Board *b, int from_id, int to_id, Player player, 
                   Direction *last_direction, int *visited_positions, int visited_count, 
                   int during_capture);

// 🆕 Helper functions for move validation
int has_any_capture_available(Board *b, Player player);
int is_position_visited(int position_id, int *visited_positions, int visited_count);
int directions_equal(Direction *d1, Direction *d2);

// 🆕 Game over detection functions
int count_alive_pieces(Board *b, Player player);
int has_any_legal_move(Board *b, Player player);
Player check_game_over(Board *b);

// 🆕 AI NEURO-SYMBOLIC RULE VALIDATION
bool ai_validate_fanorona_move(Board* board, Move* move, Player current_player);
int ai_generate_legal_moves(Board* board, Player player, Move* moves, int max_moves);
bool ai_is_mandatory_capture_situation(Board* board, Player player);
bool ai_can_continue_capture_chain(Board* board, int position, Player player, Direction* last_direction);

// 🆕 STRUCTURE LÉGÈRE POUR L'IA (pas de pointeurs)
typedef struct BoardSnapshot {
    int8_t pieces[NODES];        // -1=vide, 0=WHITE, 1=BLACK
    uint8_t alive[NODES];        // 0=mort, 1=vivant
    int white_count;
    int black_count;
    uint64_t hash;               // Hash Zobrist de la position
} BoardSnapshot;

// 🆕 FONCTIONS POUR L'IA (interface stricte)
BoardSnapshot board_create_snapshot(Board* board);
void board_apply_move_to_snapshot(BoardSnapshot* snapshot, const Move* move);
bool board_restore_from_snapshot(Board* board, const BoardSnapshot* snapshot);

// 🆕 Génération de coups validés pour l'IA
int ai_get_legal_moves_for_position(BoardSnapshot* snapshot, Player player, Move* out_moves, int max_moves);
int ai_evaluate_snapshot(BoardSnapshot* snapshot, Player player);
bool ai_is_game_over_snapshot(BoardSnapshot* snapshot);

// 🆕 Validation stricte des coups IA
bool ai_validate_move_strict(BoardSnapshot* snapshot, const Move* move, Player player);

#endif
