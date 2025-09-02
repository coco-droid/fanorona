#include "event_dispatcher.h"
#include <stdlib.h>

EventDispatcher *ed_create(LayerManager *lm) {
    EventDispatcher *ed = malloc(sizeof(EventDispatcher));
    ed->lm = lm;
    return ed;
}

void ed_destroy(EventDispatcher *ed) {
    if (!ed) return;
    free(ed);
}

void ed_dispatch(EventDispatcher *ed, SDL_Event *e) {
    if (!ed || !e) return;
    lm_dispatch(ed->lm, e);
}
