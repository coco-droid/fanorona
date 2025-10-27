#ifndef PLATEAU_TYPES_H
#define PLATEAU_TYPES_H

#include <stdbool.h>
#include "../plateau/plateau.h"
#include "../types.h"
#include "../logic/rules.h"

// === STRUCTURES ===
typedef struct VisualFeedbackState {
    int hovered_intersection;
    int selected_intersection;
    int* valid_destinations;
    int valid_count;
    float animation_timer;
    char error_message[128];
    float error_display_timer;
    int error_type;
    bool in_capture_chain;
    int capture_chain_from;
    Direction last_capture_direction;
    int visited_positions[MAX_VISITED_POSITIONS];
    int visited_count;
} VisualFeedbackState;

typedef struct PlateauRenderData PlateauRenderData;

// === GAME LOGIC FUNCTIONS ===
void apply_capture(PlateauRenderData* data, Move* move);
void apply_move_to_board(PlateauRenderData* data, int from_id, int to_id);
void calculate_valid_destinations(PlateauRenderData* data, int piece_id);
bool is_valid_destination(PlateauRenderData* data, int from_id, int to_id);
bool has_additional_captures(PlateauRenderData* data, int from_id);
void log_board_state(Board* board, const char* context);

// 🆕 Game over detection
bool check_and_handle_game_over(PlateauRenderData* data);

// 🆕 AI and Animation Logic
void execute_animated_move(PlateauRenderData* data, int from_id, int to_id);
bool is_ai_turn(PlateauRenderData* data);
void execute_ai_move(PlateauRenderData* data);
void update_ai_animation(PlateauRenderData* data, float delta_time);

#endif // PLATEAU_TYPES_H
