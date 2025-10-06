#ifndef PLATEAU_H
#define PLATEAU_H

#include "../pions/pions.h"

#define ROWS 5
#define COLS 9
#define NODES (ROWS * COLS)
#define MAX_NEI 8

typedef struct Intersection {
    int id;
    int r, c;
    Piece *piece;       // occupant or NULL
    int neighbors[MAX_NEI];
    int nnei;
    int strong;         // 1 = intersection with diagonals allowed (cross), 0 = only orthogonal (diamond)
} Intersection;

typedef struct Board {
    Intersection nodes[NODES];
    Piece *pieces[NODES]; // keep pointers to allocated pieces (for freeing)
    int piece_count;
} Board;

// Utility functions
int in_bounds(int r, int c);
int node_id(int r, int c);
void rc_from_id(int id, int *r, int *c);

// Board functions
void board_init(Board *b);
void board_print(Board *b);
void board_free(Board *b);
void board_destroy(Board* board);

#endif
