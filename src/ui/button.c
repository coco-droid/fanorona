#include "button.h"
#include <stdlib.h>
#include <string.h>

static void button_render(Layer *self, SDL_Renderer *ren) {
    Button *btn = (Button *)self;
    if (!btn || !ren) return;
    
    // Draw background image if provided
    if (btn->image) {
        SDL_RenderCopy(ren, btn->image, NULL, &self->rect);
    } else {
        // Fallback: solid color background
        SDL_SetRenderDrawColor(ren, 100, 100, 100, 255);
        SDL_RenderFillRect(ren, &self->rect);
    }
    
    // TODO: Render text on top of the background
}

static void button_event(Layer *self, SDL_Event *e) {
    Button *btn = (Button *)self;
    if (!btn || !e) return;
    
    // TODO: Implement button event handling
    if (e->type == SDL_MOUSEBUTTONDOWN && btn->on_click) {
        btn->on_click(btn->ud);
    }
}

Button *button_create(const char *text, TTF_Font *f, SDL_Texture *image, void (*cb)(void *), void *ud) {
    (void)text; (void)f; // Suppress warnings for now
    
    Button *btn = malloc(sizeof(Button));
    memset(btn, 0, sizeof(Button));
    
    btn->base.on_render = button_render;
    btn->base.on_event = button_event;
    btn->image = image;  // Store the background image
    btn->on_click = cb;
    btn->ud = ud;
    
    return btn;
}
