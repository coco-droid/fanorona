#include "animation.h"
#include <stdlib.h>
#include <math.h>

static double ease_function(double t, EaseType ease) {
    switch (ease) {
        case EASE_LINEAR: return t;
        case EASE_IN_QUAD: return t * t;
        case EASE_OUT_QUAD: return 1 - (1 - t) * (1 - t);
        case EASE_IN_OUT_QUAD: 
            return t < 0.5 ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;
        default: return t;
    }
}

MoveAnimation *anim_create_move(SDL_Point from, SDL_Point to, double duration, EaseType ease) {
    MoveAnimation *anim = malloc(sizeof(MoveAnimation));
    anim->start_pos = from;
    anim->end_pos = to;
    anim->timer = game_timer_create(duration);
    anim->ease = ease;
    anim->on_complete = NULL;
    anim->userdata = NULL;
    return anim;
}

void anim_update(MoveAnimation *anim, double dt) {
    if (!anim) return;
    (void)dt; // Timer handles its own timing
}

SDL_Point anim_get_current_pos(const MoveAnimation *anim) {
    if (!anim || !anim->timer.active) return anim->start_pos;
    
    double progress = timer_get_progress(&anim->timer);
    double eased = ease_function(progress, anim->ease);
    
    SDL_Point pos;
    pos.x = anim->start_pos.x + (int)((anim->end_pos.x - anim->start_pos.x) * eased);
    pos.y = anim->start_pos.y + (int)((anim->end_pos.y - anim->start_pos.y) * eased);
    
    return pos;
}

bool anim_is_finished(const MoveAnimation *anim) {
    if (!anim) return true;
    return timer_is_finished(&anim->timer);
}

void anim_destroy(MoveAnimation *anim) {
    if (!anim) return;
    free(anim);
}
