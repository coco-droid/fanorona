#ifndef PIONS_H
#define PIONS_H

typedef enum { EMPTY = 0, WHITE = 1, BLACK = -1 } Player;

typedef struct Piece {
    int id;
    Player owner;
    int r, c;           // coordinates
    int alive;          // 1 = in board, 0 = captured
} Piece;

#endif
