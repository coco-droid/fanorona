#include "ai.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <stdlib.h>

// === CRÉATION ET DESTRUCTION ===

AIEngine* ai_create(AIType type, AIDifficulty difficulty, Player ai_player) {
    AIEngine* ai = (AIEngine*)calloc(1, sizeof(AIEngine));
    if (!ai) {
        printf("❌ Impossible d'allouer AIEngine\n");
        return NULL;
    }
    
    ai->type = type;
    ai->difficulty = difficulty;
    ai->ai_player = ai_player;
    ai->thinking_time = 0.0f;
    
    // Définir la profondeur selon la difficulté
    switch (difficulty) {
        case AI_DIFFICULTY_EASY:   ai->max_depth = AI_DEPTH_EASY; break;
        case AI_DIFFICULTY_MEDIUM: ai->max_depth = AI_DEPTH_MEDIUM; break;
        case AI_DIFFICULTY_HARD:   ai->max_depth = AI_DEPTH_HARD; break;
        default:                   ai->max_depth = AI_DEPTH_MEDIUM; break;
    }
    
    // Initialiser les composants selon le type
    switch (type) {
        case AI_TYPE_MINIMAX:
            ai->minimax_data = tt_create(64); // 64MB cache
            break;
            
        case AI_TYPE_MARKOV:
            ai->markov_data = markov_create_model(4096);
            break;
            
        case AI_TYPE_HYBRID:
            ai->minimax_data = tt_create(32); // 32MB cache
            ai->markov_data = markov_create_model(2048);
            break;
    }
    
    // Initialiser Zobrist si pas déjà fait
    static bool zobrist_initialized = false;
    if (!zobrist_initialized) {
        zobrist_init();
        zobrist_initialized = true;
    }
    
    printf("🤖 AI Engine créée: Type=%d, Difficulté=%d, Joueur=%s, Profondeur=%d\n",
           type, difficulty, ai_player == WHITE ? "Blanc" : "Noir", ai->max_depth);
    
    return ai;
}

void ai_destroy(AIEngine* ai) {
    if (!ai) return;
    
    printf("🧹 Destruction AI Engine\n");
    
    if (ai->minimax_data) {
        tt_destroy((TranspositionTable*)ai->minimax_data);
    }
    if (ai->markov_data) {
        markov_destroy_model((MarkovModel*)ai->markov_data);
    }
    
    free(ai);
}

// === INTERFACE PRINCIPALE ===

Move ai_find_best_move(AIEngine* ai, Board* board) {
    if (!ai || !board) {
        Move invalid = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid;
    }
    
    printf("🤖 [AI_SEARCH] Recherche du meilleur coup (avec règles Fanorona)...\n");
    printf("   🎯 Type: %d, Profondeur: %d, Joueur: %s\n", 
           ai->type, ai->max_depth, ai->ai_player == WHITE ? "Blanc" : "Noir");
    
    // 🆕 NEURO-SYMBOLIC: Vérifier d'abord les règles obligatoires
    bool mandatory_capture = ai_is_mandatory_capture_situation(board, ai->ai_player);
    if (mandatory_capture) {
        printf("   ⚠️ Situation de capture obligatoire détectée\n");
    }
    
    Move best_move;
    
    switch (ai->type) {
        case AI_TYPE_MINIMAX:
            best_move = minimax_find_best_move(ai, board, ai->max_depth);
            break;
            
        case AI_TYPE_MARKOV:
            best_move = markov_find_best_move(ai, board);
            break;
            
        case AI_TYPE_HYBRID:
            best_move = ai_hybrid_find_best_move(ai, board);
            break;
            
        default:
            printf("❌ Type d'IA non supporté: %d\n", ai->type);
            best_move.from_id = -1;
            best_move.to_id = -1;
            break;
    }
    
    // 🆕 VALIDATION FINALE: S'assurer que le coup respecte les règles
    if (best_move.from_id != -1 && best_move.to_id != -1) {
        if (ai_validate_fanorona_move(board, &best_move, ai->ai_player)) {
            printf("✅ [AI_SEARCH] Coup validé selon les règles: %d → %d\n", 
                   best_move.from_id, best_move.to_id);
        } else {
            printf("❌ [AI_SEARCH] Coup invalide détecté, recherche alternative...\n");
            // Fallback: premier coup légal disponible
            Move legal_moves[MAX_MOVES];
            int legal_count = ai_generate_legal_moves(board, ai->ai_player, legal_moves, MAX_MOVES);
            if (legal_count > 0) {
                best_move = legal_moves[0];
                printf("🔄 [AI_SEARCH] Coup de secours: %d → %d\n", 
                       best_move.from_id, best_move.to_id);
            }
        }
    }
    
    return best_move;
}

// === FONCTIONS UTILITAIRES ===

int ai_count_pieces(Board* board, Player player) {
    if (!board) return 0;
    
    int count = 0;
    for (int i = 0; i < NODES; i++) {
        Piece* piece = board->nodes[i].piece;
        if (piece && piece->alive && piece->owner == player) {
            count++;
        }
    }
    return count;
}

int ai_calculate_mobility(Board* board, Player player) {
    if (!board) return 0;
    
    Move moves[MAX_MOVES];
    return generate_moves(board, player, moves, MAX_MOVES);
}

bool ai_is_endgame(Board* board) {
    int total_pieces = ai_count_pieces(board, WHITE) + ai_count_pieces(board, BLACK);
    return total_pieces < 15; // Moins de 15 pièces = fin de partie
}

void ai_print_statistics(AIEngine* ai) {
    if (!ai) return;
    
    printf("\n=== 📊 STATISTIQUES IA ===\n");
    printf("Type: %d\n", ai->type);
    printf("Difficulté: %d\n", ai->difficulty);
    printf("Profondeur max: %d\n", ai->max_depth);
    printf("Coups évalués: %d\n", ai->moves_evaluated);
    printf("Cache hits: %d\n", ai->cache_hits);
    printf("Cache misses: %d\n", ai->cache_misses);
    printf("========================\n\n");
}

// === IMPLÉMENTATION HYBRIDE BASIQUE ===

Move ai_hybrid_find_best_move(AIEngine* ai, Board* board) {
    if (!ai || !board) {
        Move invalid = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid;
    }
    
    // Simple hybride: utiliser Markov pour l'ouverture/milieu, Minimax pour la fin
    if (ai_is_endgame(board)) {
        printf("🎯 [HYBRID] Mode fin de partie: utilisation Minimax\n");
        return minimax_find_best_move(ai, board, ai->max_depth);
    } else {
        printf("🎯 [HYBRID] Mode ouverture/milieu: utilisation Markov + Minimax léger\n");
        Move markov_move = markov_find_best_move(ai, board);
        if (markov_move.from_id != -1) {
            return markov_move;
        } else {
            // Fallback sur minimax avec profondeur réduite
            return minimax_find_best_move(ai, board, ai->max_depth / 2);
        }
    }
}
