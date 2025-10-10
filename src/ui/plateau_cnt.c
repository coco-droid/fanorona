#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"
#include "../utils/asset_manager.h"
#include "../plateau/plateau.h"
#include "../pions/pions.h"
#include "../logic/rules.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// === PLATEAU COMPONENT ===

// Dimensions du plateau visuelles
#define PLATEAU_VISUAL_WIDTH 480
#define PLATEAU_VISUAL_HEIGHT 320
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
#define INTERSECTION_COLOR_R 101
#define INTERSECTION_COLOR_G 67
#define INTERSECTION_COLOR_B 33
#define WHITE_PIECE_R 240
#define WHITE_PIECE_G 240
#define WHITE_PIECE_B 240
#define BLACK_PIECE_R 30
#define BLACK_PIECE_G 30
#define BLACK_PIECE_B 30

// 🆕 VISUAL FEEDBACK STRUCTURES
typedef struct VisualFeedbackState {
    int hovered_intersection;
    int selected_intersection;
    int* valid_destinations;
    int valid_count;
    float animation_timer;
    char error_message[128];
    float error_display_timer;
    int error_type;
} VisualFeedbackState;

// Forward declaration
typedef struct PlateauRenderData PlateauRenderData;

typedef struct IntersectionElement {
    UINode* ui_node;
    int intersection_id;
    int screen_x, screen_y;
    bool is_hovered;
    bool is_selected;
    PlateauRenderData* plateau_data;
} IntersectionElement;

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
    IntersectionElement* intersection_elements[NODES];
} PlateauRenderData;

// === BUG FIX: FORWARD DECLARATIONS ===
// These declarations inform the compiler about the animation functions
// before they are called, fixing the implicit declaration errors.
static void animate_piece_move(PlateauRenderData* data, int from_id, int to_id);
static void animate_piece_selection(PlateauRenderData* data, int piece_id);
static void animate_piece_capture(PlateauRenderData* data, int piece_id);
static void animate_piece_placement(PlateauRenderData* data, int intersection_id);
static void animate_victory_dance(PlateauRenderData* data, Player winning_player);
static void animate_defeat_fade(PlateauRenderData* data, Player losing_player);
static void animate_initial_piece_wave(PlateauRenderData* data);


// 🆕 ERROR MESSAGES
static const char* error_messages[] = {
    [0] = "⚠️ Vous devez capturer quand c'est possible !",
    [1] = "⚠️ Impossible de capturer 2x dans la même direction !",
    [2] = "⚠️ Position déjà visitée pendant cette chaîne !",
    [3] = "⚠️ Les cases doivent être adjacentes !",
    [4] = "⚠️ Ce n'est pas votre pion !"
};

// === FONCTIONS UTILITAIRES ===

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
    
    // Dessiner ligne avec épaisseur
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
            if (x*x + y*y <= radius*radius) {
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
        // Intersection forte = cercle plus grand, faible = cercle plus petit
        int radius = is_strong ? INTERSECTION_RADIUS : INTERSECTION_RADIUS - 2;
        int color_intensity = is_strong ? 255 : 180;
        
        plateau_draw_filled_circle(data->renderer, x, y, radius, 
                                 INTERSECTION_COLOR_R, INTERSECTION_COLOR_G, INTERSECTION_COLOR_B, color_intensity);
    }
}

// 🆕 VISUAL FEEDBACK FUNCTIONS (moved before first use)
static void plateau_init_visual_feedback(PlateauRenderData* data) {
    data->visual_state = calloc(1, sizeof(VisualFeedbackState));
    VisualFeedbackState* vf = data->visual_state;
    
    vf->hovered_intersection = -1;
    vf->selected_intersection = -1;
    vf->valid_destinations = calloc(NODES, sizeof(int));
    vf->valid_count = 0;
    vf->animation_timer = 0.0f;
    vf->error_display_timer = 0.0f;
    
    printf("✅ Visual feedback system initialized\n");
}

// 🆕 Initialiser les joueurs de test
static void plateau_init_test_players(PlateauRenderData* data) {
    // Créer joueur 1 (pions noirs)
    data->player1 = player_create("Joueur 1", WHITE, PIECE_COLOR_BLACK, PLAYER_TYPE_HUMAN, 1);
    
    // Créer joueur 2 (pions bruns)
    data->player2 = player_create("Joueur 2", BLACK, PIECE_COLOR_BROWN, PLAYER_TYPE_HUMAN, 2);
    
    printf("🎮 Joueurs de test initialisés:\n");
    printf("   👤 %s: %s (%s)\n", data->player1->name, 
           data->player1->logical_color == WHITE ? "Blanc" : "Noir",
           piece_color_to_string(data->player1->piece_color));
    printf("   👤 %s: %s (%s)\n", data->player2->name, 
           data->player2->logical_color == WHITE ? "Blanc" : "Noir",
           piece_color_to_string(data->player2->piece_color));
    
    // 🆕 Initialize visual feedback after players
    plateau_init_visual_feedback(data);
}

// 🆕 Charger les textures des pions
static void plateau_load_piece_textures(PlateauRenderData* data, SDL_Renderer* renderer) {
    // Charger texture pions noirs
    data->texture_black = asset_load_texture(renderer, "piece_black.png");
    if (!data->texture_black) {
        printf("❌ Impossible de charger piece_black.png\n");
    } else {
        printf("✅ Texture piece_black.png chargée\n");
    }
    
    // Charger texture pions bruns
    data->texture_brown = asset_load_texture(renderer, "piece_brun.png");
    if (!data->texture_brown) {
        printf("❌ Impossible de charger piece_brun.png\n");
    } else {
        printf("✅ Texture piece_brun.png chargée\n");
    }
    
    // Assigner les textures aux joueurs
    if (data->player1 && data->player1->piece_color == PIECE_COLOR_BLACK) {
        // Player1 utilise les pions noirs
        printf("🎨 Joueur 1 utilise les pions noirs\n");
    }
    
    if (data->player2 && data->player2->piece_color == PIECE_COLOR_BROWN) {
        // Player2 utilise les pions bruns
        printf("🎨 Joueur 2 utilise les pions bruns\n");
    }
}

// 🔧 Modifier dessiner un pion pour utiliser les textures
static void plateau_draw_piece(PlateauRenderData* data, int r, int c, Player owner) {
    int x, y;
    plateau_logical_to_screen(data, r, c, &x, &y);
    
    SDL_Texture* texture_to_use = NULL;
    
    // Déterminer quelle texture utiliser selon le propriétaire et les joueurs
    if (owner == WHITE && data->player1 && data->player1->logical_color == WHITE) {
        // Les blancs appartiennent au player1
        texture_to_use = (data->player1->piece_color == PIECE_COLOR_BLACK) ? data->texture_black : data->texture_brown;
    } else if (owner == BLACK && data->player2 && data->player2->logical_color == BLACK) {
        // Les noirs appartiennent au player2
        texture_to_use = (data->player2->piece_color == PIECE_COLOR_BLACK) ? data->texture_black : data->texture_brown;
    } else if (owner == WHITE && data->player2 && data->player2->logical_color == WHITE) {
        // Les blancs appartiennent au player2
        texture_to_use = (data->player2->piece_color == PIECE_COLOR_BLACK) ? data->texture_black : data->texture_brown;
    } else if (owner == BLACK && data->player1 && data->player1->logical_color == BLACK) {
        // Les noirs appartiennent au player1
        texture_to_use = (data->player1->piece_color == PIECE_COLOR_BLACK) ? data->texture_black : data->texture_brown;
    }
    
    if (texture_to_use) {
        // Dessiner avec texture
        SDL_Rect dest_rect = {x - PIECE_RADIUS, y - PIECE_RADIUS, PIECE_RADIUS * 2, PIECE_RADIUS * 2};
        SDL_RenderCopy(data->renderer, texture_to_use, NULL, &dest_rect);
    } else {
        // Fallback: dessiner cercle coloré
        if (owner == WHITE) {
            plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS, 
                                     WHITE_PIECE_R, WHITE_PIECE_G, WHITE_PIECE_B, 255);
        } else if (owner == BLACK) {
            plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS, 
                                     BLACK_PIECE_R, BLACK_PIECE_G, BLACK_PIECE_B, 255);
        }
    }
}

// === FONCTIONS DE RENDU PRINCIPAL ===

// Dessiner toutes les lignes du plateau
static void plateau_draw_all_lines(PlateauRenderData* data) {
    Board* board = data->board;
    
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &board->nodes[id];
        int r = intersection->r;
        int c = intersection->c;
        
        // Parcourir tous les voisins et dessiner les lignes
        for (int i = 0; i < intersection->nnei; i++) {
            int neighbor_id = intersection->neighbors[i];
            Intersection* neighbor = &board->nodes[neighbor_id];
            
            // Éviter de dessiner la même ligne deux fois
            if (neighbor_id > id) {
                plateau_draw_line(data, r, c, neighbor->r, neighbor->c);
            }
        }
    }
}

// Dessiner toutes les intersections
static void plateau_draw_all_intersections(PlateauRenderData* data) {
    Board* board = data->board;
    
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &board->nodes[id];
        int r = intersection->r;
        int c = intersection->c;
        bool is_strong = intersection->strong;
        
        plateau_draw_intersection(data, r, c, is_strong);
    }
}

// Dessiner tous les pions
static void plateau_draw_all_pieces(PlateauRenderData* data) {
    Board* board = data->board;
    
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &board->nodes[id];
        
        if (intersection->piece != NULL && intersection->piece->alive) {
            plateau_draw_piece(data, intersection->r, intersection->c, intersection->piece->owner);
        }
    }
}

// Dessiner les coordonnées (debug)
static void plateau_draw_coordinates(PlateauRenderData* data) {
    if (!data->show_coordinates) return;
    
    // TODO: Implémenter l'affichage des coordonnées avec SDL_ttf
    // Pour l'instant, juste marquer le centre
    int center_r = ROWS / 2;
    int center_c = COLS / 2;
    int x, y;
    plateau_logical_to_screen(data, center_r, center_c, &x, &y);
    
    // Marquer le centre avec un petit carré rouge
    SDL_SetRenderDrawColor(data->renderer, 255, 0, 0, 255);
    SDL_Rect center_mark = {x - 3, y - 3, 6, 6};
    SDL_RenderFillRect(data->renderer, &center_mark);
}

// 🆕 VISUAL FEEDBACK FUNCTIONS

static void plateau_calculate_valid_moves(PlateauRenderData* data, int from_id) {
    VisualFeedbackState* vf = data->visual_state;
    
    vf->valid_count = 0;
    memset(vf->valid_destinations, 0, NODES * sizeof(int));
    
    // Test all intersections as potential destinations
    for (int to_id = 0; to_id < NODES; to_id++) {
        if (to_id == from_id) continue;
        if (data->board->nodes[to_id].piece) continue; // Must be empty
        
        // 🎯 Use your validator with minimal game state (simplified for now)
        int is_valid = is_move_valide(
            data->board,
            from_id,
            to_id,
            data->player1->logical_color, // Current player (simplified)
            NULL,  // No last direction for now
            NULL,  // No visited positions for now
            0,     // No visited count
            0      // Not during capture chain
        );
        
        if (is_valid) {
            vf->valid_destinations[to_id] = 1;
            vf->valid_count++;
        }
    }
    
    printf("✅ Found %d valid destinations for piece %d\n", vf->valid_count, from_id);
}

static void plateau_show_error_feedback(PlateauRenderData* data, int error_type) {
    VisualFeedbackState* vf = data->visual_state;
    
    if (error_type >= 0 && error_type < 5) {
        strncpy(vf->error_message, error_messages[error_type], 127);
        vf->error_message[127] = '\0';
        vf->error_display_timer = 2.5f; // Display for 2.5 seconds
        vf->error_type = error_type;
        
        printf("❌ Error feedback: %s\n", vf->error_message);
    }
}

static void plateau_render_hover_feedback(PlateauRenderData* data) {
    VisualFeedbackState* vf = data->visual_state;
    if (vf->hovered_intersection < 0) return;
    
    Intersection* intersection = &data->board->nodes[vf->hovered_intersection];
    int x, y;
    plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
    
    // 🔧 FIX: Vérifier si l'intersection contient un pion de l'utilisateur
    bool is_user_piece = (intersection->piece != NULL && 
                         intersection->piece->owner == data->player1->logical_color);
    
    if (is_user_piece) {
        // 🆕 DESIGN: Cercle jaune rond pour les pions utilisateur
        float pulse = (sin(vf->animation_timer * 6.0f) + 1.0f) * 0.5f; // 0-1
        int halo_radius = PIECE_RADIUS + 8 + (int)(pulse * 4);
        int alpha = 120 + (int)(pulse * 100); // 120-220 pour bien visible
        
        // 🎯 Cercle jaune vif pour les pions utilisateur
        plateau_draw_filled_circle(data->renderer, x, y, halo_radius, 255, 255, 0, alpha);
        
        // 🆕 Bordure dorée pour plus de visibilité
        for (int i = 0; i < 3; i++) {
            plateau_draw_filled_circle(data->renderer, x, y, halo_radius + i, 255, 215, 0, 80);
        }
        
        // 🆕 DEBUG: Confirmer le rendu
        static int render_debug_counter = 0;
        if (render_debug_counter++ % 30 == 0) { // Log toutes les 30 frames
            printf("🌟 Rendering YELLOW hover for user piece at (%d,%d) intersection %d\n", 
                   intersection->r, intersection->c, vf->hovered_intersection);
        }
    } else {
        // 🔧 Hover standard pour les intersections vides ou pions adverses
        float pulse = (sin(vf->animation_timer * 4.0f) + 1.0f) * 0.5f;
        int halo_radius = INTERSECTION_RADIUS + 3 + (int)(pulse * 2);
        int alpha = 60 + (int)(pulse * 60);
        
        // Cercle blanc translucide pour les autres intersections
        plateau_draw_filled_circle(data->renderer, x, y, halo_radius, 255, 255, 255, alpha);
    }
}

static void plateau_render_selection_feedback(PlateauRenderData* data) {
    VisualFeedbackState* vf = data->visual_state;
    if (vf->selected_intersection < 0) return;
    
    Intersection* intersection = &data->board->nodes[vf->selected_intersection];
    int x, y;
    plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
    
    // Blue selection ring
    SDL_SetRenderDrawColor(data->renderer, 100, 150, 255, 200);
    plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS + 8, 100, 150, 255, 100);
    SDL_SetRenderDrawColor(data->renderer, 100, 150, 255, 255);
    for (int i = 0; i < 3; i++) {
        // Draw ring outline
        // Simplified ring drawing
        plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS + 6 + i, 100, 150, 255, 50);
    }
}

static void plateau_render_valid_destinations(PlateauRenderData* data) {
    VisualFeedbackState* vf = data->visual_state;
    if (vf->selected_intersection < 0) return;
    
    for (int id = 0; id < NODES; id++) {
        if (!vf->valid_destinations[id]) continue;
        
        Intersection* intersection = &data->board->nodes[id];
        int x, y;
        plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
        
        // Green circles for valid destinations
        float pulse = (sin(vf->animation_timer * 6.0f) + 1.0f) * 0.5f;
        int alpha = 100 + (int)(pulse * 100);
        
        SDL_SetRenderDrawColor(data->renderer, 100, 255, 100, alpha);
        plateau_draw_filled_circle(data->renderer, x, y, INTERSECTION_RADIUS + 2, 100, 255, 100, alpha);
    }
}

static void plateau_render_error_feedback(PlateauRenderData* data) {
    VisualFeedbackState* vf = data->visual_state;
    if (vf->error_display_timer <= 0.0f) return;
    
    // Simple text rendering (for now, just console output)
    // TODO: Integrate with TTF system for on-screen display
    static float last_error_time = 0.0f;
    if (vf->error_display_timer > last_error_time - 0.1f) {
        printf("💬 [ERROR] %s\n", vf->error_message);
        last_error_time = vf->error_display_timer;
    }
}

static void plateau_update_visual_feedback(PlateauRenderData* data, float delta_time) {
    VisualFeedbackState* vf = data->visual_state;
    
    vf->animation_timer += delta_time * 3.0f; // 🔧 FIX: Augmenter la vitesse d'animation
    if (vf->animation_timer > 6.28f) vf->animation_timer = 0.0f; // Reset at 2π
    
    if (vf->error_display_timer > 0.0f) {
        vf->error_display_timer -= delta_time;
    }
    
    // 🆕 Force un re-rendu si on a un hover actif
    if (vf->hovered_intersection >= 0) {
        // Le rendu sera fait automatiquement par le système
    }
}

// 🆕 NOUVELLES FONCTIONS pour les intersections individuelles
static void intersection_handle_hover(void* element, SDL_Event* event) {
    (void)event;
    
    AtomicElement* atomic = (AtomicElement*)element;
    IntersectionElement* intersection_elem = (IntersectionElement*)atomic_get_custom_data(atomic, "intersection_data");
    if (!intersection_elem) return;
    
    PlateauRenderData* data = intersection_elem->plateau_data;
    int intersection_id = intersection_elem->intersection_id;
    
    // Vérifier si c'est un pion utilisateur
    Intersection* intersection = &data->board->nodes[intersection_id];
    bool is_user_piece = (intersection->piece != NULL && 
                         intersection->piece->owner == data->player1->logical_color);
    
    if (is_user_piece) {
        // Marquer comme hovered seulement les pions utilisateur
        intersection_elem->is_hovered = true;
        data->visual_state->hovered_intersection = intersection_id;
        data->visual_state->animation_timer = 0.0f; // Reset animation
        
        printf("🎯 User piece hovered at intersection %d (%d,%d)\n", 
               intersection_id, intersection->r, intersection->c);
    }
}

static void intersection_handle_unhover(void* element, SDL_Event* event) {
    (void)event;
    
    AtomicElement* atomic = (AtomicElement*)element;
    IntersectionElement* intersection_elem = (IntersectionElement*)atomic_get_custom_data(atomic, "intersection_data");
    if (!intersection_elem) return;
    
    PlateauRenderData* data = intersection_elem->plateau_data;
    int intersection_id = intersection_elem->intersection_id;
    
    if (intersection_elem->is_hovered) {
        intersection_elem->is_hovered = false;
        
        // Reset hover state si c'était l'intersection hovered
        if (data->visual_state->hovered_intersection == intersection_id) {
            data->visual_state->hovered_intersection = -1;
        }
        
        printf("🚫 Intersection %d unhovered\n", intersection_id);
    }
}

static void intersection_handle_click(void* element, SDL_Event* event) {
    (void)event;
    
    AtomicElement* atomic = (AtomicElement*)element;
    IntersectionElement* intersection_elem = (IntersectionElement*)atomic_get_custom_data(atomic, "intersection_data");
    if (!intersection_elem) return;
    
    PlateauRenderData* data = intersection_elem->plateau_data;
    int intersection_id = intersection_elem->intersection_id;
    VisualFeedbackState* vf = data->visual_state;
    
    printf("🖱️ Intersection %d clicked\n", intersection_id);
    
    Intersection* clicked = &data->board->nodes[intersection_id];
    
    if (vf->selected_intersection < 0) {
        // No piece selected - try to select this piece
        if (clicked->piece && clicked->piece->owner == data->player1->logical_color) {
            vf->selected_intersection = intersection_id;
            intersection_elem->is_selected = true;
            plateau_calculate_valid_moves(data, intersection_id);
            
            // Animation de sélection (now properly declared)
            animate_piece_selection(data, intersection_id);
            
            printf("🎯 Piece selected at intersection %d\n", intersection_id);
        } else {
            plateau_show_error_feedback(data, 4); // Wrong piece
            printf("❌ Cannot select intersection %d (no piece or wrong owner)\n", intersection_id);
        }
    } else {
        // Piece already selected - try to move
        if (intersection_id == vf->selected_intersection) {
            // Deselect
            vf->selected_intersection = -1;
            intersection_elem->is_selected = false;
            vf->valid_count = 0;
            printf("🔄 Piece deselected\n");
        } else if (vf->valid_destinations[intersection_id]) {
            // Valid move - execute it with animation
            printf("✅ Valid move from %d to %d\n", vf->selected_intersection, intersection_id);
            
            // 🔧 FIX: Update board state BEFORE animation
            Intersection* from_intersection = &data->board->nodes[vf->selected_intersection];
            Intersection* to_intersection = &data->board->nodes[intersection_id];
            
            // Move the piece in the board logic
            if (from_intersection->piece) {
                to_intersection->piece = from_intersection->piece;
                from_intersection->piece = NULL;
                printf("🔄 Board updated: piece moved from %d to %d\n", vf->selected_intersection, intersection_id);
            }
            
            // Animation de déplacement (now properly declared)
            animate_piece_move(data, vf->selected_intersection, intersection_id);
            
            // Reset selection states
            if (data->intersection_elements[vf->selected_intersection]) {
                data->intersection_elements[vf->selected_intersection]->is_selected = false;
            }
            vf->selected_intersection = -1;
            vf->valid_count = 0;
        } else {
            // Invalid move - show error
            plateau_show_error_feedback(data, 3); // Not adjacent (simplified)
            printf("❌ Invalid move to intersection %d\n", intersection_id);
        }
    }
}

// 🆕 ANIMATION FUNCTIONS FOR PIECES (moved before first use)

// Animation de déplacement d'une pièce
static void animate_piece_move(PlateauRenderData* data, int from_id, int to_id) {
    if (!data || from_id < 0 || from_id >= NODES || to_id < 0 || to_id >= NODES) return;
    
    IntersectionElement* from_elem = data->intersection_elements[from_id];
    IntersectionElement* to_elem = data->intersection_elements[to_id];
    
    if (!from_elem || !to_elem) return;
    
    // Calculer les positions écran
    Intersection* from_intersection = &data->board->nodes[from_id];
    Intersection* to_intersection = &data->board->nodes[to_id];
    
    int from_x = data->offset_x + from_intersection->c * data->cell_width;
    int from_y = data->offset_y + from_intersection->r * data->cell_height;
    int to_x = data->offset_x + to_intersection->c * data->cell_width;
    int to_y = data->offset_y + to_intersection->r * data->cell_height;
    
    // Animation X
    Animation* slide_x = animation_create("piece-move-x", ANIMATION_PROPERTY_X, 0.5f);
    animation_add_keyframe(slide_x, 0.0f, (float)from_x, "ease-out");
    animation_add_keyframe(slide_x, 1.0f, (float)to_x, "ease-out");
    
    // Animation Y
    Animation* slide_y = animation_create("piece-move-y", ANIMATION_PROPERTY_Y, 0.5f);
    animation_add_keyframe(slide_y, 0.0f, (float)from_y, "ease-out");
    animation_add_keyframe(slide_y, 1.0f, (float)to_y, "ease-out");
    
    ui_node_add_animation(from_elem->ui_node, slide_x);
    ui_node_add_animation(from_elem->ui_node, slide_y);
    
    printf("🎬 Piece move animation: (%d,%d) → (%d,%d)\n", 
           from_intersection->r, from_intersection->c,
           to_intersection->r, to_intersection->c);
}

// Animation de sélection (pulse)
static void animate_piece_selection(PlateauRenderData* data, int piece_id) {
    if (!data || piece_id < 0 || piece_id >= NODES) return;
    
    IntersectionElement* elem = data->intersection_elements[piece_id];
    if (!elem) return;
    
    // Animation de pulse (opacity)
    Animation* select_pulse = animation_create("selection-pulse", ANIMATION_PROPERTY_OPACITY, 0.8f);
    animation_add_keyframe(select_pulse, 0.0f, 255.0f, "ease-in-out");
    animation_add_keyframe(select_pulse, 0.5f, 150.0f, "ease-in-out");
    animation_add_keyframe(select_pulse, 1.0f, 255.0f, "ease-in-out");
    
    animation_set_iterations(select_pulse, 3); // 3 pulsations
    
    ui_node_add_animation(elem->ui_node, select_pulse);
    
    printf("🔵 Piece selection animation: intersection %d\n", piece_id);
}

// Animation de capture (disparition)
static void animate_piece_capture(PlateauRenderData* data, int piece_id) {
    if (!data || piece_id < 0 || piece_id >= NODES) return;
    
    IntersectionElement* elem = data->intersection_elements[piece_id];
    if (!elem) return;
    
    // Animation de fade out + scale down
    Animation* fade_out = animation_create("capture-fade", ANIMATION_PROPERTY_OPACITY, 0.4f);
    animation_add_keyframe(fade_out, 0.0f, 255.0f, "ease-in");
    animation_add_keyframe(fade_out, 0.7f, 100.0f, "ease-in");
    animation_add_keyframe(fade_out, 1.0f, 0.0f, "ease-in");
    
    ui_node_add_animation(elem->ui_node, fade_out);
    
    printf("💥 Piece capture animation: intersection %d\n", piece_id);
}

// Animation de placement (apparition)
static void animate_piece_placement(PlateauRenderData* data, int intersection_id) {
    if (!data || intersection_id < 0 || intersection_id >= NODES) return;
    
    IntersectionElement* elem = data->intersection_elements[intersection_id];
    if (!elem) return;
    
    // Animation de fade in + scale up
    Animation* appear = animation_create("piece-appear", ANIMATION_PROPERTY_OPACITY, 0.5f);
    animation_add_keyframe(appear, 0.0f, 0.0f, "ease-out");
    animation_add_keyframe(appear, 0.3f, 150.0f, "ease-out");
    animation_add_keyframe(appear, 1.0f, 255.0f, "ease-out");
    
    ui_node_add_animation(elem->ui_node, appear);
    
    printf("✨ Piece placement animation: intersection %d\n", intersection_id);
}

// Animation de victoire pour toutes les pièces d'un joueur
static void animate_victory_dance(PlateauRenderData* data, Player winning_player) {
    if (!data) return;
    
    printf("🎉 Starting victory dance for player %s\n", winning_player == WHITE ? "WHITE" : "BLACK");
    
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        if (intersection->piece && intersection->piece->owner == winning_player) {
            
            IntersectionElement* elem = data->intersection_elements[id];
            if (!elem) continue;
            
            // Animation de saut + rotation
            Animation* bounce = animation_create("victory-bounce", ANIMATION_PROPERTY_Y, 1.2f);
            animation_add_keyframe(bounce, 0.0f, 0.0f, "ease-out");
            animation_add_keyframe(bounce, 0.3f, -25.0f, "ease-out");
            animation_add_keyframe(bounce, 0.6f, 0.0f, "ease-in");
            animation_add_keyframe(bounce, 0.8f, -15.0f, "ease-out");
            animation_add_keyframe(bounce, 1.0f, 0.0f, "ease-in");
            
            animation_set_iterations(bounce, 3); // Sauter 3 fois
            
            // Délai progressif pour un effet wave
            // TODO: Implémenter activation_delay dans le système d'animation
            
            ui_node_add_animation(elem->ui_node, bounce);
        }
    }
}

// Animation de défaite pour toutes les pièces d'un joueur
static void animate_defeat_fade(PlateauRenderData* data, Player losing_player) {
    if (!data) return;
    
    printf("😞 Starting defeat fade for player %s\n", losing_player == WHITE ? "WHITE" : "BLACK");
    
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        if (intersection->piece && intersection->piece->owner == losing_player) {
            
            IntersectionElement* elem = data->intersection_elements[id];
            if (!elem) continue;
            
            // Animation de disparition lente
            Animation* defeat_fade = animation_create("defeat-fade", ANIMATION_PROPERTY_OPACITY, 2.5f);
            animation_add_keyframe(defeat_fade, 0.0f, 255.0f, "ease-in");
            animation_add_keyframe(defeat_fade, 0.8f, 100.0f, "ease-in");
            animation_add_keyframe(defeat_fade, 1.0f, 30.0f, "ease-in");
            
            ui_node_add_animation(elem->ui_node, defeat_fade);
        }
    }
}

// Animation d'apparition initiale en wave
static void animate_initial_piece_wave(PlateauRenderData* data) {
    if (!data) return;
    
    printf("🌊 Starting initial piece wave animation\n");
    
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        if (intersection->piece) {
            
            IntersectionElement* elem = data->intersection_elements[id];
            if (!elem) continue;
            
            // Animation d'apparition avec délai progressif
            Animation* appear = animation_create("initial-wave", ANIMATION_PROPERTY_OPACITY, 0.6f);
            animation_add_keyframe(appear, 0.0f, 0.0f, "ease-out");
            animation_add_keyframe(appear, 0.4f, 200.0f, "ease-out");
            animation_add_keyframe(appear, 1.0f, 255.0f, "ease-out");
            
            // TODO: Ajouter délai basé sur l'ID : (float)id * 0.05f
            
            ui_node_add_animation(elem->ui_node, appear);
        }
    }
}

// 🆕 FONCTION pour créer les éléments UI individuels des intersections
static void plateau_create_intersection_elements(PlateauRenderData* data, UINode* plateau_container) {
    if (!data || !plateau_container) return;
    
    printf("🔧 Creating individual UI elements for %d intersections\n", NODES);
    
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        
        // Créer un élément UI invisible pour chaque intersection
        char element_id[32];
        snprintf(element_id, sizeof(element_id), "intersection-%d", id);
        
        UINode* intersection_node = ui_div(plateau_container->tree, element_id);
        if (!intersection_node) continue;
        
        // Calculer position et taille de l'élément
        int screen_x = data->offset_x + intersection->c * data->cell_width;
        int screen_y = data->offset_y + intersection->r * data->cell_height;
        int size = PIECE_RADIUS * 2 + 8; // Zone de détection un peu plus grande
        
        // Positionner l'élément sur l'intersection
        atomic_set_position(intersection_node->element, 
                          screen_x - size/2, screen_y - size/2);
        atomic_set_size(intersection_node->element, size, size);
        
        // Rendre l'élément invisible (transparent)
        atomic_set_background_color(intersection_node->element, 0, 0, 0, 0);
        atomic_set_z_index(intersection_node->element, 100); // Au-dessus du plateau
        
        // Créer les données d'intersection
        IntersectionElement* intersection_elem = (IntersectionElement*)malloc(sizeof(IntersectionElement));
        if (intersection_elem) {
            intersection_elem->ui_node = intersection_node;
            intersection_elem->intersection_id = id;
            intersection_elem->screen_x = screen_x;
            intersection_elem->screen_y = screen_y;
            intersection_elem->is_hovered = false;
            intersection_elem->is_selected = false;
            intersection_elem->plateau_data = data;
            
            // Attacher les données à l'élément
            atomic_set_custom_data(intersection_node->element, "intersection_data", intersection_elem);
            
            // Configurer les événements
            atomic_set_hover_handler(intersection_node->element, intersection_handle_hover);
            atomic_set_unhover_handler(intersection_node->element, intersection_handle_unhover);
            atomic_set_click_handler(intersection_node->element, intersection_handle_click);
            
            // Stocker l'élément
            data->intersection_elements[id] = intersection_elem;
            
            // Ajouter à la hiérarchie UI
            ui_append(plateau_container, intersection_node);
        }
    }
    
    // 🆕 AJOUT: Animation initiale en wave
    animate_initial_piece_wave(data);
    
    printf("✅ Created %d individual intersection elements with wave animation\n", NODES);
}

// 🔧 MODIFIER la fonction de rendu pour mettre à jour les positions
static void plateau_update_intersection_positions(PlateauRenderData* data) {
    if (!data) return;
    
    for (int id = 0; id < NODES; id++) {
        IntersectionElement* elem = data->intersection_elements[id];
        if (!elem) continue;
        
        Intersection* intersection = &data->board->nodes[id];
        
        // Recalculer la position écran
        int screen_x = data->offset_x + intersection->c * data->cell_width;
        int screen_y = data->offset_y + intersection->r * data->cell_height;
        int size = PIECE_RADIUS * 2 + 8;
        
        // Mettre à jour la position de l'élément UI
        atomic_set_position(elem->ui_node->element, 
                          screen_x - size/2, screen_y - size/2);
        
        elem->screen_x = screen_x;
        elem->screen_y = screen_y;
    }
}

// 🔧 MODIFIER plateau_custom_render pour inclure la mise à jour des positions
static void plateau_custom_render(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(element, "plateau_data");
    if (!data) return;
    
    // Charger les textures au premier rendu si pas encore fait
    if (!data->texture_black && !data->texture_brown) {
        plateau_load_piece_textures(data, renderer);
    }
    
    // Mettre à jour le renderer
    data->renderer = renderer;
    
    // Calculer les dimensions et offsets
    SDL_Rect rect = atomic_get_render_rect(element);
    data->offset_x = rect.x + PLATEAU_MARGIN;
    data->offset_y = rect.y + PLATEAU_MARGIN;
    data->cell_width = (rect.w - 2 * PLATEAU_MARGIN) / (COLS - 1);
    data->cell_height = (rect.h - 2 * PLATEAU_MARGIN) / (ROWS - 1);
    
    // 🆕 Mettre à jour les positions des éléments d'intersection
    plateau_update_intersection_positions(data);
    
    // Dessiner le fond du plateau (mat)
    SDL_SetRenderDrawColor(renderer, PLATEAU_BG_R, PLATEAU_BG_G, PLATEAU_BG_B, 255);
    SDL_RenderFillRect(renderer, &rect);
    
    // Dessiner la bordure
    SDL_SetRenderDrawColor(renderer, LINE_COLOR_R, LINE_COLOR_G, LINE_COLOR_B, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Dessiner tous les éléments du plateau
    plateau_draw_all_lines(data);
    plateau_draw_all_intersections(data);
    
    // Mettre à jour le feedback AVANT de dessiner les pions
    plateau_update_visual_feedback(data, 0.016f);
    
    // Dessiner les feedbacks SOUS les pions pour qu'ils soient visibles
    plateau_render_hover_feedback(data);
    plateau_render_selection_feedback(data);
    plateau_render_valid_destinations(data);
    
    // Dessiner les pions PAR-DESSUS les feedbacks
    plateau_draw_all_pieces(data);
    
    // Coordonnées et erreurs en dernier
    plateau_draw_coordinates(data);
    plateau_render_error_feedback(data);
}

// 🔧 MODIFIER ui_plateau_container_with_players pour créer les éléments individuels
UINode* ui_plateau_container_with_players(UITree* tree, const char* id, GamePlayer* player1, GamePlayer* player2) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    // Créer le container principal
    UINode* plateau_container = ui_div(tree, id);
    if (!plateau_container) {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create plateau container");
        return NULL;
    }
    
    // Taille par défaut du plateau
    SET_SIZE(plateau_container, PLATEAU_VISUAL_WIDTH, PLATEAU_VISUAL_HEIGHT);
    
    // Style : fond mat avec bordure
    atomic_set_background_color(plateau_container->element, PLATEAU_BG_R, PLATEAU_BG_G, PLATEAU_BG_B, 255);
    atomic_set_border(plateau_container->element, 3, LINE_COLOR_R, LINE_COLOR_G, LINE_COLOR_B, 255);
    atomic_set_border_radius(plateau_container->element, 8);
    
    // Initialiser le plateau logique
    Board* board = (Board*)malloc(sizeof(Board));
    if (board) {
        board_init(board);
        
        // Créer les données de rendu
        PlateauRenderData* render_data = (PlateauRenderData*)malloc(sizeof(PlateauRenderData));
        if (render_data) {
            // Initialiser toutes les données
            render_data->board = board;
            render_data->renderer = NULL;
            render_data->show_intersections = true;
            render_data->show_coordinates = false;
            render_data->player1 = player1;
            render_data->player2 = player2;
            render_data->texture_black = NULL;
            render_data->texture_brown = NULL;
            
            // Initialiser le tableau des éléments d'intersection
            for (int i = 0; i < NODES; i++) {
                render_data->intersection_elements[i] = NULL;
            }
            
            // Initialiser les joueurs de test si pas fournis
            if (!player1 || !player2) {
                plateau_init_test_players(render_data);
            } else {
                plateau_init_visual_feedback(render_data);
            }
            
            // Attacher les données au composant
            atomic_set_custom_data(plateau_container->element, "plateau_data", render_data);
            atomic_set_custom_data(plateau_container->element, "board", board);
            
            // Définir le rendu personnalisé
            atomic_set_custom_render(plateau_container->element, plateau_custom_render);
            
            // 🆕 Créer les éléments UI individuels pour chaque intersection MAINTENANT
            // Calculer les positions initiales
            render_data->offset_x = PLATEAU_MARGIN;
            render_data->offset_y = PLATEAU_MARGIN;
            render_data->cell_width = (PLATEAU_VISUAL_WIDTH - 2 * PLATEAU_MARGIN) / (COLS - 1);
            render_data->cell_height = (PLATEAU_VISUAL_HEIGHT - 2 * PLATEAU_MARGIN) / (ROWS - 1);
            
            // Créer les éléments d'intersection maintenant
            plateau_create_intersection_elements(render_data, plateau_container);
            
            ui_log_event("UIComponent", "Create", id, "Plateau container created with individual intersection elements");
            printf("✅ Plateau container '%s' créé avec éléments individuels:\n", id);
            printf("   🎯 %d éléments d'intersection avec événements propres\n", NODES);
            printf("   👥 Joueurs configurés avec textures\n");
            printf("   🎨 piece_black.png et piece_brun.png\n");
            printf("   ⚫ %d pions initialisés\n", board->piece_count);
        } else {
            free(board);
            ui_log_event("UIComponent", "CreateError", id, "Failed to allocate render data");
        }
    } else {
        ui_log_event("UIComponent", "CreateError", id, "Failed to allocate board");
    }
    
    return plateau_container;
}

// 🔧 MODIFIER ui_plateau_cleanup pour nettoyer les éléments individuels
void ui_plateau_cleanup(UINode* plateau) {
    if (!plateau) return;
    
    printf("🧹 [PLATEAU_CLEANUP] Début du nettoyage pour '%s'\n", plateau->id);
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    Board* board = (Board*)atomic_get_custom_data(plateau->element, "board");
    
    if (data) {
        // 🆕 Nettoyer les éléments d'intersection individuels
        for (int i = 0; i < NODES; i++) {
            IntersectionElement* elem = data->intersection_elements[i];
            if (elem) {
                printf("🗑️ [PLATEAU_CLEANUP] Nettoyage intersection element %d\n", i);
                free(elem);
                data->intersection_elements[i] = NULL;
            }
        }
        
        // Nettoyer les joueurs de test si créés localement
        if (data->player1) {
            printf("🗑️ [PLATEAU_CLEANUP] Nettoyage player1: '%s'\n", data->player1->name);
            player_destroy(data->player1);
            data->player1 = NULL;
        }
        if (data->player2) {
            printf("🗑️ [PLATEAU_CLEANUP] Nettoyage player2: '%s'\n", data->player2->name);
            player_destroy(data->player2);
            data->player2 = NULL;
        }
        
        // Les textures sont gérées par le système de référence
        data->texture_black = NULL;
        data->texture_brown = NULL;
        
        if (data->visual_state) {
            if (data->visual_state->valid_destinations) {
                free(data->visual_state->valid_destinations);
            }
            free(data->visual_state);
            data->visual_state = NULL;
        }
        
        printf("🗑️ [PLATEAU_CLEANUP] Libération PlateauRenderData\n");
        free(data);
    }
    
    if (board) {
        printf("🗑️ [PLATEAU_CLEANUP] Nettoyage du plateau logique\n");
        board_free(board);
        free(board);
    }
    
    // Nettoyer les custom_data pour éviter les pointeurs pendants
    atomic_set_custom_data(plateau->element, "plateau_data", NULL);
    atomic_set_custom_data(plateau->element, "board", NULL);
    
    ui_log_event("UIComponent", "PlateauCleanup", plateau->id, "Plateau and individual intersection elements cleaned up");
    printf("✅ [PLATEAU_CLEANUP] Nettoyage terminé pour '%s'\n", plateau->id);
}

// 🆕 NOUVELLE FONCTION: Destruction automatique du plateau
void ui_plateau_container_destroy(UINode* plateau_container) {
    if (!plateau_container) return;
    
    printf("🧹 [PLATEAU_DESTROY] Destruction du container de plateau '%s'\n", 
           plateau_container->id ? plateau_container->id : "NoID");
    
    // Nettoyer les données spécifiques au plateau
    ui_plateau_cleanup(plateau_container);
    
    // Le UINode lui-même sera détruit par le système UI
    printf("✅ [PLATEAU_DESTROY] Container de plateau détruit\n");
}