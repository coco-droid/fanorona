#include "ai.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// === CONSTANTES MARKOV ===
#define MARKOV_DEFAULT_TABLE_SIZE 4096
#define MARKOV_MIN_FREQUENCY 5
#define MARKOV_LEARNING_RATE 0.1f
#define MARKOV_EXPLORATION_FACTOR 0.2f

// === FONCTIONS UTILITAIRES MARKOV ===

// Créer un modèle de chaînes de Markov
MarkovModel* markov_create_model(int table_size) {
    if (table_size <= 0) table_size = MARKOV_DEFAULT_TABLE_SIZE;
    
    MarkovModel* model = (MarkovModel*)calloc(1, sizeof(MarkovModel));
    if (!model) {
        printf("Impossible d'allouer MarkovModel\n");
        return NULL;
    }
    
    model->transition_table = (MarkovTransition**)calloc(table_size, sizeof(MarkovTransition*));
    if (!model->transition_table) {
        printf("Impossible d'allouer la table de transitions\n");
        free(model);
        return NULL;
    }
    
    model->table_size = table_size;
    model->total_transitions = 0;
    model->learning_games = 0;
    model->learning_mode = true;
    
    printf("Modèle Markov créé avec table de %d entrées\n", table_size);
    return model;
}

// Détruire le modèle Markov
void markov_destroy_model(MarkovModel* model) {
    if (!model) return;
    
    printf("Destruction du modèle Markov (%d transitions)\n", model->total_transitions);
    
    // Libérer toutes les chaînes de transitions
    for (int i = 0; i < model->table_size; i++) {
        MarkovTransition* current = model->transition_table[i];
        while (current) {
            MarkovTransition* next = current->next;
            free(current);
            current = next;
        }
    }
    
    free(model->transition_table);
    free(model);
}

// Déterminer l'état du jeu (ouverture/milieu/fin)
MarkovGameState markov_determine_game_state(Board* board) {
    int total_pieces = ai_count_pieces(board, WHITE) + ai_count_pieces(board, BLACK);
    
    if (total_pieces >= 35) {
        return MARKOV_STATE_OPENING;    // > 35 pièces = ouverture
    } else if (total_pieces >= 15) {
        return MARKOV_STATE_MIDGAME;    // 15-35 pièces = milieu
    } else {
        return MARKOV_STATE_ENDGAME;    // < 15 pièces = fin
    }
}

// Calculer le hash Zobrist d'une position
uint64_t markov_hash_position(Board* board) {
    uint64_t hash = 0;
    
    for (int i = 0; i < NODES; i++) {
        Piece* piece = board->nodes[i].piece;
        if (piece && piece->alive) {
            int color_index = (piece->owner == WHITE) ? 0 : 1;
            hash ^= g_zobrist.piece_keys[i][color_index];
        }
    }
    
    return hash;
}

// Extraire un pattern de position
PositionPattern markov_extract_pattern(Board* board, Move last_move) {
    PositionPattern pattern = {0};
    
    pattern.position_hash = markov_hash_position(board);
    pattern.game_state = markov_determine_game_state(board);
    pattern.piece_count[0] = ai_count_pieces(board, WHITE);
    pattern.piece_count[1] = ai_count_pieces(board, BLACK);
    pattern.last_move_type = last_move.is_capture;
    
    return pattern;
}

// Calculer l'index de hash pour la table
static int markov_hash_to_index(MarkovModel* model, uint64_t hash) {
    return (int)(hash % model->table_size);
}

// Ajouter une transition au modèle
void markov_add_transition(MarkovModel* model, PositionPattern from, Move move, PositionPattern to) {
    if (!model || !model->learning_mode) return;
    
    int index = markov_hash_to_index(model, from.position_hash);
    
    // Chercher si la transition existe déjà
    MarkovTransition* current = model->transition_table[index];
    while (current) {
        if (current->from_pattern.position_hash == from.position_hash &&
            current->transition_move.from_id == move.from_id &&
            current->transition_move.to_id == move.to_id) {
            
            // Transition existante : incrémenter la fréquence
            current->frequency++;
            current->probability = current->frequency / (float)(model->learning_games + 1);
            return;
        }
        current = current->next;
    }
    
    // Nouvelle transition
    MarkovTransition* new_transition = (MarkovTransition*)malloc(sizeof(MarkovTransition));
    if (!new_transition) return;
    
    new_transition->from_pattern = from;
    new_transition->transition_move = move;
    new_transition->to_pattern = to;
    new_transition->frequency = 1;
    new_transition->probability = 1.0f / (model->learning_games + 1);
    new_transition->next = model->transition_table[index];
    
    model->transition_table[index] = new_transition;
    model->total_transitions++;
    
    if (model->total_transitions % 1000 == 0) {
        printf("Modèle Markov: %d transitions apprises\n", model->total_transitions);
    }
}

// Obtenir la probabilité d'un coup
float markov_get_move_probability(MarkovModel* model, PositionPattern pattern, Move move) {
    if (!model) return 0.0f;
    
    int index = markov_hash_to_index(model, pattern.position_hash);
    MarkovTransition* current = model->transition_table[index];
    
    while (current) {
        if (current->from_pattern.position_hash == pattern.position_hash &&
            current->transition_move.from_id == move.from_id &&
            current->transition_move.to_id == move.to_id) {
            
            return current->probability;
        }
        current = current->next;
    }
    
    return 0.0f; // Coup jamais vu
}

// Apprendre d'une partie complète
void markov_learn_from_game(MarkovModel* model, Move* game_moves, int move_count, Player winner) {
    if (!model || !game_moves || move_count < 2) return;
    
    printf("Apprentissage Markov d'une partie de %d coups (gagnant: %s)\n", 
           move_count, winner == WHITE ? "Blanc" : "Noir");
    
    // Simuler la partie pour extraire les patterns
    Board temp_board;
    board_init(&temp_board);
    
    PositionPattern last_pattern = markov_extract_pattern(&temp_board, game_moves[0]);
    
    for (int i = 0; i < move_count; i++) {
        // Appliquer le coup
        apply_move(&temp_board, &game_moves[i]);
        
        // Extraire le nouveau pattern
        PositionPattern new_pattern = markov_extract_pattern(&temp_board, game_moves[i]);
        
        // Ajouter la transition
        markov_add_transition(model, last_pattern, game_moves[i], new_pattern);
        
        last_pattern = new_pattern;
    }
    
    model->learning_games++;
    board_free(&temp_board);
    
    printf("Apprentissage terminé (%d parties au total)\n", model->learning_games);
}

// Trouver le meilleur coup avec Markov
Move markov_find_best_move(AIEngine* ai, Board* board) {
    if (!ai || !ai->markov_data) {
        printf("Données Markov manquantes\n");
        Move invalid_move = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid_move;
    }
    
    MarkovModel* model = (MarkovModel*)ai->markov_data;
    
    // Générer tous les coups possibles
    Move possible_moves[MAX_MOVES];
    int move_count = generate_moves(board, ai->ai_player, possible_moves, MAX_MOVES);
    
    if (move_count == 0) {
        Move invalid_move = {.from_id = -1, .to_id = -1, .is_capture = 0, .capture_count = 0};
        return invalid_move;
    }
    
    // Extraire le pattern actuel
    Move dummy_move = {.from_id = 0, .to_id = 0, .is_capture = 0, .capture_count = 0};
    PositionPattern current_pattern = markov_extract_pattern(board, dummy_move);
    
    // Évaluer chaque coup selon les probabilités Markov
    float best_score = -1.0f;
    Move best_move = possible_moves[0];
    
    printf("Évaluation Markov de %d coups possibles...\n", move_count);
    
    for (int i = 0; i < move_count; i++) {
        float move_probability = markov_get_move_probability(model, current_pattern, possible_moves[i]);
        
        // Bonus pour les captures
        float capture_bonus = possible_moves[i].is_capture ? 0.3f : 0.0f;
        
        // Score combiné
        float total_score = move_probability + capture_bonus;
        
        // Factor d'exploration pour les coups non vus
        if (move_probability == 0.0f) {
            total_score = MARKOV_EXPLORATION_FACTOR;
        }
        
        if (total_score > best_score) {
            best_score = total_score;
            best_move = possible_moves[i];
        }
        
        printf("   Coup %d→%d: prob=%.3f, capture=%d, score=%.3f\n", 
               possible_moves[i].from_id, possible_moves[i].to_id, 
               move_probability, possible_moves[i].is_capture, total_score);
    }
    
    printf("Meilleur coup Markov: %d→%d (score: %.3f)\n", 
           best_move.from_id, best_move.to_id, best_score);
    
    ai->moves_evaluated = move_count;
    return best_move;
}

// === SAUVEGARDE/CHARGEMENT ===

bool markov_save_model(MarkovModel* model, const char* filename) {
    if (!model || !filename) return false;
    
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Impossible d'ouvrir %s pour écriture\n", filename);
        return false;
    }
    
    // Écrire l'en-tête
    fwrite(&model->table_size, sizeof(int), 1, file);
    fwrite(&model->total_transitions, sizeof(int), 1, file);
    fwrite(&model->learning_games, sizeof(int), 1, file);
    
    // Écrire toutes les transitions
    int saved_transitions = 0;
    for (int i = 0; i < model->table_size; i++) {
        MarkovTransition* current = model->transition_table[i];
        while (current) {
            fwrite(current, sizeof(MarkovTransition), 1, file);
            saved_transitions++;
            current = current->next;
        }
    }
    
    fclose(file);
    
    printf("Modèle Markov sauvegardé: %d transitions dans %s\n", 
           saved_transitions, filename);
    return true;
}

MarkovModel* markov_load_model(const char* filename) {
    if (!filename) return NULL;
    
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Fichier %s non trouvé, création d'un nouveau modèle\n", filename);
        return markov_create_model(MARKOV_DEFAULT_TABLE_SIZE);
    }
    
    // Lire l'en-tête
    int table_size, total_transitions, learning_games;
    fread(&table_size, sizeof(int), 1, file);
    fread(&total_transitions, sizeof(int), 1, file);
    fread(&learning_games, sizeof(int), 1, file);
    
    // Créer le modèle
    MarkovModel* model = markov_create_model(table_size);
    if (!model) {
        fclose(file);
        return NULL;
    }
    
    model->total_transitions = 0; // Sera incrémenté lors du chargement
    model->learning_games = learning_games;
    
    // Charger toutes les transitions
    MarkovTransition temp_transition;
    int loaded_transitions = 0;
    
    while (fread(&temp_transition, sizeof(MarkovTransition), 1, file) == 1) {
        // Reconstituer la transition dans la table
        int index = markov_hash_to_index(model, temp_transition.from_pattern.position_hash);
        
        MarkovTransition* new_transition = (MarkovTransition*)malloc(sizeof(MarkovTransition));
        if (new_transition) {
            *new_transition = temp_transition;
            new_transition->next = model->transition_table[index];
            model->transition_table[index] = new_transition;
            model->total_transitions++;
            loaded_transitions++;
        }
    }
    
    fclose(file);
    
    printf("Modèle Markov chargé: %d transitions, %d parties d'apprentissage\n", 
           loaded_transitions, learning_games);
    
    return model;
}
