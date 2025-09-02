#pragma once
#include "../layer/layer.h"

typedef enum {
    WIDGET_BUTTON,
    WIDGET_PIECE,
    WIDGET_BOARD,
    WIDGET_LABEL
} WidgetType;

typedef struct {
    Layer base;
    WidgetType type;
    bool enabled;
    bool visible;
    void (*on_update)(void *widget, double dt);
} Widget;

#define WIDGET_FROM_LAYER(ptr) ((Widget *)(ptr))
#define BUTTON_FROM_WIDGET(ptr) ((Button *)(ptr))
#define PIECE_FROM_WIDGET(ptr) ((PieceWidget *)(ptr))

Widget *widget_create(WidgetType type);
void widget_set_enabled(Widget *w, bool enabled);
void widget_set_visible(Widget *w, bool visible);
void widget_update(Widget *w, double dt);