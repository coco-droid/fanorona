#include "timer.h"
#include <SDL2/SDL.h>

static double start_time = 0;

double timer_now(void) {
    if (start_time == 0) {
        start_time = SDL_GetTicks() / 1000.0;
    }
    return (SDL_GetTicks() / 1000.0) - start_time;
}

void timer_delay(double ms) {
    SDL_Delay((Uint32)ms);
}

Timer game_timer_create(double duration) {
    Timer t = {0};
    t.duration = duration;
    t.active = false;
    return t;
}

void timer_start(Timer *t) {
    if (!t) return;
    t->start_time = timer_now();
    t->active = true;
}

bool timer_is_finished(const Timer *t) {
    if (!t || !t->active) return false;
    return (timer_now() - t->start_time) >= t->duration;
}

double timer_get_progress(const Timer *t) {
    if (!t || !t->active) return 0.0;
    double elapsed = timer_now() - t->start_time;
    return elapsed / t->duration;
}

double timer_get_remaining(const Timer *t) {
    if (!t || !t->active) return 0.0;
    double elapsed = timer_now() - t->start_time;
    return t->duration - elapsed;
}
