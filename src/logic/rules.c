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
// Returns: 0=no capture, 1=percussion only, 2=aspiration only, 3=both available (choice needed)
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

    // Check both capture types
    int percussion_capture_ids[MAX_CAPTURE_LIST];
    int percussion_count = 0;
    collect_captures_along(b, to_id, vr, vc, enemy, percussion_capture_ids, &percussion_count);
    
    int aspiration_capture_ids[MAX_CAPTURE_LIST];
    int aspiration_count = 0;
    collect_captures_along(b, from_id, -vr, -vc, enemy, aspiration_capture_ids, &aspiration_count);

    // CORRECTION: Handle choice between both capture types
    if (percussion_count > 0 && aspiration_count > 0) {
        // Both captures possible - return special code for UI choice
        return 3; // Caller must decide which capture type to use
    }
    
    if (percussion_count > 0) {
        mv->is_capture = 1;
        for (int i = 0; i < percussion_count; ++i) 
            mv->captured_ids[mv->capture_count++] = percussion_capture_ids[i];
        return 1; // Percussion only
    }

    if (aspiration_count > 0) {
        mv->is_capture = 1;
        for (int i = 0; i < aspiration_count; ++i) 
            mv->captured_ids[mv->capture_count++] = aspiration_capture_ids[i];
        return 2; // Aspiration only
    }

    return 0; // No capture
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
    if (!b || !mv || mv->from_id < 0 || mv->from_id >= NODES || 
        mv->to_id < 0 || mv->to_id >= NODES) {
         return;
     }
     
     Piece *pc = b->nodes[mv->from_id].piece;
     if (!pc || !pc->alive) {
         return;
     }
    
    // Move piece
    b->nodes[mv->to_id].piece = pc;
    b->nodes[mv->from_id].piece = NULL;
    int tr, tc; 
    rc_from_id(mv->to_id, &tr, &tc);
    pc->r = tr; 
    pc->c = tc;

    // Remove captured pieces
    if (mv->is_capture && mv->capture_count > 0) {
        for (int i = 0; i < mv->capture_count; ++i) {
            int cid = mv->captured_ids[i];
            Piece *cap = b->nodes[cid].piece;
            if (cap) {
                cap->alive = 0;
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

// Helper: Check if any capture move is available for the player
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

// Helper: Check if position was already visited during capture chain
int is_position_visited(int position_id, int *visited_positions, int visited_count) {
    for (int i = 0; i < visited_count; ++i) {
        if (visited_positions[i] == position_id) {
            return 1;
        }
    }
    return 0;
}

// Helper: Check if two directions are equal
int directions_equal(Direction *d1, Direction *d2) {
    if (!d1 || !d2) return 0;
    return (d1->vr == d2->vr && d1->vc == d2->vc);
}

// Main move validation function
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

// Count alive pieces for a player
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

// Check if player has any legal move (capture or paika)
int has_any_legal_move(Board *b, Player player) {
    Move temp_moves[MAX_MOVES];
    int move_count = generate_moves(b, player, temp_moves, MAX_MOVES);
    return move_count > 0;
}

// Check game over conditions and return winner (or NOBODY if ongoing/draw)
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

// AI RULE VALIDATION: Check if move follows Fanorona rules
bool ai_validate_fanorona_move(Board* board, Move* move, Player current_player) {
    if (!board || !move) return false;
    
    // Basic validation
    if (move->from_id < 0 || move->from_id >= NODES || 
        move->to_id < 0 || move->to_id >= NODES) {
        return false;
    }
    
    Piece* piece = board->nodes[move->from_id].piece;
    if (!piece || !piece->alive || piece->owner != current_player) {
        return false;
    }
    
    // Destination must be empty
    if (board->nodes[move->to_id].piece != NULL) {
        return false;
    }
    
    // Must be adjacent move
    Intersection* from = &board->nodes[move->from_id];
    bool is_adjacent = false;
    for (int i = 0; i < from->nnei; i++) {
        if (from->neighbors[i] == move->to_id) {
            is_adjacent = true;
            break;
        }
    }
    
    return is_adjacent;
}

// AI RULE UNDERSTANDING: Generate only legal moves according to Fanorona rules
int ai_generate_legal_moves(Board* board, Player player, Move* moves, int max_moves) {
    if (!board || !moves) return 0;
    
    // First check if any captures are available (mandatory captures rule)
    bool has_captures = has_any_capture_available(board, player);
    
    int move_count = 0;
    
    for (int from_id = 0; from_id < NODES && move_count < max_moves; from_id++) {
        Piece* piece = board->nodes[from_id].piece;
        if (!piece || !piece->alive || piece->owner != player) continue;
        
        Intersection* from = &board->nodes[from_id];
        for (int i = 0; i < from->nnei && move_count < max_moves; i++) {
            int to_id = from->neighbors[i];
            if (board->nodes[to_id].piece != NULL) continue;
            
            Move candidate;
            int capture_type = detect_capture(board, from_id, to_id, &candidate);
            
            // Apply Fanorona mandatory capture rule
            if (has_captures && !capture_type) {
                continue; // Skip non-capture moves when captures are available
            }
            
            if (!has_captures && !capture_type) {
                // Paika move when no captures available
                candidate.from_id = from_id;
                candidate.to_id = to_id;
                candidate.is_capture = 0;
                candidate.capture_count = 0;
            }
            
            moves[move_count++] = candidate;
        }
    }
    
    return move_count;
}

// AI NEURO-SYMBOLIC RULE VALIDATION: Check mandatory capture situation
bool ai_is_mandatory_capture_situation(Board* board, Player player) {
    return has_any_capture_available(board, player);
}

// AI NEURO-SYMBOLIC RULE VALIDATION: Check if can continue capture chain
bool ai_can_continue_capture_chain(Board* board, int position, Player player, Direction* last_direction) {
    if (!board || position < 0 || position >= NODES) return false;
    
    Intersection* from = &board->nodes[position];
    if (!from->piece || from->piece->owner != player) return false;
    
    for (int i = 0; i < from->nnei; i++) {
        int to_id = from->neighbors[i];
        if (board->nodes[to_id].piece != NULL) continue;
        
        Move test_move;
        if (detect_capture(board, position, to_id, &test_move)) {
            // Check direction restriction
            if (last_direction) {
                int fr, fc, tr, tc;
                rc_from_id(position, &fr, &fc);
                rc_from_id(to_id, &tr, &tc);
                Direction new_dir = {tr - fr, tc - fc};
                
                if (directions_equal(&new_dir, last_direction)) {
                    continue;
                }
            }
            return true;
        }
    }
    return false;
}

// SNAPSHOT: Créer une copie légère du plateau
BoardSnapshot board_create_snapshot(Board* board) {
    BoardSnapshot snapshot = {0};
    
    for (int i = 0; i < NODES; i++) {
        Piece* piece = board->nodes[i].piece;
        if (piece && piece->alive) {
            snapshot.pieces[i] = (piece->owner == WHITE) ? 0 : 1;
            snapshot.alive[i] = 1;
            if (piece->owner == WHITE) {
                snapshot.white_count++;
            } else {
                snapshot.black_count++;
            }
        } else {
            snapshot.pieces[i] = -1;
            snapshot.alive[i] = 0;
        }
    }
    
    // Hash simple (sans Zobrist pour l'instant)
    snapshot.hash = 0;
    for (int i = 0; i < NODES; i++) {
        snapshot.hash ^= ((uint64_t)snapshot.pieces[i] << (i % 64));
    }
    
    return snapshot;
}

// SNAPSHOT: Appliquer un coup sur le snapshot (simulation légère)
void board_apply_move_to_snapshot(BoardSnapshot* snapshot, const Move* move) {
    if (!snapshot || !move) return;
    
    // Déplacer la pièce
    int8_t piece_color = snapshot->pieces[move->from_id];
    snapshot->pieces[move->to_id] = piece_color;
    snapshot->pieces[move->from_id] = -1;
    snapshot->alive[move->to_id] = 1;
    snapshot->alive[move->from_id] = 0;
    
    // Supprimer les pièces capturées
    if (move->is_capture) {
        for (int i = 0; i < move->capture_count; i++) {
            int cap_id = move->captured_ids[i];
            if (snapshot->alive[cap_id]) {
                int8_t captured_color = snapshot->pieces[cap_id];
                snapshot->pieces[cap_id] = -1;
                snapshot->alive[cap_id] = 0;
                
                if (captured_color == 0) {
                    snapshot->white_count--;
                } else if (captured_color == 1) {
                    snapshot->black_count--;
                }
            }
        }
    }
    
    // Recalculer hash
    snapshot->hash = 0;
    for (int i = 0; i < NODES; i++) {
        snapshot->hash ^= ((uint64_t)snapshot->pieces[i] << (i % 64));
    }
}

// SNAPSHOT: Restaurer le Board depuis un snapshot (pour validation finale uniquement)
bool board_restore_from_snapshot(Board* board, const BoardSnapshot* snapshot) {
    (void)board;
    (void)snapshot;
    // Note: Cette fonction n'est pas utilisée par l'IA, seulement pour debug/validation
    return false; // Non implémenté car non nécessaire
}

// IA: Générer coups légaux depuis un snapshot
int ai_get_legal_moves_for_position(BoardSnapshot* snapshot, Player player, Move* out_moves, int max_moves) {
    if (!snapshot || !out_moves) return 0;
    
    int move_count = 0;
    Move capture_moves[MAX_MOVES];
    int capture_count = 0;
    
    int8_t player_code = (player == WHITE) ? 0 : 1;
    int8_t enemy_code = (player == WHITE) ? 1 : 0;
    
    // Parcourir toutes les pièces du joueur
    for (int from_id = 0; from_id < NODES && move_count < max_moves; from_id++) {
        if (snapshot->pieces[from_id] != player_code || !snapshot->alive[from_id]) {
            continue;
        }
        
        // Récupérer les voisins depuis la structure Board statique
        extern Board* g_static_board_for_ai; // Défini dans plateau.c
        if (!g_static_board_for_ai) continue;
        
        Intersection* from = &g_static_board_for_ai->nodes[from_id];
        
        // Tester chaque voisin
        for (int ni = 0; ni < from->nnei && move_count < max_moves; ni++) {
            int to_id = from->neighbors[ni];
            
            // Destination doit être vide
            if (snapshot->pieces[to_id] != -1) continue;
            
            // Détecter capture
            int fr, fc, tr, tc;
            rc_from_id(from_id, &fr, &fc);
            rc_from_id(to_id, &tr, &tc);
            int vr = tr - fr;
            int vc = tc - fc;
            
            Move candidate = {0};
            candidate.from_id = from_id;
            candidate.to_id = to_id;
            candidate.is_capture = 0;
            candidate.capture_count = 0;
            
            // Percussion
            int percussion_ids[MAX_CAPTURE_LIST];
            int percussion_count = 0;
            int rr = tr, cc = tc;
            while (1) {
                rr += vr; cc += vc;
                if (!in_bounds(rr, cc)) break;
                int nid = node_id(rr, cc);
                if (snapshot->pieces[nid] != enemy_code || !snapshot->alive[nid]) break;
                percussion_ids[percussion_count++] = nid;
                if (percussion_count >= MAX_CAPTURE_LIST) break;
            }
            
            // Aspiration
            int aspiration_ids[MAX_CAPTURE_LIST];
            int aspiration_count = 0;
            rr = fr; cc = fc;
            while (1) {
                rr -= vr; cc -= vc;
                if (!in_bounds(rr, cc)) break;
                int nid = node_id(rr, cc);
                if (snapshot->pieces[nid] != enemy_code || !snapshot->alive[nid]) break;
                aspiration_ids[aspiration_count++] = nid;
                if (aspiration_count >= MAX_CAPTURE_LIST) break;
            }
            
            // Choisir le type de capture avec le plus de captures
            if (percussion_count > 0 || aspiration_count > 0) {
                candidate.is_capture = 1;
                if (percussion_count >= aspiration_count) {
                    for (int i = 0; i < percussion_count; i++) {
                        candidate.captured_ids[candidate.capture_count++] = percussion_ids[i];
                    }
                } else {
                    for (int i = 0; i < aspiration_count; i++) {
                        candidate.captured_ids[candidate.capture_count++] = aspiration_ids[i];
                    }
                }
                
                if (capture_count < MAX_MOVES) {
                    capture_moves[capture_count++] = candidate;
                }
            } else {
                // Paika (si pas de captures disponibles)
                if (move_count < max_moves) {
                    out_moves[move_count++] = candidate;
                }
            }
        }
    }
    
    // Si des captures existent, retourner SEULEMENT les captures
    if (capture_count > 0) {
        int ret = (capture_count < max_moves) ? capture_count : max_moves;
        for (int i = 0; i < ret; i++) {
            out_moves[i] = capture_moves[i];
        }
        return ret;
    }
    
    return move_count;
}

// IA: Évaluation simple d'un snapshot
int ai_evaluate_snapshot(BoardSnapshot* snapshot, Player player) {
    if (!snapshot) return 0;
    
    int score = 0;
    int my_pieces = (player == WHITE) ? snapshot->white_count : snapshot->black_count;
    int opp_pieces = (player == WHITE) ? snapshot->black_count : snapshot->white_count;
    
    // Matériel (100 points par pièce)
    score += (my_pieces - opp_pieces) * 100;
    
    // Position centrale (5 points)
    for (int r = 1; r < ROWS-1; r++) {
        for (int c = 1; c < COLS-1; c++) {
            int id = node_id(r, c);
            if (snapshot->alive[id]) {
                int8_t piece_color = snapshot->pieces[id];
                int8_t my_color = (player == WHITE) ? 0 : 1;
                if (piece_color == my_color) {
                    score += 5;
                } else {
                    score -= 5;
                }
            }
        }
    }
    
    return score;
}

// IA: Vérifier game over sur snapshot
bool ai_is_game_over_snapshot(BoardSnapshot* snapshot) {
    if (!snapshot) return true;
    
    // Un joueur n'a plus de pièces
    if (snapshot->white_count == 0 || snapshot->black_count == 0) {
        return true;
    }
    
    // Vérifier si au moins un joueur peut bouger
    Move test_moves[MAX_MOVES];
    int white_moves = ai_get_legal_moves_for_position(snapshot, WHITE, test_moves, MAX_MOVES);
    int black_moves = ai_get_legal_moves_for_position(snapshot, BLACK, test_moves, MAX_MOVES);
    
    return (white_moves == 0 && black_moves == 0);
}

// IA: Validation stricte d'un coup sur snapshot
bool ai_validate_move_strict(BoardSnapshot* snapshot, const Move* move, Player player) {
    if (!snapshot || !move) return false;
    
    int8_t player_code = (player == WHITE) ? 0 : 1;
    
    // Vérifier que la pièce appartient au joueur
    if (snapshot->pieces[move->from_id] != player_code) {
        return false;
    }
    
    // Destination doit être vide
    if (snapshot->pieces[move->to_id] != -1) {
        return false;
    }
    
    // Vérifier si le coup est dans la liste des coups légaux
    Move legal_moves[MAX_MOVES];
    int legal_count = ai_get_legal_moves_for_position(snapshot, player, legal_moves, MAX_MOVES);
    
    for (int i = 0; i < legal_count; i++) {
        if (legal_moves[i].from_id == move->from_id && legal_moves[i].to_id == move->to_id) {
            return true;
        }
    }
    
    return false;
}
