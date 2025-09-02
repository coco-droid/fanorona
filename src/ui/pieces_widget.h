#pragma once
#include "../layer/layer.h"
#include "../engine/fanorona.h"
#include "animation.h"

typedef enum { EMPTY_PIECE, WHITE_PIECE, BLACK_PIECE } Piece;

typedef struct {
    Layer base;
    Piece piece;
    bool dragging;
    bool animating;
    SDL_Point drag_offset;
    SDL_Point grid_pos; // Board position (x, y)
    MoveAnimation *move_anim;
    bool highlighted; // For move hints
} PieceWidget;

PieceWidget *piece_widget_create(const Piece *p);
void         piece_widget_start_drag(PieceWidget *pw, int mx, int my);
void         piece_widget_stop_drag(PieceWidget *pw);
void         piece_widget_animate_to(PieceWidget *pw, SDL_Point target, double duration);
void         piece_widget_set_highlight(PieceWidget *pw, bool highlight);
void         piece_widget_update(PieceWidget *pw, double dt);