#include "minimax.h"

double minimax_eval(const GameState *g, int depth) {
    if (!g || depth <= 0) return 0.0;
    
    // Basic evaluation: count pieces
    double score = 0.0;
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 5; y++) {
            if (g->board[x][y] == WHITE) score += 1.0;
            else if (g->board[x][y] == BLACK) score -= 1.0;
        }
    }
    
    return score;
}
