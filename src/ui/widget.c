#include "widget.h"
#include <stdlib.h>
#include <string.h>

Widget *widget_create(WidgetType type) {
    Widget *w = malloc(sizeof(Widget));
    memset(w, 0, sizeof(Widget));
    w->type = type;
    w->enabled = true;
    w->visible = true;
    return w;
}

void widget_set_enabled(Widget *w, bool enabled) {
    if (!w) return;
    w->enabled = enabled;
}

void widget_set_visible(Widget *w, bool visible) {
    if (!w) return;
    w->visible = visible;
}

void widget_update(Widget *w, double dt) {
    if (!w || !w->on_update) return;
    w->on_update(w, dt);
}
