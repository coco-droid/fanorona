#ifndef AI_H
#define AI_H

#include "../plateau/plateau.h"
#include "../pions/pions.h"
#include "../logic/rules.h"
#include "../config.h"
#include <stdbool.h>
#include <stdint.h>

// === TYPES D'IA ===
typedef enum {
    AI_TYPE_MINIMAX = 0,     // Minimax classique
    AI_TYPE_MARKOV = 1,      // Chaînes de Markov
    AI_TYPE_HYBRID = 2       // Combinaison Minimax + Markov
} AIType;

// === NIVEAUX DE PROFONDEUR ===
#define AI_DEPTH_EASY 2      // Facile : 2 coups d'avance
#define AI_DEPTH_MEDIUM 4    // Moyen : 4 coups d'avance
#define AI_DEPTH_HARD 6      // Difficile : 6 coups d'avance

// === STRUCTURE PRINCIPALE IA ===
typedef struct {
    AIType type;
    AIDifficulty difficulty;
    Player ai_player;        // WHITE ou BLACK
    int max_depth;           // Profondeur maximale pour minimax
    float thinking_time;     // Temps de réflexion
    
    // Statistiques
    int moves_evaluated;
    int cache_hits;
    int cache_misses;
    
    // Données spécifiques
    void* minimax_data;      // Données minimax (cache, etc.)
    void* markov_data;       // Données chaînes de Markov
} AIEngine;

// === ÉVALUATION DE POSITION ===
typedef struct {
    int material_score;      // Score basé sur le nombre de pièces
    int position_score;      // Score basé sur la position des pièces
    int mobility_score;      // Score basé sur la mobilité
    int capture_potential;   // Potentiel de capture
    int total_score;         // Score total
} PositionEvaluation;

// === FONCTIONS PRINCIPALES ===

// Création et destruction de l'IA
AIEngine* ai_create(AIType type, AIDifficulty difficulty, Player ai_player);
void ai_destroy(AIEngine* ai);

// Configuration de l'IA
void ai_set_difficulty(AIEngine* ai, AIDifficulty difficulty);
void ai_set_thinking_time(AIEngine* ai, float max_time);
void ai_reset_statistics(AIEngine* ai);

// Recherche du meilleur coup
Move ai_find_best_move(AIEngine* ai, Board* board);
Move ai_find_move_with_time_limit(AIEngine* ai, Board* board, float time_limit);

// Évaluation de position
PositionEvaluation ai_evaluate_position(Board* board, Player player);
int ai_evaluate_simple(Board* board, Player player);

// Fonctions utilitaires
bool ai_is_endgame(Board* board);
int ai_count_pieces(Board* board, Player player);
int ai_calculate_mobility(Board* board, Player player);
void ai_print_statistics(AIEngine* ai);

// === INTERFACE MINIMAX ===

// Structure pour le cache de transposition
typedef struct TranspositionEntry {
    uint64_t hash;           // Hash de la position
    int depth;               // Profondeur de recherche
    int score;               // Score évalué
    int flag;                // Type de nœud (EXACT, LOWERBOUND, UPPERBOUND)
    Move best_move;          // Meilleur coup trouvé
} TranspositionEntry;

typedef struct {
    TranspositionEntry* table;
    int size;
    int entries_used;
} TranspositionTable;

// Fonctions minimax
Move minimax_find_best_move(AIEngine* ai, Board* board, int depth);
int minimax_search(Board* board, int depth, int alpha, int beta, bool maximizing, Player ai_player, TranspositionTable* tt);
int minimax_quiescence_search(Board* board, int alpha, int beta, bool maximizing, Player ai_player);

// Gestion du cache de transposition
TranspositionTable* tt_create(int size_mb);
void tt_destroy(TranspositionTable* tt);
void tt_store(TranspositionTable* tt, uint64_t hash, int depth, int score, int flag, Move best_move);
TranspositionEntry* tt_probe(TranspositionTable* tt, uint64_t hash);
void tt_clear(TranspositionTable* tt);

// === INTERFACE CHAÎNES DE MARKOV ===

// États du jeu pour Markov
typedef enum {
    MARKOV_STATE_OPENING = 0,    // Ouverture (premiers coups)
    MARKOV_STATE_MIDGAME = 1,    // Milieu de partie
    MARKOV_STATE_ENDGAME = 2,    // Fin de partie
    MARKOV_STATE_COUNT = 3
} MarkovGameState;

// Pattern de position pour apprentissage
typedef struct {
    uint64_t position_hash;      // Hash de la position
    MarkovGameState game_state;  // État du jeu
    int piece_count[2];          // Nombre de pièces par joueur
    int last_move_type;          // Type du dernier coup (capture/paika)
} PositionPattern;

// Chaîne de transition
typedef struct MarkovTransition {
    PositionPattern from_pattern;
    Move transition_move;
    PositionPattern to_pattern;
    float probability;           // Probabilité de cette transition
    int frequency;               // Fréquence observée
    struct MarkovTransition* next;
} MarkovTransition;

// Modèle de chaînes de Markov
typedef struct {
    MarkovTransition** transition_table;
    int table_size;
    int total_transitions;
    int learning_games;          // Nombre de parties d'apprentissage
    bool learning_mode;          // Mode apprentissage actif
} MarkovModel;

// Fonctions chaînes de Markov
Move markov_find_best_move(AIEngine* ai, Board* board);
MarkovModel* markov_create_model(int table_size);
void markov_destroy_model(MarkovModel* model);

// Apprentissage
void markov_learn_from_game(MarkovModel* model, Move* game_moves, int move_count, Player winner);
void markov_add_transition(MarkovModel* model, PositionPattern from, Move move, PositionPattern to);
float markov_get_move_probability(MarkovModel* model, PositionPattern pattern, Move move);

// Génération de patterns
PositionPattern markov_extract_pattern(Board* board, Move last_move);
MarkovGameState markov_determine_game_state(Board* board);
uint64_t markov_hash_position(Board* board);

// Sauvegarde/Chargement du modèle
bool markov_save_model(MarkovModel* model, const char* filename);
MarkovModel* markov_load_model(const char* filename);

// === FONCTIONS HYBRIDES ===

// Combine minimax et Markov pour de meilleures décisions
Move ai_hybrid_find_best_move(AIEngine* ai, Board* board);
float ai_hybrid_evaluate_move(AIEngine* ai, Board* board, Move move);

// === HASH ET ZOBRIST ===

// Hashing de Zobrist pour les positions
typedef struct {
    uint64_t piece_keys[NODES][2];   // Clés pour chaque position et couleur
    uint64_t turn_key;               // Clé pour le tour actuel
} ZobristKeys;

extern ZobristKeys g_zobrist;

void zobrist_init(void);
uint64_t zobrist_hash_board(Board* board, Player turn);
uint64_t zobrist_update_hash(uint64_t hash, int from_pos, int to_pos, Player piece_color, Player turn);

// === DEBUG ET ANALYSE ===

void ai_debug_position(Board* board, Player player);
void ai_debug_evaluation(PositionEvaluation eval);
void ai_print_best_line(AIEngine* ai, Board* board, int depth);
void ai_analyze_game(Move* moves, int move_count);

#endif // AI_H
