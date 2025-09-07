#pragma once
#include "../window/window_manager.h"
#include <stdbool.h>

typedef struct {
    WindowManager wm;
    GameWindow *menu_window;
    GameWindow *game_window;
} CoreState;

bool core_init(CoreState *core);
void core_quit(CoreState *core);
void core_create_menu_window(CoreState *core);
void core_create_game_window(CoreState *core);
void core_switch_to_menu(CoreState *core);
void core_switch_to_game(CoreState *core);