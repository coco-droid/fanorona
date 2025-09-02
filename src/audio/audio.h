#pragma once
#include <stdbool.h>

bool audio_init(void);
void audio_play(const char *sample);   // "move", "capture", etc.
void audio_quit(void);