#include "plateau_types.h"
#include "../logic/logic.h"
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Import PlateauRenderData definition
#include "../ui/ui_components.h"
#include "../ui/ui_tree.h"
#include "../ui/native/atomic.h"
#include "../window/window.h"
#include "../pions/pions.h"

// Full definition needed for logic functions
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

// Forward declaration for animation stub
static void animate_piece_capture(PlateauRenderData* data, int piece_id) {
    (void)data;
    (void)piece_id;
}

void log_board_state(Board* board, const char* context) {
    if (!board) return;
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ 📊 ÉTAT DU PLATEAU - %s\n", context);
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║     ");
    for (int c = 0; c < COLS; ++c) printf(" %d", c);
    printf("\n");
    printf("║   ┌");
    for (int c = 0; c < COLS; ++c) printf("──");
    printf("┐\n");
    for (int r = 0; r < ROWS; ++r) {
        printf("║ %d │", r);
        for (int c = 0; c < COLS; ++c) {
            int id = node_id(r, c);
            Piece* pc = board->nodes[id].piece;
            if (pc == NULL) {
                printf(" ·");
            }
            else if (pc->owner == WHITE) {
                printf(" ○");
            }
            else {
                printf(" ●");
            }
        }
        printf(" │\n");
    }
    printf("║   └");
    for (int c = 0; c < COLS; ++c) printf("──");
    printf("┘\n");
    int white_count = 0, black_count = 0;
    for (int i = 0; i < NODES; i++) {
        Piece* pc = board->nodes[i].piece;
        if (pc && pc->alive) {
            if (pc->owner == WHITE) white_count++;
            else black_count++;
        }
    }
    printf("║ 📈 Pièces: ○ Blanc=%d  ● Noir=%d\n", white_count, black_count);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
}

bool has_additional_captures(PlateauRenderData* data, int from_id) {
    if (!data || !data->board) return false;
    Intersection* from = &data->board->nodes[from_id];
    if (!from->piece) return false;
    for (int i = 0; i < from->nnei; i++) {
        int neighbor_id = from->neighbors[i];
        if (data->board->nodes[neighbor_id].piece != NULL) continue;
        Move test_move;
        if (detect_capture(data->board, from_id, neighbor_id, &test_move)) {
            if (data->visual_state && data->visual_state->in_capture_chain) {
                int fr, fc, tr, tc;
                rc_from_id(from_id, &fr, &fc);
                rc_from_id(neighbor_id, &tr, &tc);
                Direction new_dir = { tr - fr, tc - fc };
                if (directions_equal(&new_dir, &data->visual_state->last_capture_direction)) {
                    continue;
                }
                if (is_position_visited(neighbor_id, data->visual_state->visited_positions,
                    data->visual_state->visited_count)) {
                    continue;
                }
            }
            return true;
        }
    }
    return false;
}

void apply_capture(PlateauRenderData* data, Move* move) {
    if (!data || !move || !move->is_capture) return;
    printf("\n┌────────────────────────────────────────────────────────┐\n");
    printf("│ 💥 CAPTURE DÉTECTÉE\n");
    printf("├────────────────────────────────────────────────────────┤\n");
    printf("│ 📍 Mouvement: intersection %d → %d\n", move->from_id, move->to_id);
    printf("│ 🎯 Nombre de captures: %d\n", move->capture_count);
    printf("│ 📋 Pièces capturées: ");
    for (int i = 0; i < move->capture_count; i++) {
        int cap_id = move->captured_ids[i];
        Piece* captured = data->board->nodes[cap_id].piece;
        if (captured) {
            printf("%d(%s) ", cap_id, captured->owner == WHITE ? "○" : "●");
        }
    }
    printf("\n└────────────────────────────────────────────────────────┘\n");
    for (int i = 0; i < move->capture_count; i++) {
        int cap_id = move->captured_ids[i];
        Piece* captured = data->board->nodes[cap_id].piece;
        if (captured) {
            printf("   🗑️  Suppression pièce %d à l'intersection %d\n",
                captured->id, cap_id);
            captured->alive = 0;
            data->board->nodes[cap_id].piece = NULL;
            animate_piece_capture(data, cap_id);
        }
    }
}

void apply_move_to_board(PlateauRenderData* data, int from_id, int to_id) {
    if (!data || !data->board) return;
    Intersection* from = &data->board->nodes[from_id];
    Intersection* to = &data->board->nodes[to_id];
    if (!from->piece || !from->piece->alive) {
        printf("❌ [APPLY_MOVE] Pas de pièce à déplacer sur %d\n", from_id);
        return;
    }
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ 🎮 MOUVEMENT APPLIQUÉ\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ 📍 Pièce %d: intersection %d → %d\n", from->piece->id, from_id, to_id);
    printf("║ 🎨 Propriétaire: %s\n", from->piece->owner == WHITE ? "○ Blanc" : "● Noir");
    Move move;
    int capture_type = detect_capture(data->board, from_id, to_id, &move);
    bool is_capture = move.is_capture && move.capture_count > 0;
    if (is_capture) {
        printf("║ 💥 TYPE: CAPTURE (%s)\n",
            capture_type == 1 ? "Percussion" : "Aspiration");
        apply_capture(data, &move);
        if (data->visual_state) {
            int fr, fc, tr, tc;
            rc_from_id(from_id, &fr, &fc);
            rc_from_id(to_id, &tr, &tc);
            data->visual_state->last_capture_direction.vr = tr - fr;
            data->visual_state->last_capture_direction.vc = tc - fc;
            if (data->visual_state->visited_count < MAX_VISITED_POSITIONS) {
                data->visual_state->visited_positions[data->visual_state->visited_count++] = to_id;
            }
        }
    }
    else {
        printf("║ 🚶 TYPE: PAIKA (déplacement simple)\n");
    }
    to->piece = from->piece;
    from->piece = NULL;
    to->piece->r = to->r;
    to->piece->c = to->c;
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    log_board_state(data->board, "APRÈS MOUVEMENT");
    if (is_capture) {
        bool more_captures = has_additional_captures(data, to_id);
        if (more_captures) {
            printf("\n┌────────────────────────────────────────────────────────┐\n");
            printf("│ ⚠️  CAPTURES SUPPLÉMENTAIRES DISPONIBLES\n");
            printf("│ 🔗 Chaîne de captures activée\n");
            printf("│ ⏳ Le tour continue pour le même joueur\n");
            printf("└────────────────────────────────────────────────────────┘\n\n");
            if (data->visual_state) {
                data->visual_state->in_capture_chain = true;
                data->visual_state->capture_chain_from = to_id;
                data->visual_state->selected_intersection = to_id;
                calculate_valid_destinations(data, to_id);
            }
            return;
        }
        else {
            printf("\n┌────────────────────────────────────────────────────────┐\n");
            printf("│ ✅ Aucune capture supplémentaire possible\n");
            printf("│ 🔄 Fin de la chaîne de captures\n");
            printf("└────────────────────────────────────────────────────────┘\n\n");
            if (data->visual_state) {
                data->visual_state->in_capture_chain = false;
                data->visual_state->visited_count = 0;
            }
        }
    }
    GameLogic* game_logic = (GameLogic*)data->game_logic;
    if (game_logic) {
        GamePlayer* before = game_logic_get_current_player_info(game_logic);
        game_logic_switch_turn(game_logic);
        GamePlayer* after = game_logic_get_current_player_info(game_logic);
        printf("\n┌────────────────────────────────────────────────────────┐\n");
        printf("│ 🔄 CHANGEMENT DE TOUR\n");
        printf("│ 👤 Avant: %s\n", before->name);
        printf("│ 👤 Après: %s\n", after->name);
        printf("└────────────────────────────────────────────────────────┘\n\n");
    }
}

void calculate_valid_destinations(PlateauRenderData* data, int piece_id) {
    if (!data || !data->visual_state || !data->board) return;
    if (data->visual_state->valid_destinations) {
        free(data->visual_state->valid_destinations);
        data->visual_state->valid_destinations = NULL;
    }
    data->visual_state->valid_count = 0;
    Intersection* intersection = &data->board->nodes[piece_id];
    if (!intersection->piece || !intersection->piece->alive) return;
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ 🎯 CALCUL DES DESTINATIONS VALIDES\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ 📍 Pièce à l'intersection: %d\n", piece_id);
    printf("║ 🎨 Propriétaire: %s\n",
        intersection->piece->owner == WHITE ? "○ Blanc" : "● Noir");
    printf("║ 🔗 En chaîne de captures: %s\n",
        data->visual_state->in_capture_chain ? "OUI" : "NON");
    Move possible_moves[MAX_MOVES];
    int move_count = generate_moves(data->board, intersection->piece->owner, possible_moves, MAX_MOVES);
    printf("║ 📋 Coups générés au total: %d\n", move_count);
    int* destinations = malloc(sizeof(int) * move_count);
    int dest_count = 0;
    for (int i = 0; i < move_count; i++) {
        if (possible_moves[i].from_id == piece_id) {
            if (data->visual_state->in_capture_chain) {
                int fr, fc, tr, tc;
                rc_from_id(piece_id, &fr, &fc);
                rc_from_id(possible_moves[i].to_id, &tr, &tc);
                Direction new_dir = { tr - fr, tc - fc };
                if (directions_equal(&new_dir, &data->visual_state->last_capture_direction)) {
                    printf("║   ❌ Coup %d→%d rejeté (même direction)\n",
                        piece_id, possible_moves[i].to_id);
                    continue;
                }
                if (is_position_visited(possible_moves[i].to_id,
                    data->visual_state->visited_positions,
                    data->visual_state->visited_count)) {
                    printf("║   ❌ Coup %d→%d rejeté (position déjà visitée)\n",
                        piece_id, possible_moves[i].to_id);
                    continue;
                }
            }
            bool already_added = false;
            for (int j = 0; j < dest_count; j++) {
                if (destinations[j] == possible_moves[i].to_id) {
                    already_added = true;
                    break;
                }
            }
            if (!already_added) {
                destinations[dest_count++] = possible_moves[i].to_id;
                printf("║   ✅ Destination valide: %d %s\n",
                    possible_moves[i].to_id,
                    possible_moves[i].is_capture ? "(CAPTURE)" : "(PAIKA)");
            }
        }
    }
    data->visual_state->valid_destinations = destinations;
    data->visual_state->valid_count = dest_count;
    printf("║ 🎯 Total destinations valides: %d\n", dest_count);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
}

bool is_valid_destination(PlateauRenderData* data, int from_id, int to_id) {
    (void)from_id;
    if (!data || !data->visual_state) return false;
    for (int i = 0; i < data->visual_state->valid_count; i++) {
        if (data->visual_state->valid_destinations[i] == to_id) {
            return true;
        }
    }
    return false;
}

// 🆕 Check game over and update GameLogic state + visual feedback
bool check_and_handle_game_over(PlateauRenderData* data) {
    if (!data || !data->board || !data->game_logic) return false;
    
    GameLogic* logic = (GameLogic*)data->game_logic;
    Player winner = check_game_over(data->board);
    
    if (winner != NOBODY) {
        logic->game_finished = true;
        logic->winner = winner;
        logic->state = GAME_STATE_GAME_OVER;
        
        GamePlayer* winner_player = (winner == logic->player1->logical_color) ? 
                                     logic->player1 : logic->player2;
        
        printf("\n╔═══════════════════════════════════════════════════════════╗\n");
        printf("║ 🏆 PARTIE TERMINÉE!\n");
        printf("║ 🎉 Vainqueur: %s\n", winner_player->name);
        printf("║ 📊 Pièces restantes - Blanc: %d, Noir: %d\n", 
               count_alive_pieces(data->board, WHITE),
               count_alive_pieces(data->board, BLACK));
        printf("╚═══════════════════════════════════════════════════════════╝\n");
        
        // Clear selection visual state
        if (data->visual_state) {
            data->visual_state->selected_intersection = -1;
            data->visual_state->hovered_intersection = -1;
            if (data->visual_state->valid_destinations) {
                free(data->visual_state->valid_destinations);
                data->visual_state->valid_destinations = NULL;
            }
            data->visual_state->valid_count = 0;
        }
        
        return true;
    }
    
    return false;
}
