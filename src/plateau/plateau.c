#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plateau.h"

// Utility functions
int in_bounds(int r, int c) { return (r >= 0 && r < ROWS && c >= 0 && c < COLS); }
int node_id(int r, int c) { return r * COLS + c; }
void rc_from_id(int id, int *r, int *c) { *r = id / COLS; *c = id % COLS; }

// Initialize board structure, place pieces according to Fanoron-Sivy initial setup
void board_init(Board *b) {
    memset(b, 0, sizeof(Board));
    // init intersections
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            int id = node_id(r,c);
            b->nodes[id].id = id;
            b->nodes[id].r = r;
            b->nodes[id].c = c;
            b->nodes[id].piece = NULL;
            b->nodes[id].nnei = 0;
            // convention : "strong" (croix) when (r+c) even -> allow diagonals
            b->nodes[id].strong = ((r + c) % 2 == 0) ? 1 : 0;
        }
    }

    // build neighbors for each intersection
    // directions: N, S, W, E, NW, NE, SW, SE
    const int dr[8] = {-1, +1,  0,  0, -1, -1, +1, +1};
    const int dc[8] = { 0,  0, -1, +1, -1, +1, -1, +1};
    for (int id = 0; id < NODES; ++id) {
        Intersection *it = &b->nodes[id];
        for (int d = 0; d < 8; ++d) {
            int rr = it->r + dr[d];
            int cc = it->c + dc[d];
            if (!in_bounds(rr, cc)) continue;
            // diagonal directions correspond to d>=4
            if (d >= 4 && !it->strong) continue; // only strong nodes have diagonals
            // For safety: also ensure neighbor node allows that edge from its side if diagonal
            if (d >=4) {
                // neighbor must also be strong to have diagonal connection (consistent grid)
                if (!b->nodes[node_id(rr,cc)].strong) continue;
            }
            it->neighbors[it->nnei++] = node_id(rr,cc);
        }
    }

    // Place pieces according to Fanoron-sivy:
    // Top two rows (r=0,1) = WHITE
    // Bottom two rows (r=3,4) = BLACK
    // Middle row (r=2) alternating W B W B . B W B W with center (c=4) empty
    int pid = 1;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            int id = node_id(r,c);
            Player p = EMPTY;
            if (r < 2) {
                p = WHITE;
            } else if (r > 2) {
                p = BLACK;
            } else { // r == 2 middle row
                if (c == 4) p = EMPTY; // center empty
                else p = (c % 2 == 0) ? WHITE : BLACK; // alternate W B W B ....
            }
            if (p != EMPTY) {
                Piece *pc = (Piece*) malloc(sizeof(Piece));
                pc->id = pid++;
                pc->owner = p;
                pc->r = r;
                pc->c = c;
                pc->alive = 1;
                b->nodes[id].piece = pc;
                b->pieces[b->piece_count++] = pc;
            } else {
                b->nodes[id].piece = NULL;
            }
        }
    }
}

// ASCII debug print (with coordinates)
void board_print(Board *b) {
    printf("    ");
    for (int c = 0; c < COLS; ++c) printf(" %d", c);
    printf("\n");
    for (int r = 0; r < ROWS; ++r) {
        printf(" %d |", r);
        for (int c = 0; c < COLS; ++c) {
            int id = node_id(r,c);
            Piece *pc = b->nodes[id].piece;
            if (pc == NULL) {
                printf(" .");
            } else if (pc->owner == WHITE) {
                printf(" O");
            } else {
                printf(" X");
            }
        }
        printf("\n");
    }
    printf("\nLegend: O=White  X=Black  .=empty\n\n");
}

// Release allocated pieces in board
void board_free(Board *b) {
    if (!b) return;
    
    printf("🧹 [BOARD_FREE] Nettoyage du plateau avec %d pièces\n", b->piece_count);
    
    // 🔧 FIX: Libérer toutes les pièces allouées, même celles marquées comme mortes
    for (int i = 0; i < b->piece_count; ++i) {
        Piece *p = b->pieces[i];
        if (p) {
            printf("🗑️ [BOARD_FREE] Libération pièce %d (owner=%d, alive=%d)\n", 
                   p->id, p->owner, p->alive);
            free(p);
            b->pieces[i] = NULL; // 🔧 Éviter les double-free
        }
    }
    
    // 🔧 FIX: Nettoyer les références dans les intersections
    for (int i = 0; i < NODES; i++) {
        b->nodes[i].piece = NULL; // 🔧 Éviter les pointeurs pendants
    }
    
    // 🔧 FIX: Réinitialiser le compteur
    b->piece_count = 0;
    
    printf("✅ [BOARD_FREE] Plateau nettoyé complètement\n");
}

// 🆕 NOUVELLE FONCTION: Destruction complète du board
void board_destroy(Board* board) {
    if (!board) return;
    
    printf("🧹 [BOARD_DESTROY] Destruction complète du plateau\n");
    
    // Libérer toutes les pièces
    board_free(board);
    
    // Le board lui-même sera libéré par l'appelant car il peut être alloué sur la pile ou le tas
    printf("✅ [BOARD_DESTROY] Plateau détruit\n");
}
