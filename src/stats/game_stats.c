#include "game_stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

GameStatsManager* game_stats_create(void) {
    GameStatsManager* stats = (GameStatsManager*)calloc(1, sizeof(GameStatsManager));
    if (!stats) {
        printf("❌ Impossible d'allouer GameStatsManager\n");
        return NULL;
    }
    
    stats->game_active = false;
    stats->total_game_duration = 0.0f;
    stats->total_turns = 0;
    
    // Initialiser shortest_turn_time à une valeur élevée
    stats->player1_stats.shortest_turn_time = 999999.0f;
    stats->player2_stats.shortest_turn_time = 999999.0f;
    
    printf("✅ GameStatsManager créé\n");
    return stats;
}

void game_stats_destroy(GameStatsManager* stats) {
    if (stats) {
        printf("🧹 Destruction de GameStatsManager\n");
        free(stats);
    }
}

// 🆕 MISSING FUNCTIONS: Add timer management functions
void game_stats_init_player(GameStatsManager* stats, int player_number, const char* name) {
    if (!stats || player_number < 1 || player_number > 2) return;
    
    PlayerStats* player_stats = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    player_stats->player_number = player_number;
    if (name) {
        strncpy(player_stats->player_name, name, sizeof(player_stats->player_name) - 1);
        player_stats->player_name[sizeof(player_stats->player_name) - 1] = '\0';
    }
    
    // Initialize all timer values
    player_stats->total_game_time = 0.0f;
    player_stats->current_turn_time = 0.0f;
    player_stats->average_turn_time = 0.0f;
    player_stats->longest_turn_time = 0.0f;
    player_stats->shortest_turn_time = 999999.0f;
    player_stats->is_timer_running = false;
    
    printf("📊 [STATS] Player %d (%s) initialized\n", player_number, name ? name : "Unknown");
}

PlayerStats* game_stats_get_player(GameStatsManager* stats, int player_number) {
    if (!stats || player_number < 1 || player_number > 2) return NULL;
    
    return (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
}

void game_stats_start_turn_timer(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player_stats = game_stats_get_player(stats, player_number);
    if (!player_stats) return;
    
    // Reset current turn time and start timer
    player_stats->current_turn_time = 0.0f;
    player_stats->is_timer_running = true;
    
    printf("🟢 [TIMER_START] ⏱️ CHRONOMÈTRE ACTIVÉ pour %s (Joueur %d)\n", 
           player_stats->player_name, player_number);
    printf("   ⏱️ Timer réinitialisé à 00:00 et démarré\n");
    printf("   🎯 État: is_timer_running = true\n");
}

void game_stats_stop_turn_timer(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player_stats = game_stats_get_player(stats, player_number);
    if (!player_stats) return;
    
    if (player_stats->is_timer_running) {
        player_stats->is_timer_running = false;
        
        printf("🔴 [TIMER_STOP] ⏹️ CHRONOMÈTRE ARRÊTÉ pour %s (Joueur %d)\n", 
               player_stats->player_name, player_number);
        printf("   ⏱️ Temps final du tour: %.2fs\n", player_stats->current_turn_time);
        printf("   🎯 État: is_timer_running = false\n");
    }
}

void game_stats_update_timers(GameStatsManager* stats, float delta_time) {
    if (!stats) return;
    
    // Update player 1 timer if running
    if (stats->player1_stats.is_timer_running) {
        stats->player1_stats.current_turn_time += delta_time;
        stats->player1_stats.total_game_time += delta_time;
    }
    
    // Update player 2 timer if running
    if (stats->player2_stats.is_timer_running) {
        stats->player2_stats.current_turn_time += delta_time;
        stats->player2_stats.total_game_time += delta_time;
    }
    
    // Update total game duration
    stats->total_game_duration += delta_time;
}

void game_stats_reset_all(GameStatsManager* stats) {
    if (!stats) return;
    
    game_stats_init_player(stats, 1, stats->player1_stats.player_name);
    game_stats_init_player(stats, 2, stats->player2_stats.player_name);
    stats->total_game_duration = 0.0f;
    stats->total_turns = 0;
    stats->game_active = true;
    
    printf("🔄 Statistiques réinitialisées\n");
}

void game_stats_record_move(GameStatsManager* stats, int player_number, bool is_capture) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    player->total_moves++;
    stats->total_turns++;
    
    if (is_capture) {
        player->captures_made++;
    } else {
        player->paika_moves++;
    }
    
    // Recalculer le score et l'efficacité
    game_stats_calculate_score(stats, player_number);
    game_stats_calculate_efficiency(stats, player_number);
    
    printf("📈 Mouvement enregistré pour joueur %d: %s\n", 
           player_number, is_capture ? "CAPTURE" : "PAIKA");
}

void game_stats_record_capture(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    player->captures_made++;
    
    // Recalculer le score
    game_stats_calculate_score(stats, player_number);
    
    printf("💥 Capture enregistrée pour joueur %d (total: %d)\n", 
           player_number, player->captures_made);
}

void game_stats_calculate_score(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    // Score basé sur les captures + efficacité
    int capture_points = player->captures_made * 10;
    int efficiency_points = (int)(player->efficiency_rating * 2);
    
    player->current_score = capture_points + efficiency_points;
}

void game_stats_calculate_efficiency(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    if (player->total_moves == 0) {
        player->efficiency_rating = 0.0f;
        return;
    }
    
    // Efficacité basée sur le ratio captures/mouvements
    float capture_ratio = (float)player->captures_made / (float)player->total_moves;
    player->efficiency_rating = capture_ratio * 100.0f;
    
    // Limiter entre 0 et 100
    if (player->efficiency_rating > 100.0f) {
        player->efficiency_rating = 100.0f;
    }
}

float game_stats_get_current_turn_time(GameStatsManager* stats, int player_number) {
    if (!stats) return 0.0f;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    return player->current_turn_time;
}

int game_stats_get_total_moves(GameStatsManager* stats, int player_number) {
    if (!stats) return 0;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    return player->total_moves;
}

bool game_stats_save_to_file(GameStatsManager* stats, const char* filename) {
    if (!stats || !filename) return false;
    
    FILE* file = fopen(filename, "wb");
    if (!file) return false;
    
    fwrite(stats, sizeof(GameStatsManager), 1, file);
    fclose(file);
    
    printf("💾 Statistiques sauvegardées dans %s\n", filename);
    return true;
}

GameStatsManager* game_stats_load_from_file(const char* filename) {
    if (!filename) return NULL;
    
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    
    GameStatsManager* stats = malloc(sizeof(GameStatsManager));
    if (!stats) {
        fclose(file);
        return NULL;
    }
    
    if (fread(stats, sizeof(GameStatsManager), 1, file) != 1) {
        free(stats);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    printf("📂 Statistiques chargées depuis %s\n", filename);
    return stats;
}

void game_stats_print_summary(GameStatsManager* stats) {
    if (!stats) return;
    
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║                    📊 RÉSUMÉ DES STATISTIQUES            ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ Durée totale de la partie: %.1f secondes                ║\n", stats->total_game_duration);
    printf("║ Nombre total de tours: %d                                ║\n", stats->total_turns);
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ JOUEUR 1: %-20s                             ║\n", stats->player1_stats.player_name);
    printf("║   Captures: %d                                           ║\n", stats->player1_stats.captures_made);
    printf("║   Score: %d                                              ║\n", stats->player1_stats.current_score);
    printf("║   Efficacité: %.1f%%                                     ║\n", stats->player1_stats.efficiency_rating);
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ JOUEUR 2: %-20s                             ║\n", stats->player2_stats.player_name);
    printf("║   Captures: %d                                           ║\n", stats->player2_stats.captures_made);
    printf("║   Score: %d                                              ║\n", stats->player2_stats.current_score);
    printf("║   Efficacité: %.1f%%                                     ║\n", stats->player2_stats.efficiency_rating);
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
}

void game_stats_print_player(PlayerStats* stats) {
    if (!stats) return;
    
    printf("👤 %s (Joueur %d):\n", stats->player_name, stats->player_number);
    printf("   🎯 Captures: %d\n", stats->captures_made);
    printf("   🎮 Mouvements totaux: %d\n", stats->total_moves);
    printf("   📊 Score: %d\n", stats->current_score);
    printf("   ⚡ Efficacité: %.1f%%\n", stats->efficiency_rating);
    printf("   ⏱️ Temps total: %.1fs\n", stats->total_game_time);
    printf("   ⏱️ Tour actuel: %.1fs\n", stats->current_turn_time);
    printf("   🔄 Timer: %s\n", stats->is_timer_running ? "ACTIF" : "ARRÊTÉ");
}
