#pragma once
#include <stdbool.h>

typedef struct {
    // Display settings
    int window_width;
    int window_height;
    bool fullscreen;
    bool vsync;
    
    // Audio settings
    float master_volume;
    float sfx_volume;
    bool audio_enabled;
    
    // Gameplay settings
    int ai_difficulty; // 1-5
    bool show_hints;
    bool animate_moves;
    double animation_speed;
    
    // Network settings
    unsigned short default_port;
    char player_name[32];
} Config;

bool config_load(Config *cfg, const char *filename);
bool config_save(const Config *cfg, const char *filename);
void config_set_defaults(Config *cfg);
