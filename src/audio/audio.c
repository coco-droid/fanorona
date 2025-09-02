#include "audio.h"
#include <stdio.h>

#ifdef HAVE_SDL_MIXER
#include <SDL2/SDL_mixer.h>

static bool audio_initialized = false;

bool audio_init(void) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    audio_initialized = true;
    return true;
}

void audio_play(const char *sample) {
    if (!audio_initialized) return;
    // TODO: Load and play sound samples
    (void)sample; // Suppress unused warning
}

void audio_quit(void) {
    if (audio_initialized) {
        Mix_Quit();
        audio_initialized = false;
    }
}

#else

bool audio_init(void) {
    printf("Audio disabled - SDL_mixer not available\n");
    return false;
}

void audio_play(const char *sample) {
    (void)sample;
}

void audio_quit(void) {
    // No-op
}

#endif
