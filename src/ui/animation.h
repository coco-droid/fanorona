#pragma once
#include "../core/timer.h"
#include <SDL2/SDL.h>

typedef enum {
    EASE_LINEAR,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD
} EaseType;

typedef struct {
    SDL_Point start_pos;
    SDL_Point end_pos;
    Timer timer;
    EaseType ease;
    void (*on_complete)(void *userdata);
    void *userdata;
} MoveAnimation;

MoveAnimation *anim_create_move(SDL_Point from, SDL_Point to, double duration, EaseType ease);
void anim_update(MoveAnimation *anim, double dt);
SDL_Point anim_get_current_pos(const MoveAnimation *anim);
bool anim_is_finished(const MoveAnimation *anim);
void anim_destroy(MoveAnimation *anim);
