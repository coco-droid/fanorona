#ifndef PIONS_H
#define PIONS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../config.h"  // 🔧 FIX: Include config.h BEFORE using AvatarID

typedef enum { EMPTY = 0, WHITE = 1, BLACK = -1, NOBODY = 2 } Player;

// 🆕 TYPES DE PIONS (couleur visuelle)
typedef enum {
    PIECE_COLOR_WHITE = 0,    // Pions blancs
    PIECE_COLOR_BLACK = 1,    // Pions noirs
    PIECE_COLOR_BROWN = 2     // Pions bruns (variante)
} PieceColor;

// 🆕 TYPES DE JOUEUR
typedef enum {
    PLAYER_TYPE_HUMAN = 0,
    PLAYER_TYPE_AI = 1,
    PLAYER_TYPE_ONLINE = 2
} PlayerType;

// 🆕 STRUCTURE PIECE AMÉLIORÉE
typedef struct Piece {
    int id;
    Player owner;           // WHITE ou BLACK (logique)
    PieceColor color;       // Couleur visuelle du pion
    int r, c;              // Coordonnées
    int alive;             // 1 = sur plateau, 0 = capturé
    
    // 🆕 TEXTURES
    SDL_Texture* texture;        // Texture principale
    SDL_Texture* hover_texture;  // Texture au survol
    SDL_Texture* selected_texture; // Texture sélectionné
    
    // 🆕 ÉTAT VISUEL
    bool is_selected;
    bool is_hovered;
    bool is_highlighted;   // Pour montrer les coups possibles
} Piece;

// 🆕 STRUCTURE PLAYER COMPLÈTE
typedef struct GamePlayer {
    char name[64];              
    Player logical_color;       
    PieceColor piece_color;     
    PlayerType type;            
    int player_number;          
    
    // 🆕 AVATAR avec ID fixe pour multijoueur
    AvatarID avatar;            // ID fixe de l'avatar sélectionné
    
    // 🆕 STATISTIQUES
    int pieces_remaining;       // Pions restants
    int captures_made;          // Captures effectuées
    float total_time;           // Temps total de jeu
    float thinking_time;        // 🔧 FIX: Add missing thinking_time field
    
    // 🆕 ÉTAT DU TOUR
    bool is_current_turn;
    bool has_mandatory_capture; // Doit capturer
    
    // 🆕 PARAMÈTRES IA (si applicable)
    int ai_difficulty;          // 1=facile, 2=moyen, 3=difficile
    float ai_thinking_time;     // Temps de réflexion
    
    // 🆕 PARAMÈTRES RÉSEAU (si applicable)
    bool is_connected;
    char remote_id[32];
} GamePlayer;

// 🆕 FONCTIONS D'INITIALISATION DES PIONS
Piece* piece_create(int id, Player owner, PieceColor color, int r, int c);
void piece_destroy(Piece* piece);
void piece_set_textures(Piece* piece, SDL_Texture* normal, SDL_Texture* hover, SDL_Texture* selected);
void piece_set_position(Piece* piece, int r, int c);
void piece_set_visual_state(Piece* piece, bool selected, bool hovered, bool highlighted);

// 🆕 FONCTIONS D'INITIALISATION DES JOUEURS
GamePlayer* player_create(const char* name, Player logical_color, PieceColor piece_color, 
                         PlayerType type, int player_number);
void player_destroy(GamePlayer* player);
void player_init_human(GamePlayer* player, const char* name, Player logical_color, 
                      PieceColor piece_color, int player_number);
void player_init_ai(GamePlayer* player, const char* name, Player logical_color, 
                   PieceColor piece_color, int player_number, int difficulty);
void player_init_online(GamePlayer* player, const char* name, Player logical_color, 
                       PieceColor piece_color, int player_number, const char* remote_id);

// 🆕 FONCTIONS UTILITAIRES
const char* piece_color_to_string(PieceColor color);
const char* player_type_to_string(PlayerType type);
SDL_Texture* piece_get_current_texture(Piece* piece);
void player_reset_stats(GamePlayer* player);
void player_add_capture(GamePlayer* player);
void player_set_turn(GamePlayer* player, bool is_turn);

#endif
