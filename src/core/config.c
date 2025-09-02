#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void config_set_defaults(Config *cfg) {
    if (!cfg) return;
    
    // Display settings
    cfg->window_width = 1024;
    cfg->window_height = 768;
    cfg->fullscreen = false;
    cfg->vsync = true;
    
    // Audio settings
    cfg->master_volume = 1.0f;
    cfg->sfx_volume = 1.0f;
    cfg->audio_enabled = true;
    
    // Gameplay settings
    cfg->ai_difficulty = 3;
    cfg->show_hints = true;
    cfg->animate_moves = true;
    cfg->animation_speed = 1.0;
    
    // Network settings
    cfg->default_port = 7777;
    strcpy(cfg->player_name, "Player");
}

bool config_load(Config *cfg, const char *filename) {
    if (!cfg || !filename) return false;
    
    config_set_defaults(cfg);
    
    FILE *f = fopen(filename, "r");
    if (!f) return false;
    
    // Simple key=value parser
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (strncmp(line, "width=", 6) == 0) {
            cfg->window_width = atoi(line + 6);
        } else if (strncmp(line, "height=", 7) == 0) {
            cfg->window_height = atoi(line + 7);
        }
        // TODO: Add more config parsing
    }
    
    fclose(f);
    return true;
}

bool config_save(const Config *cfg, const char *filename) {
    if (!cfg || !filename) return false;
    
    FILE *f = fopen(filename, "w");
    if (!f) return false;
    
    fprintf(f, "width=%d\n", cfg->window_width);
    fprintf(f, "height=%d\n", cfg->window_height);
    fprintf(f, "fullscreen=%d\n", cfg->fullscreen ? 1 : 0);
    fprintf(f, "volume=%.2f\n", cfg->master_volume);
    // TODO: Add more config saving
    
    fclose(f);
    return true;
}
