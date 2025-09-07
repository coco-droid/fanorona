#include "sdl_init.h"

bool core_init(CoreState *core) {
    if (!core) return false;
    
    if (!wm_init(&core->wm)) {
        return false;
    }
    
    core->menu_window = NULL;
    core->game_window = NULL;
    
    return true;
}

void core_quit(CoreState *core) {
    if (!core) return;
    wm_quit(&core->wm);
}

void core_create_menu_window(CoreState *core) {
    if (!core) return;
    
    core->menu_window = wm_create_window(&core->wm, WINDOW_MENU, 
                                        "Fanorona - Menu", 
                                        600, 400,    // Plus petite fenêtre
                                        true, 15);   // Coins arrondis avec rayon personnalisé
}

void core_create_game_window(CoreState *core) {
    if (!core) return;
    
    core->game_window = wm_create_window(&core->wm, WINDOW_GAME,
                                        "Fanorona - Game",
                                        1024, 768,   // Grande fenêtre
                                        true, 8);    // Coins arrondis avec rayon plus petit
}

void core_switch_to_menu(CoreState *core) {
    if (!core) return;
    
    if (core->game_window) {
        wm_hide_window(&core->wm, WINDOW_GAME);
    }
    
    if (!core->menu_window) {
        core_create_menu_window(core);
    }
    
    wm_show_window(&core->wm, WINDOW_MENU);
    wm_set_active_window(&core->wm, WINDOW_MENU);
}

void core_switch_to_game(CoreState *core) {
    if (!core) return;
    
    if (core->menu_window) {
        wm_hide_window(&core->wm, WINDOW_MENU);
    }
    
    if (!core->game_window) {
        core_create_game_window(core);
    }
    
    wm_show_window(&core->wm, WINDOW_GAME);
    wm_set_active_window(&core->wm, WINDOW_GAME);
}