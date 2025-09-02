#pragma once
#include <stdbool.h>

typedef struct {
    double start_time;
    double duration;
    bool active;
} Timer;

double timer_now(void);
void   timer_delay(double ms);

// Animation timing utilities
Timer  game_timer_create(double duration);  // Renamed to avoid conflict
void   timer_start(Timer *t);
bool   timer_is_finished(const Timer *t);
double timer_get_progress(const Timer *t); // 0.0 to 1.0
double timer_get_remaining(const Timer *t);