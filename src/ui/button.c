#include "button.h"
#include <stdlib.h>
#include <string.h>

static void button_render(Layer *self, SDL_Renderer *ren) {
    Button *btn = (Button *)self;
    if (!btn || !ren) return;
    
    // TODO: Implement button rendering
    SDL_SetRenderDrawColor(ren, 100, 100, 100, 255);
    SDL_RenderFillRect(ren, &self->rect);
}

static void button_event(Layer *self, SDL_Event *e) {
    Button *btn = (Button *)self;
    if (!btn || !e) return;
    
    // TODO: Implement button event handling
    if (e->type == SDL_MOUSEBUTTONDOWN && btn->on_click) {
        btn->on_click(btn->ud);
    }
}

Button *button_create(const char *text, TTF_Font *f, void (*cb)(void *), void *ud) {
    (void)text; (void)f; // Suppress warnings for now
    
    Button *btn = malloc(sizeof(Button));
    memset(btn, 0, sizeof(Button));
    
    btn->base.on_render = button_render;
    btn->base.on_event = button_event;
    btn->on_click = cb;
    btn->ud = ud;
    
    return btn;
}
