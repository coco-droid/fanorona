#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"
#include "../utils/asset_manager.h"
#include "../plateau/plateau.h"
#include "../types.h"
#include "../config.h"
#include "../pions/pions.h"
#include "../logic/logic.h"
#include "../logic/rules.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Dimensions du plateau visuelles
#define PLATEAU_VISUAL_WIDTH 550
#define PLATEAU_VISUAL_HEIGHT 370
#define PLATEAU_MARGIN 30
#define INTERSECTION_RADIUS 8
#define PIECE_RADIUS 12
#define LINE_THICKNESS 2

// Couleurs
#define PLATEAU_BG_R 139
#define PLATEAU_BG_G 115
#define PLATEAU_BG_B 85
#define LINE_COLOR_R 0
#define LINE_COLOR_G 0
#define LINE_COLOR_B 0

// === VISUAL FEEDBACK STRUCTURES ===
typedef struct VisualFeedbackState {
    int hovered_intersection;
    int selected_intersection;
    int* valid_destinations;
    int valid_count;
    float animation_timer;
    char error_message[128];
    float error_display_timer;
    int error_type;
    // 🆕 CHAÎNE DE CAPTURES
    bool in_capture_chain;
    int capture_chain_from;
    Direction last_capture_direction;
    int visited_positions[MAX_VISITED_POSITIONS];
    int visited_count;
} VisualFeedbackState;

typedef struct PlateauRenderData {
    Board* board;
    SDL_Renderer* renderer;
    int offset_x;
    int offset_y;
    int cell_width;
    int cell_height;
    bool show_intersections;
    bool show_coordinates;
    GamePlayer* player1;
    GamePlayer* player2;
    SDL_Texture* texture_black;
    SDL_Texture* texture_brown;
    VisualFeedbackState* visual_state;
    void* game_logic;
    AtomicElement* intersection_elements[NODES];
} PlateauRenderData;

// === FORWARD DECLARATIONS ===
static void on_intersection_click(void* element, SDL_Event* event);
static void on_intersection_hover(void* element, SDL_Event* event);
static void on_intersection_unhover(void* element, SDL_Event* event);
static void create_intersection_atomic_elements(PlateauRenderData* data);
static void plateau_draw_visual_effects(PlateauRenderData* data);
static void update_intersection_positions(PlateauRenderData* data);
static void calculate_valid_destinations(PlateauRenderData* data, int piece_id);
static void plateau_draw_valid_destinations(PlateauRenderData* data);
static bool is_valid_destination(PlateauRenderData* data, int from_id, int to_id);
static void execute_animated_move(PlateauRenderData* data, int from_id, int to_id);
static void apply_move_to_board(PlateauRenderData* data, int from_id, int to_id);
// FIX: Added forward declarations for functions called before they are defined
static void log_board_state(Board* board, const char* context);
static bool has_additional_captures(PlateauRenderData* data, int from_id);


// === PLATEAU COMPONENT ===

// === PLACEHOLDER IMPLEMENTATIONS ===
static void animate_piece_move(PlateauRenderData* data, int from_id, int to_id) {
    (void)data;
    printf("🔄 Animate piece move: %d -> %d\n", from_id, to_id);
}

static void animate_piece_selection(PlateauRenderData* data, int piece_id) {
    (void)data;
    printf("🎯 Animate piece selection: %d\n", piece_id);
}

static void animate_piece_capture(PlateauRenderData* data, int piece_id) {
    (void)data;
    printf("💥 Animate piece capture: %d\n", piece_id);
}

static void animate_piece_placement(PlateauRenderData* data, int intersection_id) {
    (void)data;
    printf("📍 Animate piece placement: %d\n", intersection_id);
}

static void animate_victory_dance(PlateauRenderData* data, Player winning_player) {
    (void)data;
    printf("🎉 Animate victory dance for player %d\n", winning_player);
}

static void animate_defeat_fade(PlateauRenderData* data, Player losing_player) {
    (void)data;
    (void)losing_player; // 🔧 FIX: Paramètre non utilisé
    printf("😞 Animate defeat fade for player %d\n", losing_player);
}

static void animate_initial_piece_wave(PlateauRenderData* data) {
    (void)data;
    printf("🌊 Animate initial piece wave\n");
}

// === FONCTIONS UTILITAIRES DE DESSIN ===

// Convertir coordonnées logiques (r,c) en coordonnées écran (x,y)
static void plateau_logical_to_screen(PlateauRenderData* data, int r, int c, int* x, int* y) {
    *x = data->offset_x + c * data->cell_width;
    *y = data->offset_y + r * data->cell_height;
}

// Dessiner une ligne entre deux intersections
static void plateau_draw_line(PlateauRenderData* data, int r1, int c1, int r2, int c2) {
    int x1, y1, x2, y2;
    plateau_logical_to_screen(data, r1, c1, &x1, &y1);
    plateau_logical_to_screen(data, r2, c2, &x2, &y2);
    SDL_SetRenderDrawColor(data->renderer, LINE_COLOR_R, LINE_COLOR_G, LINE_COLOR_B, 255);
    for (int i = 0; i < LINE_THICKNESS; i++) {
        SDL_RenderDrawLine(data->renderer, x1 + i, y1, x2 + i, y2);
        SDL_RenderDrawLine(data->renderer, x1, y1 + i, x2, y2 + i);
    }
}

// Dessiner un cercle plein
static void plateau_draw_filled_circle(SDL_Renderer* renderer, int center_x, int center_y, int radius,
    int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, center_x + x, center_y + y);
            }
        }
    }
}

// Dessiner une intersection
static void plateau_draw_intersection(PlateauRenderData* data, int r, int c, bool is_strong) {
    int x, y;
    plateau_logical_to_screen(data, r, c, &x, &y);
    if (data->show_intersections) {
        int radius = is_strong ? INTERSECTION_RADIUS : INTERSECTION_RADIUS - 2;
        int color_intensity = is_strong ? 255 : 180;
        plateau_draw_filled_circle(data->renderer, x, y, radius, 101, 67, 33, color_intensity);
    }
}

// Dessiner un pion avec texture et effets visuels
static void plateau_draw_piece(PlateauRenderData* data, int r, int c, Player owner) {
    int x, y;
    plateau_logical_to_screen(data, r, c, &x, &y);

    SDL_Texture* texture_to_use = NULL;
    extern GameConfig* config_get_current(void);
    GameConfig* cfg = config_get_current();

    if (owner == WHITE) {
        texture_to_use = (cfg->player1_piece_color == PIECE_COLOR_BLACK) ?
            data->texture_black : data->texture_brown;
    }
    else if (owner == BLACK) {
        texture_to_use = (cfg->player2_piece_color == PIECE_COLOR_BLACK) ?
            data->texture_black : data->texture_brown;
    }

    int intersection_id = node_id(r, c);
    bool is_hovered = data->visual_state && (data->visual_state->hovered_intersection == intersection_id);
    bool is_selected = data->visual_state && (data->visual_state->selected_intersection == intersection_id);

    if (texture_to_use) {
        SDL_Rect dest_rect = { x - PIECE_RADIUS, y - PIECE_RADIUS, PIECE_RADIUS * 2, PIECE_RADIUS * 2 };
        SDL_RenderCopy(data->renderer, texture_to_use, NULL, &dest_rect);
    }
    else {
        if (owner == WHITE) {
            plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS, 240, 240, 240, 255);
        }
        else {
            plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS, 30, 30, 30, 255);
        }
    }

    if (is_selected) {
        SDL_SetRenderDrawColor(data->renderer, 0, 255, 0, 255);
        for (int t = 0; t < 3; t++) {
            for (int angle = 0; angle < 360; angle += 5) {
                float rad = angle * 3.14159f / 180.0f;
                int bx = x + (PIECE_RADIUS + t) * cosf(rad);
                int by = y + (PIECE_RADIUS + t) * sinf(rad);
                SDL_RenderDrawPoint(data->renderer, bx, by);
            }
        }
    }
    else if (is_hovered) {
        SDL_SetRenderDrawColor(data->renderer, 255, 215, 0, 255);
        for (int angle = 0; angle < 360; angle += 3) {
            float rad = angle * 3.14159f / 180.0f;
            int bx = x + (PIECE_RADIUS + 2) * cosf(rad);
            int by = y + (PIECE_RADIUS + 2) * sinf(rad);
            SDL_RenderDrawPoint(data->renderer, bx, by);
        }
    }
}

// === FONCTIONS DE RENDU PRINCIPAL ===

static void plateau_draw_all_lines(PlateauRenderData* data) {
    if (!data->board) return;
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        for (int i = 0; i < intersection->nnei; i++) {
            int neighbor_id = intersection->neighbors[i];
            if (neighbor_id > id) {
                Intersection* neighbor = &data->board->nodes[neighbor_id];
                plateau_draw_line(data, intersection->r, intersection->c, neighbor->r, neighbor->c);
            }
        }
    }
}

static void plateau_draw_all_intersections(PlateauRenderData* data) {
    if (!data->board) return;
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        plateau_draw_intersection(data, intersection->r, intersection->c, intersection->strong);
    }
}

static void plateau_draw_all_pieces(PlateauRenderData* data) {
    if (!data->board) return;
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        if (intersection->piece && intersection->piece->alive) {
            plateau_draw_piece(data, intersection->r, intersection->c, intersection->piece->owner);
        }
    }
}

static void plateau_load_piece_textures(PlateauRenderData* data, SDL_Renderer* renderer) {
    if (!data->texture_black) {
        data->texture_black = asset_load_texture(renderer, "piece_black.png");
        if (data->texture_black) {
            printf("✅ Texture piece_black.png chargée\n");
        }
    }
    if (!data->texture_brown) {
        data->texture_brown = asset_load_texture(renderer, "piece_brun.png");
        if (data->texture_brown) {
            printf("✅ Texture piece_brun.png chargée\n");
        }
    }
}

// FIX: Added function definitions for the hover/unhover handlers
static void on_intersection_hover(void* element, SDL_Event* event) {
    (void)event;
    AtomicElement* atomic = (AtomicElement*)element;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(atomic, "plateau_data");
    int intersection_id = (int)(intptr_t)atomic_get_custom_data(atomic, "intersection_id");
    if (data && data->visual_state) {
        data->visual_state->hovered_intersection = intersection_id;
    }
}

static void on_intersection_unhover(void* element, SDL_Event* event) {
    (void)event;
    AtomicElement* atomic = (AtomicElement*)element;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(atomic, "plateau_data");
    if (data && data->visual_state) {
        if (data->visual_state->hovered_intersection == (int)(intptr_t)atomic_get_custom_data(atomic, "intersection_id")) {
             data->visual_state->hovered_intersection = -1;
        }
    }
}

// FIX: Added function definition for drawing valid destinations
static void plateau_draw_valid_destinations(PlateauRenderData* data) {
    if (!data || !data->visual_state || !data->visual_state->valid_destinations) {
        return;
    }
    for (int i = 0; i < data->visual_state->valid_count; i++) {
        int id = data->visual_state->valid_destinations[i];
        if (id >= 0 && id < NODES) {
            Intersection* intersection = &data->board->nodes[id];
            int x, y;
            plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
            // Draw a semi-transparent green circle as a hint for a valid move
            plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS - 2, 0, 255, 0, 100);
        }
    }
}

static void create_intersection_atomic_elements(PlateauRenderData* data) {
    if (!data || !data->board) return;
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        char atomic_id[64];
        snprintf(atomic_id, sizeof(atomic_id), "plateau-intersection-%d", id);
        AtomicElement* atomic_intersection = atomic_create(atomic_id);
        if (atomic_intersection) {
            int x, y;
            x = intersection->c * 50;
            y = intersection->r * 40;
            int hitbox_size = PIECE_RADIUS * 3;
            atomic_set_position(atomic_intersection, x - hitbox_size / 2, y - hitbox_size / 2);
            atomic_set_size(atomic_intersection, hitbox_size, hitbox_size);
            atomic_set_background_color(atomic_intersection, 255, 0, 0, 30);
            atomic_set_z_index(atomic_intersection, 100);
            atomic_set_custom_data(atomic_intersection, "intersection_id", (void*)(intptr_t)id);
            atomic_set_custom_data(atomic_intersection, "plateau_data", data);
            atomic_set_click_handler(atomic_intersection, on_intersection_click);
            atomic_set_hover_handler(atomic_intersection, on_intersection_hover);
            atomic_set_unhover_handler(atomic_intersection, on_intersection_unhover);
            data->intersection_elements[id] = atomic_intersection;
        }
    }
    printf("✅ %d AtomicElements d'intersections créés avec hitboxes agrandies\n", NODES);
}

static void update_intersection_positions(PlateauRenderData* data) {
    if (!data) return;
    for (int id = 0; id < NODES; id++) {
        if (data->intersection_elements[id]) {
            Intersection* intersection = &data->board->nodes[id];
            int x, y;
            plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
            int hitbox_size = PIECE_RADIUS * 3;
            atomic_set_position(data->intersection_elements[id],
                x - hitbox_size / 2,
                y - hitbox_size / 2);
            extern UITree* ui_get_global_tree(void);
            UITree* tree = ui_get_global_tree();
            if (tree && tree->event_manager) {
                atomic_sync_event_manager_position(data->intersection_elements[id], tree->event_manager);
            }
        }
    }
}

static void plateau_custom_render(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(element, "plateau_data");
    if (!data) return;
    if (!data->texture_black || !data->texture_brown) {
        plateau_load_piece_textures(data, renderer);
    }
    data->renderer = renderer;
    SDL_Rect rect = atomic_get_render_rect(element);
    data->offset_x = rect.x + PLATEAU_MARGIN;
    data->offset_y = rect.y + PLATEAU_MARGIN;
    data->cell_width = (rect.w - 2 * PLATEAU_MARGIN) / (COLS - 1);
    data->cell_height = (rect.h - 2 * PLATEAU_MARGIN) / (ROWS - 1);
    update_intersection_positions(data);
    plateau_draw_all_lines(data);
    plateau_draw_all_intersections(data);
    plateau_draw_all_pieces(data);
    plateau_draw_visual_effects(data);
}

static void plateau_draw_visual_effects(PlateauRenderData* data) {
    if (!data || !data->visual_state) return;
    static int last_selected = -2;
    static int last_hovered = -2;
    if (data->visual_state->selected_intersection != last_selected) {
        printf("🎨 [VISUAL_UPDATE] Sélection: %d -> %d\n",
            last_selected, data->visual_state->selected_intersection);
        last_selected = data->visual_state->selected_intersection;
    }
    if (data->visual_state->hovered_intersection != last_hovered) {
        printf("🎨 [VISUAL_UPDATE] Survol: %d -> %d\n",
            last_hovered, data->visual_state->hovered_intersection);
        last_hovered = data->visual_state->hovered_intersection;
    }
    if (data->visual_state->selected_intersection >= 0) {
        int id = data->visual_state->selected_intersection;
        if (id < NODES) {
            Intersection* intersection = &data->board->nodes[id];
            int x, y;
            plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
            SDL_SetRenderDrawColor(data->renderer, 0, 100, 255, 255);
            for (int t = 0; t < 3; t++) {
                for (int angle = 0; angle < 360; angle += 2) {
                    float rad = angle * 3.14159f / 180.0f;
                    int bx = x + (PIECE_RADIUS + 6 + t) * cosf(rad);
                    int by = y + (PIECE_RADIUS + 6 + t) * sinf(rad);
                    SDL_RenderDrawPoint(data->renderer, bx, by);
                }
            }
        }
    }
    if (data->visual_state->hovered_intersection >= 0 &&
        data->visual_state->hovered_intersection != data->visual_state->selected_intersection) {
        int id = data->visual_state->hovered_intersection;
        if (id < NODES) {
            Intersection* intersection = &data->board->nodes[id];
            int x, y;
            plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
            SDL_SetRenderDrawColor(data->renderer, 255, 215, 0, 200);
            for (int t = 0; t < 2; t++) {
                for (int angle = 0; angle < 360; angle += 3) {
                    float rad = angle * 3.14159f / 180.0f;
                    int bx = x + (PIECE_RADIUS + 4 + t) * cosf(rad);
                    int by = y + (PIECE_RADIUS + 4 + t) * sinf(rad);
                    SDL_RenderDrawPoint(data->renderer, bx, by);
                }
            }
        }
    }
    plateau_draw_valid_destinations(data);
}

static void on_intersection_click(void* element, SDL_Event* event) {
    (void)event;
    AtomicElement* atomic = (AtomicElement*)element;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(atomic, "plateau_data");
    int intersection_id = (int)(intptr_t)atomic_get_custom_data(atomic, "intersection_id");

    if (!data || intersection_id < 0 || intersection_id >= NODES) {
        printf("❌ [PLATEAU_CLICK] Données invalides: data=%p, id=%d\n", (void*)data, intersection_id);
        return;
    }
    if (!data->visual_state) {
        printf("❌ [PLATEAU_CLICK] État visuel non initialisé\n");
        return;
    }

    Piece* piece = data->board->nodes[intersection_id].piece;

    if (data->visual_state->selected_intersection >= 0 &&
        data->visual_state->selected_intersection != intersection_id) {
        if (is_valid_destination(data, data->visual_state->selected_intersection, intersection_id)) {
            printf("✅ [PLATEAU_MOVE] Mouvement valide: %d → %d\n",
                data->visual_state->selected_intersection, intersection_id);
            execute_animated_move(data, data->visual_state->selected_intersection, intersection_id);
            data->visual_state->selected_intersection = -1;
            if (data->visual_state->valid_destinations) {
                free(data->visual_state->valid_destinations);
                data->visual_state->valid_destinations = NULL;
            }
            data->visual_state->valid_count = 0;
            return;
        }
        else {
            printf("🚫 [PLATEAU_MOVE] Mouvement invalide: %d → %d\n",
                data->visual_state->selected_intersection, intersection_id);
        }
    }

    if (piece && piece->alive) {
        GameLogic* game_logic = (GameLogic*)data->game_logic;
        if (game_logic) {
            if (!game_logic_can_select_piece(game_logic, piece->owner)) {
                printf("🚫 [PLATEAU_CLICK] Interaction refusée - Ce n'est pas le tour du propriétaire de cette pièce\n");
                printf("   📍 Pièce: %s, Tour actuel: Joueur %d\n",
                    piece->owner == WHITE ? "Blanc" : "Noir",
                    game_logic->current_player);
                return;
            }
        }
    }

    printf("🎯 [PLATEAU_CLICK] DÉBUT - Intersection %d cliquée\n", intersection_id);
    printf("   📍 Position: (r=%d, c=%d)\n",
        data->board->nodes[intersection_id].r,
        data->board->nodes[intersection_id].c);
    printf("   📊 Sélection actuelle AVANT: %d\n", data->visual_state->selected_intersection);

    if (data->visual_state->selected_intersection == intersection_id) {
        data->visual_state->selected_intersection = -1;
        if (data->visual_state->valid_destinations) {
            free(data->visual_state->valid_destinations);
            data->visual_state->valid_destinations = NULL;
        }
        data->visual_state->valid_count = 0;
        printf("🔄 [PLATEAU_CLICK] DÉSELECTION intersection %d\n", intersection_id);
    }
    else {
        int old_selection = data->visual_state->selected_intersection;
        data->visual_state->selected_intersection = intersection_id;
        if (piece && piece->alive) {
            calculate_valid_destinations(data, intersection_id);
        }
        else {
            if (data->visual_state->valid_destinations) {
                free(data->visual_state->valid_destinations);
                data->visual_state->valid_destinations = NULL;
            }
            data->visual_state->valid_count = 0;
        }
        printf("✅ [PLATEAU_CLICK] NOUVELLE SÉLECTION: %d -> %d\n", old_selection, intersection_id);
        if (piece && piece->alive) {
            printf("🔵 [PLATEAU_CLICK] PIÈCE sélectionnée - Propriétaire: %s\n",
                piece->owner == WHITE ? "Blanc" : "Noir");
        }
        else {
            printf("⚫ [PLATEAU_CLICK] Intersection VIDE sélectionnée\n");
        }
    }

    printf("   📊 Sélection actuelle APRÈS: %d\n", data->visual_state->selected_intersection);
    printf("✅ [PLATEAU_CLICK] FIN - État de sélection mis à jour\n\n");
}

static bool is_valid_destination(PlateauRenderData* data, int from_id, int to_id) {
    (void)from_id;
    if (!data || !data->visual_state) return false;
    for (int i = 0; i < data->visual_state->valid_count; i++) {
        if (data->visual_state->valid_destinations[i] == to_id) {
            return true;
        }
    }
    return false;
}

static void execute_animated_move(PlateauRenderData* data, int from_id, int to_id) {
    if (!data || !data->board) return;
    printf("🎬 [ANIMATE_MOVE] Démarrage animation: %d → %d\n", from_id, to_id);
    Intersection* from_intersection = &data->board->nodes[from_id];
    Intersection* to_intersection = &data->board->nodes[to_id];
    int from_x, from_y, to_x, to_y;
    plateau_logical_to_screen(data, from_intersection->r, from_intersection->c, &from_x, &from_y);
    plateau_logical_to_screen(data, to_intersection->r, to_intersection->c, &to_x, &to_y);
    printf("   📍 Animation: (%d,%d) → (%d,%d) pixels\n", from_x, from_y, to_x, to_y);
    apply_move_to_board(data, from_id, to_id);
    printf("✅ [ANIMATE_MOVE] Mouvement appliqué avec animation\n");
}

static void apply_capture(PlateauRenderData* data, Move* move) {
    if (!data || !move || !move->is_capture) return;
    printf("\n┌────────────────────────────────────────────────────────┐\n");
    printf("│ 💥 CAPTURE DÉTECTÉE\n");
    printf("├────────────────────────────────────────────────────────┤\n");
    printf("│ 📍 Mouvement: intersection %d → %d\n", move->from_id, move->to_id);
    printf("│ 🎯 Nombre de captures: %d\n", move->capture_count);
    printf("│ 📋 Pièces capturées: ");
    for (int i = 0; i < move->capture_count; i++) {
        int cap_id = move->captured_ids[i];
        Piece* captured = data->board->nodes[cap_id].piece;
        if (captured) {
            printf("%d(%s) ", cap_id, captured->owner == WHITE ? "○" : "●");
        }
    }
    printf("\n└────────────────────────────────────────────────────────┘\n");
    for (int i = 0; i < move->capture_count; i++) {
        int cap_id = move->captured_ids[i];
        Piece* captured = data->board->nodes[cap_id].piece;
        if (captured) {
            printf("   🗑️  Suppression pièce %d à l'intersection %d\n",
                captured->id, cap_id);
            captured->alive = 0;
            data->board->nodes[cap_id].piece = NULL;
            animate_piece_capture(data, cap_id);
        }
    }
}

static void apply_move_to_board(PlateauRenderData* data, int from_id, int to_id) {
    if (!data || !data->board) return;
    Intersection* from = &data->board->nodes[from_id];
    Intersection* to = &data->board->nodes[to_id];
    if (!from->piece || !from->piece->alive) {
        printf("❌ [APPLY_MOVE] Pas de pièce à déplacer sur %d\n", from_id);
        return;
    }
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ 🎮 MOUVEMENT APPLIQUÉ\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ 📍 Pièce %d: intersection %d → %d\n", from->piece->id, from_id, to_id);
    printf("║ 🎨 Propriétaire: %s\n", from->piece->owner == WHITE ? "○ Blanc" : "● Noir");
    Move move;
    int capture_type = detect_capture(data->board, from_id, to_id, &move);
    bool is_capture = move.is_capture && move.capture_count > 0;
    if (is_capture) {
        printf("║ 💥 TYPE: CAPTURE (%s)\n",
            capture_type == 1 ? "Percussion" : "Aspiration");
        apply_capture(data, &move);
        if (data->visual_state) {
            int fr, fc, tr, tc;
            rc_from_id(from_id, &fr, &fc);
            rc_from_id(to_id, &tr, &tc);
            data->visual_state->last_capture_direction.vr = tr - fr;
            data->visual_state->last_capture_direction.vc = tc - fc;
            if (data->visual_state->visited_count < MAX_VISITED_POSITIONS) {
                data->visual_state->visited_positions[data->visual_state->visited_count++] = to_id;
            }
        }
    }
    else {
        printf("║ 🚶 TYPE: PAIKA (déplacement simple)\n");
    }
    to->piece = from->piece;
    from->piece = NULL;
    to->piece->r = to->r;
    to->piece->c = to->c;
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    log_board_state(data->board, "APRÈS MOUVEMENT");
    if (is_capture) {
        bool more_captures = has_additional_captures(data, to_id);
        if (more_captures) {
            printf("\n┌────────────────────────────────────────────────────────┐\n");
            printf("│ ⚠️  CAPTURES SUPPLÉMENTAIRES DISPONIBLES\n");
            printf("│ 🔗 Chaîne de captures activée\n");
            printf("│ ⏳ Le tour continue pour le même joueur\n");
            printf("└────────────────────────────────────────────────────────┘\n\n");
            if (data->visual_state) {
                data->visual_state->in_capture_chain = true;
                data->visual_state->capture_chain_from = to_id;
                data->visual_state->selected_intersection = to_id;
                calculate_valid_destinations(data, to_id);
            }
            return;
        }
        else {
            printf("\n┌────────────────────────────────────────────────────────┐\n");
            printf("│ ✅ Aucune capture supplémentaire possible\n");
            printf("│ 🔄 Fin de la chaîne de captures\n");
            printf("└────────────────────────────────────────────────────────┘\n\n");
            if (data->visual_state) {
                data->visual_state->in_capture_chain = false;
                data->visual_state->visited_count = 0;
            }
        }
    }
    GameLogic* game_logic = (GameLogic*)data->game_logic;
    if (game_logic) {
        GamePlayer* before = game_logic_get_current_player_info(game_logic);
        game_logic_switch_turn(game_logic);
        GamePlayer* after = game_logic_get_current_player_info(game_logic);
        printf("\n┌────────────────────────────────────────────────────────┐\n");
        printf("│ 🔄 CHANGEMENT DE TOUR\n");
        printf("│ 👤 Avant: %s\n", before->name);
        printf("│ 👤 Après: %s\n", after->name);
        printf("└────────────────────────────────────────────────────────┘\n\n");
    }
}

static void calculate_valid_destinations(PlateauRenderData* data, int piece_id) {
    if (!data || !data->visual_state || !data->board) return;
    if (data->visual_state->valid_destinations) {
        free(data->visual_state->valid_destinations);
        data->visual_state->valid_destinations = NULL;
    }
    data->visual_state->valid_count = 0;
    Intersection* intersection = &data->board->nodes[piece_id];
    if (!intersection->piece || !intersection->piece->alive) return;
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ 🎯 CALCUL DES DESTINATIONS VALIDES\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ 📍 Pièce à l'intersection: %d\n", piece_id);
    printf("║ 🎨 Propriétaire: %s\n",
        intersection->piece->owner == WHITE ? "○ Blanc" : "● Noir");
    printf("║ 🔗 En chaîne de captures: %s\n",
        data->visual_state->in_capture_chain ? "OUI" : "NON");
    Move possible_moves[MAX_MOVES];
    int move_count = generate_moves(data->board, intersection->piece->owner, possible_moves, MAX_MOVES);
    printf("║ 📋 Coups générés au total: %d\n", move_count);
    int* destinations = malloc(sizeof(int) * move_count);
    int dest_count = 0;
    for (int i = 0; i < move_count; i++) {
        if (possible_moves[i].from_id == piece_id) {
            if (data->visual_state->in_capture_chain) {
                int fr, fc, tr, tc;
                rc_from_id(piece_id, &fr, &fc);
                rc_from_id(possible_moves[i].to_id, &tr, &tc);
                Direction new_dir = { tr - fr, tc - fc };
                if (directions_equal(&new_dir, &data->visual_state->last_capture_direction)) {
                    printf("║   ❌ Coup %d→%d rejeté (même direction)\n",
                        piece_id, possible_moves[i].to_id);
                    continue;
                }
                if (is_position_visited(possible_moves[i].to_id,
                    data->visual_state->visited_positions,
                    data->visual_state->visited_count)) {
                    printf("║   ❌ Coup %d→%d rejeté (position déjà visitée)\n",
                        piece_id, possible_moves[i].to_id);
                    continue;
                }
            }
            bool already_added = false;
            for (int j = 0; j < dest_count; j++) {
                if (destinations[j] == possible_moves[i].to_id) {
                    already_added = true;
                    break;
                }
            }
            if (!already_added) {
                destinations[dest_count++] = possible_moves[i].to_id;
                printf("║   ✅ Destination valide: %d %s\n",
                    possible_moves[i].to_id,
                    possible_moves[i].is_capture ? "(CAPTURE)" : "(PAIKA)");
            }
        }
    }
    data->visual_state->valid_destinations = destinations;
    data->visual_state->valid_count = dest_count;
    printf("║ 🎯 Total destinations valides: %d\n", dest_count);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
}

// FIX: Corrected function signature to use pointers for consistency.
static void log_board_state(Board* board, const char* context) {
    if (!board) return;
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ 📊 ÉTAT DU PLATEAU - %s\n", context);
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║     ");
    for (int c = 0; c < COLS; ++c) printf(" %d", c);
    printf("\n");
    printf("║   ┌");
    for (int c = 0; c < COLS; ++c) printf("──");
    printf("┐\n");
    for (int r = 0; r < ROWS; ++r) {
        printf("║ %d │", r);
        for (int c = 0; c < COLS; ++c) {
            int id = node_id(r, c);
            Piece* pc = board->nodes[id].piece;
            if (pc == NULL) {
                printf(" ·");
            }
            else if (pc->owner == WHITE) {
                printf(" ○");
            }
            else {
                printf(" ●");
            }
        }
        printf(" │\n");
    }
    printf("║   └");
    for (int c = 0; c < COLS; ++c) printf("──");
    printf("┘\n");
    int white_count = 0, black_count = 0;
    for (int i = 0; i < NODES; i++) {
        Piece* pc = board->nodes[i].piece;
        if (pc && pc->alive) {
            if (pc->owner == WHITE) white_count++;
            else black_count++;
        }
    }
    printf("║ 📈 Pièces: ○ Blanc=%d  ● Noir=%d\n", white_count, black_count);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
}

// FIX: Corrected signature to use a pointer and removed the unused variable.
static bool has_additional_captures(PlateauRenderData* data, int from_id) {
    if (!data || !data->board) return false;
    Intersection* from = &data->board->nodes[from_id];
    if (!from->piece) return false;
    // FIX: Removed unused variable 'current_player'
    for (int i = 0; i < from->nnei; i++) {
        int neighbor_id = from->neighbors[i];
        if (data->board->nodes[neighbor_id].piece != NULL) continue;
        Move test_move;
        if (detect_capture(data->board, from_id, neighbor_id, &test_move)) {
            if (data->visual_state && data->visual_state->in_capture_chain) {
                int fr, fc, tr, tc;
                rc_from_id(from_id, &fr, &fc);
                rc_from_id(neighbor_id, &tr, &tc);
                Direction new_dir = { tr - fr, tc - fc };
                if (directions_equal(&new_dir, &data->visual_state->last_capture_direction)) {
                    continue;
                }
                if (is_position_visited(neighbor_id, data->visual_state->visited_positions,
                    data->visual_state->visited_count)) {
                    continue;
                }
            }
            return true;
        }
    }
    return false;
}

// === PUBLIC API FUNCTIONS ===
void ui_plateau_set_players(UINode* plateau, GamePlayer* player1, GamePlayer* player2) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->player1 = player1;
        data->player2 = player2;
    }
}

GamePlayer* ui_plateau_get_player1(UINode* plateau) {
    if (!plateau) return NULL;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    return data ? data->player1 : NULL;
}

GamePlayer* ui_plateau_get_player2(UINode* plateau) {
    if (!plateau) return NULL;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    return data ? data->player2 : NULL;
}

void ui_plateau_set_game_logic(UINode* plateau, void* game_logic) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->game_logic = game_logic;
    }
}

void* ui_plateau_get_game_logic(UINode* plateau) {
    if (!plateau) return NULL;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    return data ? data->game_logic : NULL;
}

void ui_plateau_cleanup(UINode* plateau) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        if (data->visual_state && data->visual_state->valid_destinations) {
            free(data->visual_state->valid_destinations);
            data->visual_state->valid_destinations = NULL;
        }
        for (int i = 0; i < NODES; i++) {
            if (data->intersection_elements[i]) {
                atomic_destroy_safe(data->intersection_elements[i]);
                data->intersection_elements[i] = NULL;
            }
        }
        if (data->visual_state) {
            free(data->visual_state);
        }
        free(data);
    }
}

void ui_plateau_container_destroy(UINode* plateau_container) {
    if (plateau_container) ui_plateau_cleanup(plateau_container);
}

// Animation API functions
void ui_plateau_animate_piece_move(UINode* plateau, int from_id, int to_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_move(data, from_id, to_id);
}

void ui_plateau_animate_piece_capture(UINode* plateau, int piece_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_capture(data, piece_id);
}

void ui_plateau_animate_piece_placement(UINode* plateau, int intersection_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_placement(data, intersection_id);
}

void ui_plateau_animate_piece_selection(UINode* plateau, int piece_id) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_piece_selection(data, piece_id);
}

void ui_plateau_animate_victory_dance(UINode* plateau, int winning_player) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_victory_dance(data, (Player)winning_player);
}

void ui_plateau_animate_defeat_fade(UINode* plateau, int losing_player) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_defeat_fade(data, (Player)losing_player);
}

void ui_plateau_animate_initial_wave(UINode* plateau) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) animate_initial_piece_wave(data);
}

void ui_plateau_set_mouse_handlers(UINode* plateau) {
    if (!plateau) return;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (!data) return;
    printf("🖱️ Mouse handlers plateau connectés (intersections atomiques prêtes)\n");
}

UINode* ui_plateau_container_with_players(UITree* tree, const char* id, GamePlayer* player1, GamePlayer* player2) {
    if (!tree) return NULL;
    UINode* plateau_container = ui_div(tree, id);
    if (!plateau_container) return NULL;
    SET_SIZE(plateau_container, PLATEAU_VISUAL_WIDTH, PLATEAU_VISUAL_HEIGHT);
    atomic_set_background_color(plateau_container->element, 0, 0, 0, 0);
    atomic_set_border(plateau_container->element, 3, 255, 0, 0, 255);
    Board* board = (Board*)malloc(sizeof(Board));
    if (!board) return plateau_container;
    board_init(board);
    PlateauRenderData* data = calloc(1, sizeof(PlateauRenderData));
    if (data) {
        data->board = board;
        data->show_intersections = true;
        data->show_coordinates = false;
        data->player1 = player1;
        data->player2 = player2;
        data->visual_state = calloc(1, sizeof(VisualFeedbackState));
        if (data->visual_state) {
            data->visual_state->hovered_intersection = -1;
            data->visual_state->selected_intersection = -1;
            data->visual_state->in_capture_chain = false;
            data->visual_state->capture_chain_from = -1;
            data->visual_state->visited_count = 0;
        }
        create_intersection_atomic_elements(data);
        if (tree->event_manager) {
            for (int i = 0; i < NODES; i++) {
                if (data->intersection_elements[i]) {
                    atomic_register_with_event_manager(data->intersection_elements[i], tree->event_manager);
                }
            }
            printf("✅ %d intersections enregistrées dans EventManager\n", NODES);
        }
        else {
            printf("❌ EventManager non disponible dans tree\n");
        }
        atomic_set_custom_data(plateau_container->element, "plateau_data", data);
        atomic_set_custom_data(plateau_container->element, "board", board);
        atomic_set_custom_render(plateau_container->element, plateau_custom_render);
        printf("✅ Plateau créé avec %d pions, rendu via custom_render + intersections interactives\n", board->piece_count);
    }
    else {
        free(board);
    }
    return plateau_container;
}

UINode* ui_plateau_container(UITree* tree, const char* id) {
    return ui_plateau_container_with_players(tree, id, NULL, NULL);
}

UINode* ui_plateau_container_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* plateau = ui_plateau_container(tree, id);
    if (plateau) {
        SET_SIZE(plateau, width, height);
    }
    return plateau;
}

// === DEBUG FUNCTIONS ===
void ui_plateau_debug_intersections(UINode* plateau) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (!data) {
        printf("❌ [PLATEAU_DEBUG] Pas de données plateau\n");
        return;
    }
    
    printf("\n=== 🔍 DEBUG INTERSECTIONS PLATEAU ===\n");
    printf("Nombre d'intersections créées: %d\n", NODES);
    
    int active_count = 0;
    int with_handlers = 0;
    
    for (int i = 0; i < NODES; i++) {
        if (data->intersection_elements[i]) {
            active_count++;
            
            AtomicElement* elem = data->intersection_elements[i];
            bool has_click = (elem->events.on_click != NULL);
            bool has_hover = (elem->events.on_hover != NULL);
            bool has_unhover = (elem->events.on_unhover != NULL);
            
            if (has_click || has_hover || has_unhover) {
                with_handlers++;
            }
            
            if (i < 5) {
                printf("  Intersection %d: ID=%s, handlers(c:%s h:%s u:%s)\n",
                       i, elem->id ? elem->id : "NULL",
                       has_click ? "✓" : "✗",
                       has_hover ? "✓" : "✗", 
                       has_unhover ? "✓" : "✗");
            }
        }
    }
    
    printf("Résumé: %d actives, %d avec handlers\n", active_count, with_handlers);
    printf("=====================================\n\n");
}

void ui_plateau_debug_current_selection(UINode* plateau) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (!data || !data->visual_state) {
        printf("❌ [PLATEAU_DEBUG] Pas d'état visuel\n");
        return;
    }
    
    printf("🎯 [PLATEAU_SELECTION] État complet:\n");
    printf("   ✅ Visual state: %p\n", (void*)data->visual_state);
    printf("   📍 Sélection: %d", data->visual_state->selected_intersection);
    
    if (data->visual_state->selected_intersection >= 0) {
        int id = data->visual_state->selected_intersection;
        if (id < NODES) {
            Intersection* intersection = &data->board->nodes[id];
            printf(" ➜ (r=%d, c=%d)", intersection->r, intersection->c);
            
            if (intersection->piece && intersection->piece->alive) {
                printf(" ➜ PIÈCE %s", intersection->piece->owner == WHITE ? "BLANCHE" : "NOIRE");
            } else {
                printf(" ➜ VIDE");
            }
        }
    }
    printf("\n");
    
    printf("   🌟 Survol: %d", data->visual_state->hovered_intersection);
    if (data->visual_state->hovered_intersection >= 0) {
        int id = data->visual_state->hovered_intersection;
        if (id < NODES) {
            Intersection* intersection = &data->board->nodes[id];
            printf(" ➜ (r=%d, c=%d)", intersection->r, intersection->c);
        }
    }
    printf("\n");
    printf("   🎨 Rendu custom: %s\n", data->board ? "ACTIF" : "INACTIF");
    printf("=====================================\n");
}

void ui_plateau_debug_visual_state(UINode* plateau) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (!data) return;
    
    printf("👁️ [PLATEAU_VISUAL] État visuel du plateau:\n");
    printf("   Board: %p\n", (void*)data->board);
    printf("   Visual state: %p\n", (void*)data->visual_state);
    printf("   Game logic: %p\n", data->game_logic);
    printf("   Intersections visibles: %s\n", data->show_intersections ? "OUI" : "NON");
}

void ui_plateau_register_events(UINode* plateau, EventManager* event_manager) {
    if (!plateau || !event_manager) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (!data) return;
    
    int registered_count = 0;
    for (int i = 0; i < NODES; i++) {
        if (data->intersection_elements[i]) {
            atomic_register_with_event_manager(data->intersection_elements[i], event_manager);
            registered_count++;
        }
    }
    
    printf("🔗 Plateau '%s': %d intersections enregistrées dans EventManager\n", 
           plateau->id, registered_count);
}