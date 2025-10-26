#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"
#include "../utils/asset_manager.h"
#include "../plateau/plateau.h"
#include "../types.h"
#include "../config.h"
#include "../pions/pions.h"
#include "../logic/logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// === PLATEAU COMPONENT ===

// Dimensions du plateau visuelles
#define PLATEAU_VISUAL_WIDTH 480
#define PLATEAU_VISUAL_HEIGHT 320
#define PLATEAU_MARGIN 30
#define INTERSECTION_RADIUS 8
#define PIECE_RADIUS 12
#define LINE_THICKNESS 2

// Couleurs
#define PLATEAU_BG_R 139
#define PLATEAU_BG_G 115
#define PLATEAU_BG_B 85
#define LINE_COLOR_R 0
#define LINE_COLOR_G 0
#define LINE_COLOR_B 0

// === VISUAL FEEDBACK STRUCTURES ===
typedef struct VisualFeedbackState {
    int hovered_intersection;
    int selected_intersection;
    int* valid_destinations;
    int valid_count;
    float animation_timer;
    char error_message[128];
    float error_display_timer;
    int error_type;
} VisualFeedbackState;

typedef struct PlateauRenderData {
    Board* board;
    SDL_Renderer* renderer;
    int offset_x;
    int offset_y;
    int cell_width;
    int cell_height;
    bool show_intersections;
    bool show_coordinates;
    GamePlayer* player1;
    GamePlayer* player2;
    SDL_Texture* texture_black;
    SDL_Texture* texture_brown;
    VisualFeedbackState* visual_state;
    void* game_logic;
    void* intersection_elements[NODES];
} PlateauRenderData;

// === PLACEHOLDER IMPLEMENTATIONS ===
static void animate_piece_move(PlateauRenderData* data, int from_id, int to_id) {
    (void)data;
    printf("🔄 Animate piece move: %d -> %d\n", from_id, to_id);
}

static void animate_piece_selection(PlateauRenderData* data, int piece_id) {
    (void)data;
    printf("🎯 Animate piece selection: %d\n", piece_id);
}

static void animate_piece_capture(PlateauRenderData* data, int piece_id) {
    (void)data;
    printf("💥 Animate piece capture: %d\n", piece_id);
}

static void animate_piece_placement(PlateauRenderData* data, int intersection_id) {
    (void)data;
    printf("📍 Animate piece placement: %d\n", intersection_id);
}

static void animate_victory_dance(PlateauRenderData* data, Player winning_player) {
    (void)data;
    printf("🎉 Animate victory dance for player %d\n", winning_player);
}

static void animate_defeat_fade(PlateauRenderData* data, Player losing_player) {
    (void)data;
    printf("😞 Animate defeat fade for player %d\n", losing_player);
}

static void animate_initial_piece_wave(PlateauRenderData* data) {
    (void)data;
    printf("🌊 Animate initial piece wave\n");
}

// === SIMPLIFIED PLATEAU CREATION ===
UINode* ui_plateau_container_with_players(UITree* tree, const char* id, GamePlayer* player1, GamePlayer* player2) {
    if (!tree) return NULL;
    
    UINode* plateau_container = ui_div(tree, id);
    if (!plateau_container) return NULL;
    
    SET_SIZE(plateau_container, PLATEAU_VISUAL_WIDTH, PLATEAU_VISUAL_HEIGHT);
    atomic_set_background_color(plateau_container->element, PLATEAU_BG_R, PLATEAU_BG_G, PLATEAU_BG_B, 255);
    
    PlateauRenderData* data = calloc(1, sizeof(PlateauRenderData));
    if (data) {
        data->player1 = player1;
        data->player2 = player2;
        atomic_set_custom_data(plateau_container->element, "plateau_data", data);
    }
    
    return plateau_container;
}

// === PUBLIC API FUNCTIONS ===
void ui_plateau_set_players(UINode* plateau, GamePlayer* player1, GamePlayer* player2) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->player1 = player1;
        data->player2 = player2;
    }
}

GamePlayer* ui_plateau_get_player1(UINode* plateau) {
    if (!plateau) return NULL;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    return data ? data->player1 : NULL;
}

GamePlayer* ui_plateau_get_player2(UINode* plateau) {
    if (!plateau) return NULL;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    return data ? data->player2 : NULL;
}

void ui_plateau_set_game_logic(UINode* plateau, void* game_logic) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) data->game_logic = game_logic;
}

void* ui_plateau_get_game_logic(UINode* plateau) {
    if (!plateau) return NULL;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    return data ? data->game_logic : NULL;
}

void ui_plateau_cleanup(UINode* plateau) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) free(data);
}

void ui_plateau_container_destroy(UINode* plateau_container) {
    if (plateau_container) ui_plateau_cleanup(plateau_container);
}

// Animation API functions
void ui_plateau_animate_piece_move(UINode* plateau, int from_id, int to_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_move(data, from_id, to_id);
}

void ui_plateau_animate_piece_capture(UINode* plateau, int piece_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_capture(data, piece_id);
}

void ui_plateau_animate_piece_placement(UINode* plateau, int intersection_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_placement(data, intersection_id);
}

void ui_plateau_animate_piece_selection(UINode* plateau, int piece_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_selection(data, piece_id);
}

void ui_plateau_animate_victory_dance(UINode* plateau, int winning_player) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_victory_dance(data, (Player)winning_player);
}

void ui_plateau_animate_defeat_fade(UINode* plateau, int losing_player) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_defeat_fade(data, (Player)losing_player);
}

void ui_plateau_animate_initial_wave(UINode* plateau) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_initial_piece_wave(data);
}