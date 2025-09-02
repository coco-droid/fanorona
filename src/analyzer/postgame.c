#include "postgame.h"
#include <stdio.h>

void analyzer_start(const GameState *history, int len,
                    void (*callback)(AltMove *alts, int count)) {
    (void)history; (void)len;
    
    if (callback) {
        // Placeholder: no alternative moves for now
        callback(NULL, 0);
    }
    
    printf("Postgame analysis not implemented yet\n");
}

void analyzer_wait(void) {
    // Wait for analysis to complete
}
