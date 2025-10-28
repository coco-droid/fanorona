#include "pions.h"
#include "../stats/game_stats.h"  // 🔧 FIX: Add missing include for complete PlayerStats definition
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === FONCTIONS POUR LES PIONS ===

Piece* piece_create(int id, Player owner, PieceColor color, int r, int c) {
    Piece* piece = (Piece*)calloc(1, sizeof(Piece));
    if (!piece) {
        printf("❌ Impossible d'allouer la mémoire pour le pion %d\n", id);
        return NULL;
    }
    
    piece->id = id;
    piece->owner = owner;
    piece->color = color;
    piece->r = r;
    piece->c = c;
    piece->alive = 1;
    
    // État visuel par défaut
    piece->is_selected = false;
    piece->is_hovered = false;
    piece->is_highlighted = false;
    
    // Textures initialisées à NULL
    piece->texture = NULL;
    piece->hover_texture = NULL;
    piece->selected_texture = NULL;
    
    printf("✅ Pion %d créé: %s %s en (%d,%d)\n", 
           id, piece_color_to_string(color), 
           owner == WHITE ? "Blanc" : "Noir", r, c);
    
    return piece;
}

void piece_destroy(Piece* piece) {
    if (piece) {
        printf("🧹 Destruction du pion %d\n", piece->id);
        free(piece);
    }
}

void piece_set_textures(Piece* piece, SDL_Texture* normal, SDL_Texture* hover, SDL_Texture* selected) {
    if (!piece) return;
    
    piece->texture = normal;
    piece->hover_texture = hover;
    piece->selected_texture = selected;
    
    printf("🎨 Textures définies pour le pion %d\n", piece->id);
}

void piece_set_position(Piece* piece, int r, int c) {
    if (!piece) return;
    
    piece->r = r;
    piece->c = c;
    printf("📍 Pion %d déplacé en (%d,%d)\n", piece->id, r, c);
}

void piece_set_visual_state(Piece* piece, bool selected, bool hovered, bool highlighted) {
    if (!piece) return;
    
    piece->is_selected = selected;
    piece->is_hovered = hovered;
    piece->is_highlighted = highlighted;
}

SDL_Texture* piece_get_current_texture(Piece* piece) {
    if (!piece) return NULL;
    
    // Priorité: sélectionné > survolé > normal
    if (piece->is_selected && piece->selected_texture) {
        return piece->selected_texture;
    } else if (piece->is_hovered && piece->hover_texture) {
        return piece->hover_texture;
    } else {
        return piece->texture;
    }
}

// === FONCTIONS POUR LES JOUEURS ===

GamePlayer* player_create(const char* name, Player logical_color, PieceColor piece_color, 
                         PlayerType type, int player_number) {
    GamePlayer* player = (GamePlayer*)calloc(1, sizeof(GamePlayer));
    if (!player) {
        printf("❌ Impossible d'allouer la mémoire pour le joueur\n");
        return NULL;
    }
    
    // Copier le nom
    if (name) {
        strncpy(player->name, name, sizeof(player->name) - 1);
        player->name[sizeof(player->name) - 1] = '\0';
    } else {
        snprintf(player->name, sizeof(player->name), "Joueur %d", player_number);
    }
    
    player->logical_color = logical_color;
    player->piece_color = piece_color;
    player->type = type;
    player->player_number = player_number;
    
    // 🔧 FIX: Initialize avatar field
    player->avatar = AVATAR_WARRIOR; // Default avatar
    
    // Initialiser les statistiques
    player->pieces_remaining = 22;
    player->captures_made = 0;
    player->total_time = 0.0f;
    player->thinking_time = 0.0f; // 🔧 FIX: Initialize thinking_time
    player->is_current_turn = false;
    player->has_mandatory_capture = false;
    
    // Paramètres par défaut
    player->ai_difficulty = 2; // Moyen
    player->ai_thinking_time = 0.0f;
    player->is_connected = false;
    player->remote_id[0] = '\0';
    
    // 🔧 FIX: Initialize stats pointer
    player->stats = NULL;
    
    printf("✅ Joueur créé: '%s' (%s %s, %s, Joueur %d)\n",
           player->name,
           logical_color == WHITE ? "Blanc" : "Noir",
           piece_color_to_string(piece_color),
           player_type_to_string(type),
           player_number);
    
    return player;
}

void player_destroy(GamePlayer* player) {
    if (player) {
        printf("🧹 Destruction du joueur '%s'\n", player->name);
        free(player);
    }
}

void player_init_human(GamePlayer* player, const char* name, Player logical_color, 
                      PieceColor piece_color, int player_number) {
    if (!player) return;
    
    if (name) {
        strncpy(player->name, name, sizeof(player->name) - 1);
        player->name[sizeof(player->name) - 1] = '\0';
    }
    
    player->logical_color = logical_color;
    player->piece_color = piece_color;
    player->type = PLAYER_TYPE_HUMAN;
    player->player_number = player_number;
    
    printf("👤 Joueur humain initialisé: '%s'\n", player->name);
}

void player_init_ai(GamePlayer* player, const char* name, Player logical_color, 
                   PieceColor piece_color, int player_number, int difficulty) {
    if (!player) return;
    
    player_init_human(player, name, logical_color, piece_color, player_number);
    player->type = PLAYER_TYPE_AI;
    player->ai_difficulty = difficulty;
    
    printf("🤖 Joueur IA initialisé: '%s' (Difficulté: %d)\n", 
           player->name, difficulty);
}

void player_init_online(GamePlayer* player, const char* name, Player logical_color, 
                       PieceColor piece_color, int player_number, const char* remote_id) {
    if (!player) return;
    
    player_init_human(player, name, logical_color, piece_color, player_number);
    player->type = PLAYER_TYPE_ONLINE;
    player->is_connected = true;
    
    if (remote_id) {
        strncpy(player->remote_id, remote_id, sizeof(player->remote_id) - 1);
        player->remote_id[sizeof(player->remote_id) - 1] = '\0';
    }
    
    printf("🌐 Joueur en ligne initialisé: '%s' (ID: %s)\n", 
           player->name, player->remote_id);
}

// === FONCTIONS UTILITAIRES ===

const char* piece_color_to_string(PieceColor color) {
    switch (color) {
        case PIECE_COLOR_WHITE: return "Blanc";
        case PIECE_COLOR_BLACK: return "Noir";
        case PIECE_COLOR_BROWN: return "Brun";
        default: return "Inconnu";
    }
}

const char* player_type_to_string(PlayerType type) {
    switch (type) {
        case PLAYER_TYPE_HUMAN:  return "Humain";
        case PLAYER_TYPE_AI:     return "IA";
        case PLAYER_TYPE_ONLINE: return "En ligne";
        default: return "Inconnu";
    }
}

void player_reset_stats(GamePlayer* player) {
    if (!player) return;
    
    player->pieces_remaining = 22;
    player->captures_made = 0;
    player->total_time = 0.0f;
    player->is_current_turn = false;
    player->has_mandatory_capture = false;
    player->ai_thinking_time = 0.0f;
    
    printf("🔄 Statistiques du joueur '%s' réinitialisées\n", player->name);
}

void player_add_capture(GamePlayer* player) {
    if (!player) return;
    
    player->captures_made++;
    printf("🎯 Capture for '%s': %d captures total\n", 
           player->name, player->captures_made);
           
    // 🆕 CRITICAL FIX: Update stats manager too
    if (player->stats) {
        player->stats->captures_made = player->captures_made;
        printf("📊 Stats updated: %s now has %d captures\n", 
               player->name, player->stats->captures_made);
    }
}

// 🆕 NEW FUNCTION: Force capture count update
void player_set_captures(GamePlayer* player, int capture_count) {
    if (!player) return;
    
    int old_count = player->captures_made;
    player->captures_made = capture_count;
    
    if (player->stats) {
        player->stats->captures_made = capture_count;
    }
    
    if (old_count != capture_count) {
        printf("🔄 [CAPTURE_UPDATE] %s: %d -> %d captures\n", 
               player->name, old_count, capture_count);
    }
}

void player_set_turn(GamePlayer* player, bool is_turn) {
    if (!player) return;
    
    player->is_current_turn = is_turn;
    if (is_turn) {
        printf("▶️ C'est au tour de '%s'\n", player->name);
    }
}
