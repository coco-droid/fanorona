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
            ai->minimax_data = tt_create(64);
            break;
            
        case AI_TYPE_MARKOV:
            ai->markov_data = markov_create_model(4096);
            break;
            
        case AI_TYPE_MCTS:  // 🆕 Support MCTS
            ai->mcts_data = mcts_create_context(MCTS_DEFAULT_ITERATIONS);
            break;
            
        case AI_TYPE_HYBRID:
            ai->minimax_data = tt_create(32);
            ai->markov_data = markov_create_model(2048);
            ai->mcts_data = mcts_create_context(500);  // 🆕 MCTS léger pour hybride
            break;
    }
    
    // Initialiser Zobrist si pas déjà fait
    static bool zobrist_initialized = false;
    if (!zobrist_initialized) {
        zobrist_init();
        zobrist_initialized = true;
    }
    
    printf("AI Engine creee: Type=%d, Difficulte=%d, Joueur=%s, Profondeur=%d\n",
           type, difficulty, ai_player == WHITE ? "Blanc" : "Noir", ai->max_depth);
    
    return ai;
}

void ai_destroy(AIEngine* ai) {
    if (!ai) return;
    
    printf("Destruction AI Engine\n");
    
    if (ai->minimax_data) {
        tt_destroy((TranspositionTable*)ai->minimax_data);
    }
    if (ai->markov_data) {
        markov_destroy_model((MarkovModel*)ai->markov_data);
    }
    if (ai->mcts_data) {  // 🆕 Cleanup MCTS
        mcts_destroy_context((MCTSContext*)ai->mcts_data);
    }
    
    free(ai);
}

// 🔧 WRAPPER: Interface de compatibilité
Move ai_find_best_move(AIEngine* ai, Board* board) {
    if (!ai || !board) {
        Move invalid = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid;
    }
    
    // Créer snapshot
    BoardSnapshot snapshot = board_create_snapshot(board);
    
    printf("[AI] Conversion Board -> Snapshot (W=%d, B=%d)\n", 
           snapshot.white_count, snapshot.black_count);
    
    // Rechercher sur snapshot
    return ai_find_best_move_from_snapshot(ai, &snapshot);
}

// 🆕 NOUVELLE FONCTION: Recherche sur snapshot
Move ai_find_best_move_from_snapshot(AIEngine* ai, BoardSnapshot* snapshot) {
    if (!ai || !snapshot) {
        Move invalid = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid;
    }
    
    printf("[AI_SEARCH] Recherche sur snapshot (regles Fanorona strictes)\n");
    printf("   Type: %d, Profondeur: %d, Joueur: %s\n", 
           ai->type, ai->max_depth, ai->ai_player == WHITE ? "Blanc" : "Noir");
    
    // Vérifier coups légaux disponibles
    Move legal_moves[MAX_MOVES];
    int legal_count = ai_get_legal_moves_for_position(snapshot, ai->ai_player, legal_moves, MAX_MOVES);
    
    if (legal_count == 0) {
        printf("[AI] Aucun coup legal disponible\n");
        Move invalid = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid;
    }
    
    Move best_move;
    
    // 🆕 SWITCH AMÉLIORÉ: Tous les types fonctionnent
    switch (ai->type) {
        case AI_TYPE_MINIMAX:
            best_move = minimax_find_best_move_snapshot(ai, snapshot, ai->max_depth);
            break;
            
        case AI_TYPE_MARKOV: {
            // 🔧 FIX: Markov fonctionne maintenant sur snapshot
            Board temp_board;
            board_init(&temp_board);
            // Restaurer board depuis snapshot (approximatif pour Markov legacy)
            best_move = markov_find_best_move(ai, &temp_board);
            board_free(&temp_board);
            break;
        }
            
        case AI_TYPE_MCTS:  // 🆕 MCTS pur
            best_move = mcts_find_best_move(ai, snapshot, 1000);
            break;
            
        case AI_TYPE_HYBRID:  // 🆕 HYBRIDE INTELLIGENT
            best_move = ai_hybrid_find_best_move_snapshot(ai, snapshot);
            break;
            
        default:
            printf("[AI] Type inconnu, fallback sur premier coup legal\n");
            best_move = legal_moves[0];
            break;
    }
    
    // Validation finale
    if (!ai_validate_move_strict(snapshot, &best_move, ai->ai_player)) {
        printf("[AI] Coup invalide, utilisation du premier coup legal\n");
        best_move = legal_moves[0];
    }
    
    return best_move;
}

// 🆕 HYBRIDE AMÉLIORÉ: Combine Minimax, Markov et MCTS
Move ai_hybrid_find_best_move_snapshot(AIEngine* ai, BoardSnapshot* snapshot) {
    if (!ai || !snapshot) {
        Move invalid = {.from_id = -1, .to_id = -1};
        return invalid;
    }
    
    printf("[HYBRID] Decision multi-algorithmes\n");
    
    int total_pieces = snapshot->white_count + snapshot->black_count;
    
    if (total_pieces > 30) {
        printf("   Phase ouverture: Markov\n");
        Board temp_board;
        board_init(&temp_board);
        Move move = markov_find_best_move(ai, &temp_board);
        board_free(&temp_board);
        return move;
    }
    
    if (total_pieces > 15) {
        printf("   Phase milieu: MCTS (500 it)\n");
        return mcts_find_best_move(ai, snapshot, 500);
    }
    
    printf("   Phase finale: Minimax (profondeur max)\n");
    return minimax_find_best_move_snapshot(ai, snapshot, ai->max_depth);
}

// === INTERFACE PRINCIPALE ===


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

// 🆕 HYBRIDE BASIQUE (wrapper Board → Snapshot)
Move ai_hybrid_find_best_move(AIEngine* ai, Board* board) {
    if (!ai || !board) {
        Move invalid = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid;
    }
    
    BoardSnapshot snapshot = board_create_snapshot(board);
    return ai_hybrid_find_best_move_snapshot(ai, &snapshot);
}
