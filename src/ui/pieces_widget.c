#include "pieces_widget.h"
#include <stdlib.h>
#include <string.h>

PieceWidget *piece_widget_create(const Piece *p) {
    PieceWidget *pw = malloc(sizeof(PieceWidget));
    memset(pw, 0, sizeof(PieceWidget));
    
    if (p) pw->piece = *p;
    pw->dragging = false;
    pw->animating = false;
    pw->highlighted = false;
    
    return pw;
}

void piece_widget_start_drag(PieceWidget *pw, int mx, int my) {
    if (!pw) return;
    pw->dragging = true;
    pw->drag_offset.x = mx - pw->base.rect.x;
    pw->drag_offset.y = my - pw->base.rect.y;
}

void piece_widget_stop_drag(PieceWidget *pw) {
    if (!pw) return;
    pw->dragging = false;
}

void piece_widget_animate_to(PieceWidget *pw, SDL_Point target, double duration) {
    if (!pw) return;
    // TODO: Implement animation
    (void)target; (void)duration;
}

void piece_widget_set_highlight(PieceWidget *pw, bool highlight) {
    if (!pw) return;
    pw->highlighted = highlight;
}

void piece_widget_update(PieceWidget *pw, double dt) {
    if (!pw) return;
    // TODO: Update animations
    (void)dt;
}
