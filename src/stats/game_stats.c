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

void game_stats_init_player(GameStatsManager* stats, int player_number, const char* name) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    player->player_number = player_number;
    strncpy(player->player_name, name, sizeof(player->player_name) - 1);
    player->total_game_time = 0.0f;
    player->current_turn_time = 0.0f;
    player->average_turn_time = 0.0f;
    player->longest_turn_time = 0.0f;
    player->shortest_turn_time = 999999.0f;
    player->total_moves = 0;
    player->captures_made = 0;
    player->paika_moves = 0;
    player->current_score = 0;
    player->efficiency_rating = 0.0f;
    player->is_timer_running = false;
    
    printf("📊 Joueur %d '%s' initialisé dans le système de stats\n", player_number, name);
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

void game_stats_start_turn_timer(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    // 🆕 NOUVEAU: Reset complet du timer du tour actuel
    player->current_turn_time = 0.0f;
    player->is_timer_running = true;
    
    // 🆕 NOUVEAU: Log détaillé de l'activation du timer
    printf("🟢 [TIMER_START] ⏱️ CHRONOMÈTRE ACTIVÉ pour %s (Joueur %d)\n", 
           player->player_name, player_number);
    printf("   ⏱️ Timer réinitialisé à 00:00 et démarré\n");
    printf("   🎯 État: is_timer_running = true\n");
    
    // 🆕 DEBUG: Vérifier que les autres joueurs n'ont pas leur timer actif
    PlayerStats* other_player = (player_number == 1) ? &stats->player2_stats : &stats->player1_stats;
    if (other_player->is_timer_running) {
        printf("🔴 [TIMER_CONFLICT] ⚠️ Timer de l'autre joueur encore actif - correction automatique\n");
        other_player->is_timer_running = false;
        printf("   ⏹️ Timer de %s arrêté automatiquement\n", other_player->player_name);
    }
}

void game_stats_stop_turn_timer(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    if (!player->is_timer_running) {
        printf("🟡 [TIMER_STOP] ⚠️ Timer déjà arrêté pour %s\n", player->player_name);
        return;
    }
    
    // 🆕 NOUVEAU: Log détaillé de l'arrêt du timer
    printf("🔴 [TIMER_STOP] ⏹️ CHRONOMÈTRE ARRÊTÉ pour %s (Joueur %d)\n", 
           player->player_name, player_number);
    printf("   ⏱️ Temps final du tour: %.2fs\n", player->current_turn_time);
    printf("   🎯 État: is_timer_running = false\n");
    
    player->is_timer_running = false;
    
    // 🆕 NOUVEAU: Enregistrer ce tour dans les statistiques
    if (player->current_turn_time > 0.0f) {
        // Mettre à jour les statistiques de temps
        if (player->current_turn_time > player->longest_turn_time) {
            player->longest_turn_time = player->current_turn_time;
        }
        
        if (player->current_turn_time < player->shortest_turn_time) {
            player->shortest_turn_time = player->current_turn_time;
        }
        
        // Recalculer la moyenne (le nombre de moves sera incrémenté ailleurs)
        if (player->total_moves > 0) {
            player->average_turn_time = player->total_game_time / (float)player->total_moves;
        }
        
        printf("⏸️ Timer arrêté pour %s - Temps de réflexion: %.2fs\n", 
               player->player_name, player->current_turn_time);
        printf("   📊 Stats: Moy=%.1fs, Max=%.1fs, Min=%.1fs\n",
               player->average_turn_time, player->longest_turn_time, 
               player->shortest_turn_time < 999999.0f ? player->shortest_turn_time : 0.0f);
    }
}

void game_stats_update_timers(GameStatsManager* stats, float delta_time) {
    if (!stats || !stats->game_active) return;
    
    stats->total_game_duration += delta_time;
    
    // 🆕 NOUVEAU: Logger les mises à jour de timer actif
    static float last_log_time = 0.0f;
    last_log_time += delta_time;
    
    // Mettre à jour le timer du joueur actif
    if (stats->player1_stats.is_timer_running) {
        stats->player1_stats.current_turn_time += delta_time;
        stats->player1_stats.total_game_time += delta_time;
        
        // Log toutes les 5 secondes
        if (last_log_time >= 5.0f) {
            printf("⏱️ [TIMER_UPDATE] %s: %.2fs (Timer actif)\n", 
                   stats->player1_stats.player_name, 
                   stats->player1_stats.current_turn_time);
        }
    }
    
    if (stats->player2_stats.is_timer_running) {
        stats->player2_stats.current_turn_time += delta_time;
        stats->player2_stats.total_game_time += delta_time;
        
        // Log toutes les 5 secondes
        if (last_log_time >= 5.0f) {
            printf("⏱️ [TIMER_UPDATE] %s: %.2fs (Timer actif)\n", 
                   stats->player2_stats.player_name, 
                   stats->player2_stats.current_turn_time);
        }
    }
    
    if (last_log_time >= 5.0f) {
        last_log_time = 0.0f;
    }
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
    
    printf("📝 Coup enregistré pour %s: Total=%d, Captures=%d\n",
           player->player_name, player->total_moves, player->captures_made);
}

void game_stats_record_capture(GameStatsManager* stats, int player_number) {
    game_stats_record_move(stats, player_number, true);
}

void game_stats_calculate_score(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    // Score = (captures * 100) + (efficacité * 50)
    player->current_score = (player->captures_made * 100);
    
    // Bonus pour rapidité (moins de 10s par coup = bonus)
    if (player->average_turn_time < 10.0f && player->total_moves > 0) {
        player->current_score += (int)(50 * (10.0f - player->average_turn_time) / 10.0f);
    }
}

void game_stats_calculate_efficiency(GameStatsManager* stats, int player_number) {
    if (!stats) return;
    
    PlayerStats* player = (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
    
    if (player->total_moves == 0) {
        player->efficiency_rating = 0.0f;
        return;
    }
    
    // Efficacité = ratio captures/coups * 100
    float capture_ratio = (float)player->captures_made / (float)player->total_moves;
    player->efficiency_rating = fminf(capture_ratio * 100.0f, 100.0f);
}

PlayerStats* game_stats_get_player(GameStatsManager* stats, int player_number) {
    if (!stats) return NULL;
    return (player_number == 1) ? &stats->player1_stats : &stats->player2_stats;
}

float game_stats_get_current_turn_time(GameStatsManager* stats, int player_number) {
    PlayerStats* player = game_stats_get_player(stats, player_number);
    return player ? player->current_turn_time : 0.0f;
}

int game_stats_get_total_moves(GameStatsManager* stats, int player_number) {
    PlayerStats* player = game_stats_get_player(stats, player_number);
    return player ? player->total_moves : 0;
}

bool game_stats_save_to_file(GameStatsManager* stats, const char* filename) {
    if (!stats || !filename) return false;
    
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("❌ Impossible d'ouvrir %s pour écriture\n", filename);
        return false;
    }
    
    fwrite(stats, sizeof(GameStatsManager), 1, file);
    fclose(file);
    
    printf("💾 Statistiques sauvegardées dans %s\n", filename);
    return true;
}

GameStatsManager* game_stats_load_from_file(const char* filename) {
    if (!filename) return NULL;
    
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("❌ Impossible d'ouvrir %s pour lecture\n", filename);
        return NULL;
    }
    
    GameStatsManager* stats = (GameStatsManager*)malloc(sizeof(GameStatsManager));
    if (!stats) {
        fclose(file);
        return NULL;
    }
    
    fread(stats, sizeof(GameStatsManager), 1, file);
    fclose(file);
    
    printf("📂 Statistiques chargées depuis %s\n", filename);
    return stats;
}

void game_stats_print_player(PlayerStats* stats) {
    if (!stats) return;
    
    printf("\n=== 📊 STATISTIQUES %s (Joueur %d) ===\n", 
           stats->player_name, stats->player_number);
    printf("⏱️ Temps:\n");
    printf("   Total: %.2fs (%.0fm %.0fs)\n", 
           stats->total_game_time,
           floorf(stats->total_game_time / 60.0f),
           fmodf(stats->total_game_time, 60.0f));
    printf("   Tour actuel: %.2fs\n", stats->current_turn_time);
    printf("   Moyenne: %.2fs\n", stats->average_turn_time);
    printf("   Plus long: %.2fs\n", stats->longest_turn_time);
    printf("   Plus court: %.2fs\n", 
           stats->shortest_turn_time < 999999.0f ? stats->shortest_turn_time : 0.0f);
    
    printf("\n🎮 Coups:\n");
    printf("   Total: %d\n", stats->total_moves);
    printf("   Captures: %d\n", stats->captures_made);
    printf("   Paika: %d\n", stats->paika_moves);
    
    printf("\n🏆 Performance:\n");
    printf("   Score: %d\n", stats->current_score);
    printf("   Efficacité: %.1f%%\n", stats->efficiency_rating);
    printf("=====================================\n");
}

void game_stats_print_summary(GameStatsManager* stats) {
    if (!stats) return;
    
    printf("\n🎮 === RÉSUMÉ DE LA PARTIE ===\n");
    printf("⏱️ Durée totale: %.2fs (%.0fm %.0fs)\n",
           stats->total_game_duration,
           floorf(stats->total_game_duration / 60.0f),
           fmodf(stats->total_game_duration, 60.0f));
    printf("🔄 Nombre de tours: %d\n", stats->total_turns);
    
    game_stats_print_player(&stats->player1_stats);
    game_stats_print_player(&stats->player2_stats);
}
