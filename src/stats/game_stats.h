#ifndef GAME_STATS_H
#define GAME_STATS_H

#include <stdbool.h>
#include "../types.h"

// === STATISTIQUES PAR JOUEUR ===
typedef struct PlayerStats {
    int player_number;              // 1 ou 2
    char player_name[64];           // Nom du joueur
    
    // Temps
    float total_game_time;          // Temps total depuis le début de la partie
    float current_turn_time;        // Temps du tour actuel (reset à chaque tour)
    float average_turn_time;        // Temps moyen par tour
    float longest_turn_time;        // Tour le plus long
    float shortest_turn_time;       // Tour le plus court
    
    // Coups
    int total_moves;                // Nombre total de coups joués
    int captures_made;              // Captures effectuées
    int paika_moves;                // Mouvements sans capture
    
    // Score
    int current_score;              // Score actuel (basé sur captures + efficacité)
    float efficiency_rating;        // Note d'efficacité (0-100)
    
    // État
    bool is_timer_running;          // Le chrono tourne-t-il ?
} PlayerStats;

// === GESTIONNAIRE DE STATISTIQUES ===
typedef struct GameStatsManager {
    PlayerStats player1_stats;
    PlayerStats player2_stats;
    float total_game_duration;      // Durée totale de la partie
    int total_turns;                // Nombre de tours total
    bool game_active;               // Partie en cours ?
} GameStatsManager;

// === FONCTIONS PRINCIPALES ===

// Création et destruction
GameStatsManager* game_stats_create(void);
void game_stats_destroy(GameStatsManager* stats);

// Initialisation des joueurs
void game_stats_init_player(GameStatsManager* stats, int player_number, const char* name);
void game_stats_reset_all(GameStatsManager* stats);

// Gestion du timer
void game_stats_start_turn_timer(GameStatsManager* stats, int player_number);
void game_stats_stop_turn_timer(GameStatsManager* stats, int player_number);

// Enregistrement des actions
void game_stats_record_move(GameStatsManager* stats, int player_number, bool is_capture);
void game_stats_record_capture(GameStatsManager* stats, int player_number);

// Calcul du score
void game_stats_calculate_score(GameStatsManager* stats, int player_number);
void game_stats_calculate_efficiency(GameStatsManager* stats, int player_number);

// Accesseurs
PlayerStats* game_stats_get_player(GameStatsManager* stats, int player_number);
void game_stats_update_timers(GameStatsManager* stats, float delta_time);
float game_stats_get_current_turn_time(GameStatsManager* stats, int player_number);
int game_stats_get_total_moves(GameStatsManager* stats, int player_number);

// Persistance
bool game_stats_save_to_file(GameStatsManager* stats, const char* filename);
GameStatsManager* game_stats_load_from_file(const char* filename);

// Debug
void game_stats_print_summary(GameStatsManager* stats);
void game_stats_print_player(PlayerStats* stats);

#endif // GAME_STATS_H
