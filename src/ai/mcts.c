#include "ai.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define MCTS_DEFAULT_ITERATIONS 1000
#define MCTS_EXPLORATION_CONSTANT 1.414f  // sqrt(2)
#define MCTS_SIMULATION_DEPTH 50

// Créer un nœud MCTS
static MCTSNode* mcts_create_node(BoardSnapshot* state, Move move, MCTSNode* parent, Player player) {
    MCTSNode* node = (MCTSNode*)calloc(1, sizeof(MCTSNode));
    if (!node) return NULL;
    
    node->state = *state;
    node->move_from_parent = move;
    node->parent = parent;
    node->visits = 0;
    node->total_reward = 0.0f;
    node->is_fully_expanded = false;
    node->player = player;
    node->children = NULL;
    node->num_children = 0;
    
    return node;
}

// Détruire récursivement l'arbre MCTS
static void mcts_destroy_node(MCTSNode* node) {
    if (!node) return;
    
    for (int i = 0; i < node->num_children; i++) {
        mcts_destroy_node(node->children[i]);
    }
    
    free(node->children);
    free(node);
}

// Créer contexte MCTS
MCTSContext* mcts_create_context(int max_iterations) {
    MCTSContext* ctx = (MCTSContext*)calloc(1, sizeof(MCTSContext));
    if (!ctx) return NULL;
    
    ctx->max_iterations = max_iterations > 0 ? max_iterations : MCTS_DEFAULT_ITERATIONS;
    ctx->exploration_constant = MCTS_EXPLORATION_CONSTANT;
    ctx->simulation_depth = MCTS_SIMULATION_DEPTH;
    ctx->root = NULL;
    
    return ctx;
}

// Détruire contexte MCTS
void mcts_destroy_context(MCTSContext* ctx) {
    if (!ctx) return;
    
    if (ctx->root) {
        mcts_destroy_node(ctx->root);
    }
    
    free(ctx);
}

// Calculer UCB1 pour la sélection
static float mcts_ucb1(MCTSNode* node, float exploration_constant) {
    if (node->visits == 0) return FLT_MAX;  // Nœuds non visités prioritaires
    
    float exploitation = node->total_reward / (float)node->visits;
    float exploration = exploration_constant * sqrtf(logf((float)node->parent->visits) / (float)node->visits);
    
    return exploitation + exploration;
}

// SÉLECTION: Descendre dans l'arbre selon UCB1
MCTSNode* mcts_select(MCTSNode* node) {
    while (node->is_fully_expanded && node->num_children > 0) {
        // Choisir le meilleur enfant selon UCB1
        MCTSNode* best_child = NULL;
        float best_ucb = -FLT_MAX;
        
        for (int i = 0; i < node->num_children; i++) {
            float ucb = mcts_ucb1(node->children[i], MCTS_EXPLORATION_CONSTANT);
            if (ucb > best_ucb) {
                best_ucb = ucb;
                best_child = node->children[i];
            }
        }
        
        node = best_child;
    }
    
    return node;
}

// EXPANSION: Ajouter un nouvel enfant
MCTSNode* mcts_expand(MCTSNode* node) {
    if (ai_is_game_over_snapshot(&node->state)) {
        node->is_fully_expanded = true;
        return node;
    }
    
    // Générer tous les coups légaux
    Move legal_moves[MAX_MOVES];
    int move_count = ai_get_legal_moves_for_position(&node->state, node->player, legal_moves, MAX_MOVES);
    
    if (move_count == 0) {
        node->is_fully_expanded = true;
        return node;
    }
    
    // Si pas encore d'enfants, les créer tous
    if (node->children == NULL) {
        node->children = (MCTSNode**)calloc(move_count, sizeof(MCTSNode*));
        node->num_children = move_count;
        
        for (int i = 0; i < move_count; i++) {
            BoardSnapshot child_state = node->state;
            board_apply_move_to_snapshot(&child_state, &legal_moves[i]);
            
            Player next_player = (node->player == WHITE) ? BLACK : WHITE;
            node->children[i] = mcts_create_node(&child_state, legal_moves[i], node, next_player);
        }
    }
    
    // Trouver un enfant non visité
    for (int i = 0; i < node->num_children; i++) {
        if (node->children[i]->visits == 0) {
            return node->children[i];
        }
    }
    
    // Tous les enfants visités = fully expanded
    node->is_fully_expanded = true;
    return node->children[0];  // Fallback
}

// SIMULATION: Playout aléatoire rapide
float mcts_simulate(BoardSnapshot* state, Player ai_player) {
    BoardSnapshot sim_state = *state;
    Player current_player = ai_player;
    int depth = 0;
    
    while (depth < MCTS_SIMULATION_DEPTH && !ai_is_game_over_snapshot(&sim_state)) {
        Move legal_moves[MAX_MOVES];
        int move_count = ai_get_legal_moves_for_position(&sim_state, current_player, legal_moves, MAX_MOVES);
        
        if (move_count == 0) break;
        
        // Sélection aléatoire intelligente (biais vers captures)
        int chosen_move = 0;
        int capture_count = 0;
        
        // Compter les captures
        for (int i = 0; i < move_count; i++) {
            if (legal_moves[i].is_capture) capture_count++;
        }
        
        // 70% de chance de choisir une capture si disponible
        if (capture_count > 0 && (rand() % 100) < 70) {
            int capture_index = rand() % capture_count;
            int found = 0;
            for (int i = 0; i < move_count; i++) {
                if (legal_moves[i].is_capture) {
                    if (found == capture_index) {
                        chosen_move = i;
                        break;
                    }
                    found++;
                }
            }
        } else {
            chosen_move = rand() % move_count;
        }
        
        board_apply_move_to_snapshot(&sim_state, &legal_moves[chosen_move]);
        current_player = (current_player == WHITE) ? BLACK : WHITE;
        depth++;
    }
    
    // Évaluation finale
    int ai_pieces = (ai_player == WHITE) ? sim_state.white_count : sim_state.black_count;
    int opp_pieces = (ai_player == WHITE) ? sim_state.black_count : sim_state.white_count;
    
    if (ai_pieces == 0) return 0.0f;  // Défaite
    if (opp_pieces == 0) return 1.0f;  // Victoire
    
    // Score normalisé entre 0 et 1
    float material_advantage = (float)(ai_pieces - opp_pieces) / (ai_pieces + opp_pieces);
    return 0.5f + material_advantage * 0.5f;
}

// BACKPROPAGATION: Remonter le résultat
void mcts_backpropagate(MCTSNode* node, float reward) {
    while (node != NULL) {
        node->visits++;
        node->total_reward += reward;
        node = node->parent;
        reward = 1.0f - reward;  // Inverser pour adversaire
    }
}

// RECHERCHE MCTS PRINCIPALE
Move mcts_find_best_move(AIEngine* ai, BoardSnapshot* snapshot, int iterations) {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ MONTE CARLO TREE SEARCH\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ Itérations: %d\n", iterations);
    printf("║ Joueur: %s (W=%d, B=%d)\n", 
           ai->ai_player == WHITE ? "Blanc" : "Noir",
           snapshot->white_count, snapshot->black_count);
    
    MCTSContext* ctx = mcts_create_context(iterations);
    if (!ctx) {
        Move invalid = {.from_id = -1, .to_id = -1};
        return invalid;
    }
    
    // Créer le nœud racine
    Move dummy = {0};
    ctx->root = mcts_create_node(snapshot, dummy, NULL, ai->ai_player);
    
    // BOUCLE PRINCIPALE MCTS
    for (int i = 0; i < iterations; i++) {
        // 1. SÉLECTION
        MCTSNode* node = mcts_select(ctx->root);
        
        // 2. EXPANSION
        node = mcts_expand(node);
        
        // 3. SIMULATION
        float reward = mcts_simulate(&node->state, ai->ai_player);
        
        // 4. BACKPROPAGATION
        mcts_backpropagate(node, reward);
        
        if (i % 100 == 0 && i > 0) {
            printf("║ Progression: %d/%d itérations\n", i, iterations);
        }
    }
    
    // Choisir le meilleur coup (enfant le plus visité)
    MCTSNode* best_child = NULL;
    int max_visits = -1;
    
    for (int i = 0; i < ctx->root->num_children; i++) {
        MCTSNode* child = ctx->root->children[i];
        printf("║   %d. %d→%d: visits=%d, avg_reward=%.3f%s\n",
               i+1, child->move_from_parent.from_id, child->move_from_parent.to_id,
               child->visits, child->total_reward / (child->visits + 1),
               child->move_from_parent.is_capture ? " [CAPTURE]" : " [MOVE]");
        
        if (child->visits > max_visits) {
            max_visits = child->visits;
            best_child = child;
        }
    }
    
    Move best_move = best_child ? best_child->move_from_parent : (Move){.from_id = -1, .to_id = -1};
    
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ MEILLEUR: %d → %d (visits=%d, winrate=%.1f%%)\n",
           best_move.from_id, best_move.to_id, max_visits,
           (best_child->total_reward / best_child->visits) * 100.0f);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    mcts_destroy_context(ctx);
    return best_move;
}
