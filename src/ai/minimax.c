#include "ai.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

// === CONSTANTES MINIMAX ===
#define MINIMAX_INFINITY 1000000
#define MINIMAX_DEFAULT_CACHE_SIZE 64  // MB
#define QUIESCENCE_MAX_DEPTH 8
#define TT_FLAG_EXACT 0
#define TT_FLAG_LOWERBOUND 1
#define TT_FLAG_UPPERBOUND 2

// === ZOBRIST HASHING ===
ZobristKeys g_zobrist;

void zobrist_init(void) {
    srand((unsigned int)time(NULL));
    
    // Générer des clés aléatoires pour chaque position et couleur
    for (int pos = 0; pos < NODES; pos++) {
        for (int color = 0; color < 2; color++) {
            g_zobrist.piece_keys[pos][color] = 
                ((uint64_t)rand() << 32) | (uint64_t)rand();
        }
    }
    
    g_zobrist.turn_key = ((uint64_t)rand() << 32) | (uint64_t)rand();
    
    printf("🎲 Clés Zobrist initialisées\n");
}

uint64_t zobrist_hash_board(Board* board, Player turn) {
    uint64_t hash = 0;
    
    for (int i = 0; i < NODES; i++) {
        Piece* piece = board->nodes[i].piece;
        if (piece && piece->alive) {
            int color_index = (piece->owner == WHITE) ? 0 : 1;
            hash ^= g_zobrist.piece_keys[i][color_index];
        }
    }
    
    if (turn == BLACK) {
        hash ^= g_zobrist.turn_key;
    }
    
    return hash;
}

// === TABLE DE TRANSPOSITION ===

TranspositionTable* tt_create(int size_mb) {
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) return NULL;
    
    int entries = (size_mb * 1024 * 1024) / sizeof(TranspositionEntry);
    tt->table = (TranspositionEntry*)calloc(entries, sizeof(TranspositionEntry));
    if (!tt->table) {
        free(tt);
        return NULL;
    }
    
    tt->size = entries;
    tt->entries_used = 0;
    
    printf("💾 Cache de transposition créé: %d entrées (%d MB)\n", entries, size_mb);
    return tt;
}

void tt_destroy(TranspositionTable* tt) {
    if (tt) {
        free(tt->table);
        free(tt);
    }
}

void tt_clear(TranspositionTable* tt) {
    if (tt) {
        memset(tt->table, 0, tt->size * sizeof(TranspositionEntry));
        tt->entries_used = 0;
    }
}

void tt_store(TranspositionTable* tt, uint64_t hash, int depth, int score, int flag, Move best_move) {
    if (!tt) return;
    
    int index = (int)(hash % tt->size);
    TranspositionEntry* entry = &tt->table[index];
    
    // Remplacer si nouvelle entrée ou profondeur supérieure
    if (entry->hash == 0 || entry->depth <= depth) {
        if (entry->hash == 0) {
            tt->entries_used++;
        }
        
        entry->hash = hash;
        entry->depth = depth;
        entry->score = score;
        entry->flag = flag;
        entry->best_move = best_move;
    }
}

TranspositionEntry* tt_probe(TranspositionTable* tt, uint64_t hash) {
    if (!tt) return NULL;
    
    int index = (int)(hash % tt->size);
    TranspositionEntry* entry = &tt->table[index];
    
    if (entry->hash == hash) {
        return entry;
    }
    
    return NULL;
}

// === ÉVALUATION DE POSITION ===

int ai_evaluate_simple(Board* board, Player player) {
    int score = 0;
    Player opponent = (player == WHITE) ? BLACK : WHITE;
    
    // Matériel (pièces)
    int my_pieces = ai_count_pieces(board, player);
    int opp_pieces = ai_count_pieces(board, opponent);
    score += (my_pieces - opp_pieces) * 100;
    
    // Mobilité
    Move my_moves[MAX_MOVES];
    Move opp_moves[MAX_MOVES];
    int my_mobility = generate_moves(board, player, my_moves, MAX_MOVES);
    int opp_mobility = generate_moves(board, opponent, opp_moves, MAX_MOVES);
    score += (my_mobility - opp_mobility) * 10;
    
    // Positions centrales (bonus)
    for (int r = 1; r < ROWS-1; r++) {
        for (int c = 1; c < COLS-1; c++) {
            int id = node_id(r, c);
            Piece* piece = board->nodes[id].piece;
            if (piece && piece->alive) {
                if (piece->owner == player) {
                    score += 5; // Bonus pour position centrale
                } else {
                    score -= 5;
                }
            }
        }
    }
    
    // Intersections fortes (bonus)
    for (int i = 0; i < NODES; i++) {
        if (board->nodes[i].strong && board->nodes[i].piece) {
            if (board->nodes[i].piece->owner == player) {
                score += 3;
            } else {
                score -= 3;
            }
        }
    }
    
    return score;
}

PositionEvaluation ai_evaluate_position(Board* board, Player player) {
    PositionEvaluation eval = {0};
    Player opponent = (player == WHITE) ? BLACK : WHITE;
    
    // Score matériel
    eval.material_score = (ai_count_pieces(board, player) - ai_count_pieces(board, opponent)) * 100;
    
    // Score de mobilité
    eval.mobility_score = (ai_calculate_mobility(board, player) - ai_calculate_mobility(board, opponent)) * 10;
    
    // Score positionnel
    eval.position_score = 0;
    for (int i = 0; i < NODES; i++) {
        Piece* piece = board->nodes[i].piece;
        if (piece && piece->alive) {
            int pos_value = board->nodes[i].strong ? 3 : 1;
            if (piece->owner == player) {
                eval.position_score += pos_value;
            } else {
                eval.position_score -= pos_value;
            }
        }
    }
    
    // Potentiel de capture
    Move moves[MAX_MOVES];
    int move_count = generate_moves(board, player, moves, MAX_MOVES);
    eval.capture_potential = 0;
    for (int i = 0; i < move_count; i++) {
        if (moves[i].is_capture) {
            eval.capture_potential += moves[i].capture_count * 20;
        }
    }
    
    eval.total_score = eval.material_score + eval.position_score + 
                      eval.mobility_score + eval.capture_potential;
    
    return eval;
}

// === RECHERCHE QUIESCENCE ===

int minimax_quiescence_search(Board* board, int alpha, int beta, bool maximizing, Player ai_player) {
    int stand_pat = ai_evaluate_simple(board, ai_player);
    
    if (maximizing) {
        if (stand_pat >= beta) return beta;
        if (alpha < stand_pat) alpha = stand_pat;
    } else {
        if (stand_pat <= alpha) return alpha;
        if (beta > stand_pat) beta = stand_pat;
    }
    
    // Générer seulement les captures
    Move moves[MAX_MOVES];
    Player current_player = maximizing ? ai_player : (ai_player == WHITE ? BLACK : WHITE);
    int move_count = generate_moves(board, current_player, moves, MAX_MOVES);
    
    for (int i = 0; i < move_count; i++) {
        if (!moves[i].is_capture) continue; // Seulement les captures
        
        // Appliquer le coup
        apply_move(board, &moves[i]);
        
        int score = minimax_quiescence_search(board, alpha, beta, !maximizing, ai_player);
        
        // Annuler le coup (simulation simplifiée)
        // TODO: Implémenter undo_move() pour performance
        
        if (maximizing) {
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        } else {
            if (score <= alpha) return alpha;
            if (score < beta) beta = score;
        }
    }
    
    return maximizing ? alpha : beta;
}

// === ALGORITHME MINIMAX PRINCIPAL ===

int minimax_search(Board* board, int depth, int alpha, int beta, bool maximizing, Player ai_player, TranspositionTable* tt) {
    // Vérifier le cache
    uint64_t hash = zobrist_hash_board(board, maximizing ? ai_player : (ai_player == WHITE ? BLACK : WHITE));
    TranspositionEntry* cached = tt_probe(tt, hash);
    
    if (cached && cached->depth >= depth) {
        if (cached->flag == TT_FLAG_EXACT) {
            return cached->score;
        } else if (cached->flag == TT_FLAG_LOWERBOUND && cached->score >= beta) {
            return beta;
        } else if (cached->flag == TT_FLAG_UPPERBOUND && cached->score <= alpha) {
            return alpha;
        }
    }
    
    // Cas terminal
    if (depth == 0) {
        return minimax_quiescence_search(board, alpha, beta, maximizing, ai_player);
    }
    
    // Générer les coups LÉGAUX selon les règles Fanorona
    Player current_player = maximizing ? ai_player : (ai_player == WHITE ? BLACK : WHITE);
    Move moves[MAX_MOVES];
    int move_count = ai_generate_legal_moves(board, current_player, moves, MAX_MOVES);
    
    if (move_count == 0) {
        // Aucun coup possible = défaite
        return maximizing ? -MINIMAX_INFINITY : MINIMAX_INFINITY;
    }
    
    Move best_move = moves[0];
    int best_score = maximizing ? -MINIMAX_INFINITY : MINIMAX_INFINITY;
    
    for (int i = 0; i < move_count; i++) {
        // Valider le coup selon les règles Fanorona
        if (!ai_validate_fanorona_move(board, &moves[i], current_player)) {
            continue; // Skip invalid moves
        }
        
        // 🔧 FIX: Allouer sur le heap au lieu de la pile pour éviter stack overflow
        Board* temp_board = (Board*)malloc(sizeof(Board));
        if (!temp_board) continue; // Skip si allocation échoue
        
        memcpy(temp_board, board, sizeof(Board));
        
        // Deep-clone pieces
        for (int j = 0; j < NODES; j++) {
            if (board->nodes[j].piece) {
                Piece* np = (Piece*)malloc(sizeof(Piece));
                if (np) {
                    *np = *board->nodes[j].piece;
                    temp_board->nodes[j].piece = np;
                } else {
                    temp_board->nodes[j].piece = NULL;
                }
            }
        }
        
        apply_move(temp_board, &moves[i]);
        
        int score = minimax_search(temp_board, depth - 1, alpha, beta, !maximizing, ai_player, tt);
        
        // 🔧 FIX: Libérer les pièces clonées + le board heap
        for (int j = 0; j < NODES; j++) {
            if (temp_board->nodes[j].piece) {
                free(temp_board->nodes[j].piece);
            }
        }
        free(temp_board);
        
        if (maximizing) {
            if (score > best_score) {
                best_score = score;
                best_move = moves[i];
            }
            alpha = (alpha > score) ? alpha : score;
            if (beta <= alpha) break; // Élagage alpha-beta
        } else {
            if (score < best_score) {
                best_score = score;
                best_move = moves[i];
            }
            beta = (beta < score) ? beta : score;
            if (beta <= alpha) break; // Élagage alpha-beta
        }
    }
    
    // Sauvegarder dans le cache
    int flag = TT_FLAG_EXACT;
    if (best_score <= alpha) flag = TT_FLAG_UPPERBOUND;
    else if (best_score >= beta) flag = TT_FLAG_LOWERBOUND;
    
    tt_store(tt, hash, depth, best_score, flag, best_move);
    
    return best_score;
}

// === INTERFACE PUBLIQUE MINIMAX ===

Move minimax_find_best_move(AIEngine* ai, Board* board, int depth) {
    if (!ai || !board) {
        Move invalid_move = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid_move;
    }
    
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ 🤖 RECHERCHE MINIMAX - IA\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ 🎯 Profondeur: %d\n", depth);
    printf("║ 🎨 Joueur IA: %s\n", ai->ai_player == WHITE ? "Blanc" : "Noir");
    
    // 🆕 Vérifier captures obligatoires
    bool has_mandatory_captures = ai_is_mandatory_capture_situation(board, ai->ai_player);
    printf("║ ⚠️  Captures obligatoires: %s\n", has_mandatory_captures ? "OUI" : "NON");
    
    TranspositionTable* tt = (TranspositionTable*)ai->minimax_data;
    if (!tt) {
        tt = tt_create(MINIMAX_DEFAULT_CACHE_SIZE);
        ai->minimax_data = tt;
    }
    
    Move moves[MAX_MOVES];
    int move_count = generate_moves(board, ai->ai_player, moves, MAX_MOVES);
    
    if (move_count == 0) {
        printf("║ ❌ Aucun coup possible - L'IA ne peut pas jouer\n");
        printf("╚═══════════════════════════════════════════════════════════╝\n");
        Move invalid_move = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid_move;
    }
    
    printf("║ 📋 Coups générés: %d\n", move_count);
    
    // 🆕 Compter les captures disponibles
    int capture_moves = 0;
    for (int i = 0; i < move_count; i++) {
        if (moves[i].is_capture) capture_moves++;
    }
    printf("║ 💥 Dont captures: %d\n", capture_moves);
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    
    Move best_move = moves[0];
    int best_score = -MINIMAX_INFINITY;
    
    ai->moves_evaluated = 0;
    ai->cache_hits = 0;
    ai->cache_misses = 0;
    
    for (int i = 0; i < move_count; i++) {
        // 🔧 FIX: Deep copy at root to prevent original board mutation
        Board* temp_board = (Board*)malloc(sizeof(Board));
        if (!temp_board) continue;
        memcpy(temp_board, board, sizeof(Board));
        for (int j = 0; j < NODES; j++) {
            if (board->nodes[j].piece) {
                Piece* np = (Piece*)malloc(sizeof(Piece));
                if (np) {
                    *np = *board->nodes[j].piece;
                    temp_board->nodes[j].piece = np;
                }
            }
        }
        apply_move(temp_board, &moves[i]);
         
        int score = minimax_search(temp_board, depth - 1, -MINIMAX_INFINITY, MINIMAX_INFINITY, 
                                  false, ai->ai_player, tt);
        
        // Cleanup
        for (int j = 0; j < NODES; j++) {
            if (temp_board->nodes[j].piece) free(temp_board->nodes[j].piece);
        }
        free(temp_board);
        
        ai->moves_evaluated++;
        
        printf("║   %d. Coup %d→%d: score=%d%s\n", 
               i+1, moves[i].from_id, moves[i].to_id, score, 
               moves[i].is_capture ? " 💥 CAPTURE" : " 🚶 PAIKA");
        
        if (score > best_score) {
            best_score = score;
            best_move = moves[i];
        }
    }
    
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ 🎯 MEILLEUR COUP SÉLECTIONNÉ\n");
    printf("║ 📍 Mouvement: %d → %d\n", best_move.from_id, best_move.to_id);
    printf("║ 📊 Score: %d\n", best_score);
    
    // 🆕 LOGS DÉTAILLÉS DES CONSÉQUENCES DU COUP IA
    if (best_move.is_capture) {
        printf("║ 💥 TYPE: CAPTURE (%d pièce(s))\n", best_move.capture_count);
        if (best_move.capture_count > 0) {
            printf("║ 📍 IDs capturés: ");
            for (int i = 0; i < best_move.capture_count; i++) {
                printf("%d%s", best_move.captured_ids[i], 
                       (i < best_move.capture_count - 1) ? ", " : "");
            }
            printf("\n");
        }
    } else {
        printf("║ 🚶 TYPE: PAIKA (pas de capture)\n");
    }
    
    // État du plateau après simulation
    int ai_pieces = ai_count_pieces(board, ai->ai_player);
    Player opponent = (ai->ai_player == WHITE) ? BLACK : WHITE;
    int opp_pieces = ai_count_pieces(board, opponent);
    printf("║ 📊 État: IA=%d pièces, Adversaire=%d pièces\n", 
           ai_pieces, opp_pieces);
    
    printf("║ 🔢 Statistiques: %d coups évalués, cache %d/%d\n", 
           ai->moves_evaluated, ai->cache_hits, ai->cache_misses);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    return best_move;
}
