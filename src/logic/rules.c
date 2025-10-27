#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rules.h"

// Helper: check contiguous enemy pieces along vector (vr,vc) starting at start_id
int collect_captures_along(Board *b, int start_id, int vr, int vc, Player enemy, int out_ids[], int *out_count) {
    int r,c;
    rc_from_id(start_id, &r, &c);
    int cnt = 0;
    int rr = r, cc = c;
    while (1) {
        rr += vr; cc += vc;
        if (!in_bounds(rr, cc)) break;
        int nid = node_id(rr, cc);
        Piece *p = b->nodes[nid].piece;
        if (!p) break;
        if (p->owner != enemy) break;
        out_ids[cnt++] = nid;
        if (cnt >= MAX_CAPTURE_LIST) break;
    }
    *out_count = cnt;
    return cnt;
}

// Determine type of capture for a proposed move (from_id -> to_id).
int detect_capture(Board *b, int from_id, int to_id, Move *mv) {
    mv->from_id = from_id;
    mv->to_id = to_id;
    mv->capture_count = 0;
    mv->is_capture = 0;

    int fr, fc, tr, tc;
    rc_from_id(from_id, &fr, &fc);
    rc_from_id(to_id, &tr, &tc);
    int vr = tr - fr;
    int vc = tc - fc;

    // movement must be to adjacent node
    if (abs(vr) > 1 || abs(vc) > 1) return 0;
    if (vr == 0 && vc == 0) return 0;

    // determine owner at origin
    Piece *origin_piece = b->nodes[from_id].piece;
    if (!origin_piece) return 0;
    Player me = origin_piece->owner;
    Player enemy = (me == WHITE) ? BLACK : WHITE;

    // Percussion: check immediate after destination in same vector
    int percussion_capture_ids[MAX_CAPTURE_LIST];
    int percussion_count = 0;
    collect_captures_along(b, to_id, vr, vc, enemy, percussion_capture_ids, &percussion_count);
    if (percussion_count > 0) {
        mv->is_capture = 1;
        for (int i = 0; i < percussion_count; ++i) mv->captured_ids[mv->capture_count++] = percussion_capture_ids[i];
        return 1;
    }

    // Aspiration: check before origin in opposite vector
    int aspiration_capture_ids[MAX_CAPTURE_LIST];
    int aspiration_count = 0;
    collect_captures_along(b, from_id, -vr, -vc, enemy, aspiration_capture_ids, &aspiration_count);
    if (aspiration_count > 0) {
        mv->is_capture = 1;
        for (int i = 0; i < aspiration_count; ++i) mv->captured_ids[mv->capture_count++] = aspiration_capture_ids[i];
        return 2;
    }

    return 0;
}

// Generate all legal moves for given player
int generate_moves(Board *b, Player player, Move out_moves[], int max_moves) {
    int moves = 0;
    Move captures[MAX_MOVES];
    int capture_count = 0;

    // scan all nodes for player's pieces
    for (int id = 0; id < NODES; ++id) {
        Piece *pc = b->nodes[id].piece;
        if (!pc) continue;
        if (pc->owner != player) continue;

        Intersection *it = &b->nodes[id];
        // test each neighbor
        for (int ni = 0; ni < it->nnei; ++ni) {
            int nid = it->neighbors[ni];
            // must move into empty neighbor
            if (b->nodes[nid].piece != NULL) continue;
            Move temp;
            if (detect_capture(b, id, nid, &temp)) {
                // record capture move
                if (capture_count < MAX_MOVES) captures[capture_count++] = temp;
            }
        }
    }

    if (capture_count > 0) {
        // return only captures
        int ret = (capture_count < max_moves) ? capture_count : max_moves;
        for (int i = 0; i < ret; ++i) out_moves[i] = captures[i];
        return ret;
    }

    // Else, generate simple paika moves
    for (int id = 0; id < NODES; ++id) {
        Piece *pc = b->nodes[id].piece;
        if (!pc) continue;
        if (pc->owner != player) continue;

        Intersection *it = &b->nodes[id];
        for (int ni = 0; ni < it->nnei; ++ni) {
            int nid = it->neighbors[ni];
            if (b->nodes[nid].piece != NULL) continue;
            if (moves < max_moves) {
                Move m;
                m.from_id = id;
                m.to_id = nid;
                m.capture_count = 0;
                m.is_capture = 0;
                out_moves[moves++] = m;
            }
        }
    }
    return moves;
}

// Apply a move: move piece, remove captured pieces
void apply_move(Board *b, const Move *mv) {
    int from = mv->from_id;
    int to = mv->to_id;
    Piece *pc = b->nodes[from].piece;
    if (!pc) {
        fprintf(stderr, "apply_move: no piece at from=%d\n", from);
        return;
    }
    // move piece
    b->nodes[to].piece = pc;
    b->nodes[from].piece = NULL;
    int tr, tc; rc_from_id(to, &tr, &tc);
    pc->r = tr; pc->c = tc;

    // remove captured pieces
    if (mv->is_capture && mv->capture_count > 0) {
        for (int i = 0; i < mv->capture_count; ++i) {
            int cid = mv->captured_ids[i];
            Piece *cap = b->nodes[cid].piece;
            if (cap) {
                cap->alive = 0;
                // free memory and clear pointer
                free(cap);
                b->nodes[cid].piece = NULL;
            }
        }
    }
}

// Small helper to print a single move for debug
void print_move(Board *b, const Move *m) {
    (void)b; // Suppress unused parameter warning
    int fr,fc,tr,tc;
    rc_from_id(m->from_id, &fr, &fc);
    rc_from_id(m->to_id, &tr, &tc);
    if (m->is_capture)
        printf("MOVE (capture) from (%d,%d) -> (%d,%d) captured=%d\n", fr,fc,tr,tc, m->capture_count);
    else
        printf("MOVE (paika) from (%d,%d) -> (%d,%d)\n", fr,fc,tr,tc);
}

// 🆕 Helper: Check if any capture move is available for the player
int has_any_capture_available(Board *b, Player player) {
    for (int id = 0; id < NODES; ++id) {
        Piece *pc = b->nodes[id].piece;
        if (!pc || pc->owner != player) continue;

        Intersection *it = &b->nodes[id];
        for (int ni = 0; ni < it->nnei; ++ni) {
            int nid = it->neighbors[ni];
            if (b->nodes[nid].piece != NULL) continue; // must be empty
            
            Move temp;
            if (detect_capture(b, id, nid, &temp)) {
                return 1; // Found at least one capture
            }
        }
    }
    return 0; // No captures available
}

// 🆕 Helper: Check if position was already visited during capture chain
int is_position_visited(int position_id, int *visited_positions, int visited_count) {
    for (int i = 0; i < visited_count; ++i) {
        if (visited_positions[i] == position_id) {
            return 1;
        }
    }
    return 0;
}

// 🆕 Helper: Check if two directions are equal
int directions_equal(Direction *d1, Direction *d2) {
    if (!d1 || !d2) return 0;
    return (d1->vr == d2->vr && d1->vc == d2->vc);
}

// 🆕 Main move validation function
int is_move_valide(Board *b, int from_id, int to_id, Player player, 
                   Direction *last_direction, int *visited_positions, int visited_count, 
                   int during_capture) {
    
    // Basic validation
    if (!b || from_id < 0 || from_id >= NODES || to_id < 0 || to_id >= NODES) {
        return 0;
    }
    
    // 1. Verify piece ownership at origin
    Piece *piece = b->nodes[from_id].piece;
    if (!piece || piece->owner != player) {
        return 0;
    }
    
    // 2. Verify destination is empty
    if (b->nodes[to_id].piece != NULL) {
        return 0;
    }
    
    // 3. Verify adjacency (to_id must be neighbor of from_id)
    Intersection *from_intersection = &b->nodes[from_id];
    int is_adjacent = 0;
    for (int i = 0; i < from_intersection->nnei; ++i) {
        if (from_intersection->neighbors[i] == to_id) {
            is_adjacent = 1;
            break;
        }
    }
    if (!is_adjacent) {
        return 0;
    }
    
    // 4. Calculate movement direction vector
    int fr, fc, tr, tc;
    rc_from_id(from_id, &fr, &fc);
    rc_from_id(to_id, &tr, &tc);
    Direction current_direction = {tr - fr, tc - fc};
    
    // 5. During capture chain: check restrictions
    if (during_capture) {
        // 5a. Cannot revisit same position
        if (is_position_visited(to_id, visited_positions, visited_count)) {
            return 0;
        }
        
        // 5b. Cannot capture twice in same direction consecutively
        if (last_direction && directions_equal(&current_direction, last_direction)) {
            return 0;
        }
    }
    
    // 6. Detect if this move would be a capture
    Move temp_move;
    int is_capture_move = detect_capture(b, from_id, to_id, &temp_move);
    
    // 7. Check capture obligation rules
    if (!during_capture) {
        // If not in capture chain, check if any captures are available
        int captures_available = has_any_capture_available(b, player);
        
        if (captures_available && !is_capture_move) {
            // Capture is available but player wants to do paika - INVALID
            return 0;
        }
        
        if (!captures_available && !is_capture_move) {
            // No captures available, paika is allowed - VALID
            return 1;
        }
        
        if (is_capture_move) {
            // Capture move when captures are available - VALID
            return 1;
        }
    } else {
        // During capture chain
        if (is_capture_move) {
            // Continue capture chain - VALID
            return 1;
        } else {
            // End capture chain with paika - VALID (player can choose to stop)
            return 1;
        }
    }
    
    return 1; // Default valid
}

// 🆕 Count alive pieces for a player
int count_alive_pieces(Board *b, Player player) {
    int count = 0;
    for (int id = 0; id < NODES; ++id) {
        Piece *pc = b->nodes[id].piece;
        if (pc && pc->owner == player && pc->alive) {
            count++;
        }
    }
    return count;
}

// 🆕 Check if player has any legal move (capture or paika)
int has_any_legal_move(Board *b, Player player) {
    Move temp_moves[MAX_MOVES];
    int move_count = generate_moves(b, player, temp_moves, MAX_MOVES);
    return move_count > 0;
}

// 🆕 Check game over conditions and return winner (or NOBODY if ongoing/draw)
Player check_game_over(Board *b) {
    int white_count = count_alive_pieces(b, WHITE);
    int black_count = count_alive_pieces(b, BLACK);
    
    // Condition 1: Un joueur n'a plus de pions
    if (white_count == 0) return BLACK;
    if (black_count == 0) return WHITE;
    
    // Condition 2: Un joueur ne peut plus bouger
    int white_can_move = has_any_legal_move(b, WHITE);
    int black_can_move = has_any_legal_move(b, BLACK);
    
    if (!white_can_move && !black_can_move) return NOBODY; // Match nul
    if (!white_can_move) return BLACK;
    if (!black_can_move) return WHITE;
    
    return NOBODY; // Partie continue
}
