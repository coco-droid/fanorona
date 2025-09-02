#pragma once
#include "../engine/fanorona.h"
typedef struct { Pos from, to; float score; } AltMove;
void analyzer_start(const GameState *history, int len,
                    void (*callback)(AltMove *alts, int count));
void analyzer_wait(void);