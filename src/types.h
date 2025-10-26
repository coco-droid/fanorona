#ifndef FANORONA_TYPES_H
#define FANORONA_TYPES_H

// === TYPES DE BASE DU JEU ===

typedef enum { 
    EMPTY = 0, 
    WHITE = 1, 
    BLACK = -1, 
    NOBODY = 2,
    PLAYER_1 = 1,   // 🆕 Premier joueur (joue en premier)
    PLAYER_2 = 2    // 🆕 Deuxième joueur
} Player;

// === COULEURS DES PIÈCES ===

typedef enum {
    PIECE_COLOR_WHITE = 0,    // Pions blancs
    PIECE_COLOR_BLACK = 1,    // Pions noirs
    PIECE_COLOR_BROWN = 2     // Pions bruns (variante)
} PieceColor;

// === SYSTÈME D'AVATARS ===

typedef enum {
    AVATAR_WARRIOR = 1,      // p1.png - Guerrier
    AVATAR_STRATEGIST = 2,   // p2.png - Stratège
    AVATAR_DIPLOMAT = 3,     // p3.png - Diplomate
    AVATAR_EXPLORER = 4,     // p4.png - Explorateur
    AVATAR_MERCHANT = 5,     // p5.png - Marchand
    AVATAR_SAGE = 6          // p6.png - Sage
} AvatarID;

// === MODES DE JEU ===

typedef enum {
    GAME_MODE_NONE = 0,           // Aucun mode sélectionné
    GAME_MODE_LOCAL_MULTIPLAYER,  // Multijoueur sur le même PC
    GAME_MODE_ONLINE_MULTIPLAYER, // Multijoueur en ligne
    GAME_MODE_VS_AI               // Contre l'IA
} GameMode;

// === NIVEAUX DE DIFFICULTÉ IA ===

typedef enum {
    AI_DIFFICULTY_EASY = 1,    // Facile
    AI_DIFFICULTY_MEDIUM = 2,  // Moyen
    AI_DIFFICULTY_HARD = 3     // Difficile
} AIDifficulty;

#endif // FANORONA_TYPES_H
