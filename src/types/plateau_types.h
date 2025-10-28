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

// 🆕 ANIMATION SYSTEM FOR PIECES
typedef struct PieceAnimation {
    int piece_id;
    int from_intersection;
    int to_intersection;
    float start_x, start_y;     // Position de départ (pixels)
    float end_x, end_y;         // Position d'arrivée (pixels)
    float current_x, current_y; // Position actuelle (pixels)
    float progress;             // 0.0 à 1.0
    float duration;             // Durée totale en secondes
    float elapsed_time;         // Temps écoulé
    bool is_active;
    bool is_capture_move;
    int captured_pieces[MAX_CAPTURE_LIST];
    int capture_count;
    void (*on_complete)(struct PieceAnimation* anim); // Callback de fin
} PieceAnimation;

typedef struct PieceAnimationManager {
    PieceAnimation animations[10]; // Max 10 animations simultanées
    int active_count;
    bool animation_in_progress;
    float global_speed_multiplier; // 1.0 = normal, 0.5 = lent, 2.0 = rapide
} PieceAnimationManager;

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

// 🆕 PIECE ANIMATION FUNCTIONS
void piece_animation_manager_init(PieceAnimationManager* manager);
bool piece_animation_start(PlateauRenderData* data, int from_id, int to_id, bool is_capture, 
                          const int* captured_pieces, int capture_count);
void piece_animation_update(PlateauRenderData* data, float delta_time);
bool piece_animation_is_active(PlateauRenderData* data);
void piece_animation_set_speed(PlateauRenderData* data, float speed_multiplier);
void piece_animation_clear_all(PlateauRenderData* data);

// Add extern declaration for global manager access
extern PieceAnimationManager g_piece_manager;

#endif // PLATEAU_TYPES_H
