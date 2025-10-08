#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"
#include "../utils/asset_manager.h"
#include "../plateau/plateau.h"
#include "../pions/pions.h"
#include "../logic/rules.h"  // 🔧 FIX: Add missing include
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
    int hovered_intersection;           // -1 if none
    int selected_intersection;          // -1 if none
    int* valid_destinations;            // Array of valid destinations (0/1)
    int valid_count;
    float animation_timer;              // 0.0 - 1.0
    char error_message[128];
    float error_display_timer;          // Seconds remaining
    int error_type;                     // 0=capture required, 1=direction forbidden, 2=visited
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
    GamePlayer* player1;  // 🆕 Joueur 1
    GamePlayer* player2;  // 🆕 Joueur 2
    SDL_Texture* texture_black;  // 🆕 Texture pions noirs
    SDL_Texture* texture_brown;  // 🆕 Texture pions bruns
    VisualFeedbackState* visual_state;  // 🆕 Visual feedback
    void* game_logic;                   // 🆕 Reference to GameLogic
} PlateauRenderData;

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

static int plateau_find_intersection_at_mouse(PlateauRenderData* data, int mouse_x, int mouse_y) {
    for (int id = 0; id < NODES; id++) {
        Intersection* intersection = &data->board->nodes[id];
        int screen_x, screen_y;
        plateau_logical_to_screen(data, intersection->r, intersection->c, &screen_x, &screen_y);
        
        int dx = mouse_x - screen_x;
        int dy = mouse_y - screen_y;
        int distance_squared = dx*dx + dy*dy;
        int detection_radius = PIECE_RADIUS + 8; // Player-friendly detection
        
        if (distance_squared <= detection_radius * detection_radius) {
            return id;
        }
    }
    return -1;
}

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
    
    // 🆕 DEBUG
    static int hover_debug_counter = 0;
    if (hover_debug_counter++ % 60 == 0) { // Log toutes les 60 frames
        printf("🌟 Rendering hover feedback for intersection %d\n", vf->hovered_intersection);
    }
    
    Intersection* intersection = &data->board->nodes[vf->hovered_intersection];
    int x, y;
    plateau_logical_to_screen(data, intersection->r, intersection->c, &x, &y);
    
    // Animated halo
    float pulse = (sin(vf->animation_timer * 8.0f) + 1.0f) * 0.5f; // 0-1
    int halo_radius = PIECE_RADIUS + 5 + (int)(pulse * 3);
    int alpha = 80 + (int)(pulse * 80); // 80-160
    
    // Golden hover halo
    plateau_draw_filled_circle(data->renderer, x, y, halo_radius, 255, 255, 100, alpha);
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
    
    vf->animation_timer += delta_time * 2.0f; // Animation speed
    if (vf->animation_timer > 6.28f) vf->animation_timer = 0.0f; // Reset at 2π
    
    if (vf->error_display_timer > 0.0f) {
        vf->error_display_timer -= delta_time;
    }
}

static void plateau_handle_mouse_move(void* element, SDL_Event* event) {
    (void)event; // Éviter le warning unused parameter
    
    AtomicElement* atomic = (AtomicElement*)element;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(atomic, "plateau_data");
    if (!data) return;
    
    VisualFeedbackState* vf = data->visual_state;
    
    // 🔧 FIX: Obtenir les coordonnées de souris absolues
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    // 🔧 FIX: Convertir en coordonnées relatives au plateau
    SDL_Rect rect = atomic_get_render_rect(atomic);
    int relative_mouse_x = mouse_x - rect.x;
    int relative_mouse_y = mouse_y - rect.y;
    
    // 🆕 DEBUG: Afficher les coordonnées pour debug (réduit pour éviter le spam)
    static int debug_counter = 0;
    if (debug_counter++ % 60 == 0) { // Log toutes les 60 frames
        printf("🐭 Plateau hover: abs(%d,%d) -> rel(%d,%d)\n", 
               mouse_x, mouse_y, relative_mouse_x, relative_mouse_y);
    }
    
    int new_hovered = plateau_find_intersection_at_mouse(data, relative_mouse_x, relative_mouse_y);
    
    if (new_hovered != vf->hovered_intersection) {
        vf->hovered_intersection = new_hovered;
        vf->animation_timer = 0.0f; // Reset animation
        
        // 🆕 DEBUG: Confirmer la détection avec effet de scale
        if (new_hovered >= 0) {
            printf("🎯 Intersection %d hovered with scale effect\n", new_hovered);
        }
    }
}

static void plateau_handle_mouse_click(void* element, SDL_Event* event) {
    (void)event; // Éviter le warning unused parameter
    
    AtomicElement* atomic = (AtomicElement*)element;
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(atomic, "plateau_data");
    if (!data) return;
    
    VisualFeedbackState* vf = data->visual_state;
    
    // 🔧 FIX: Même correction pour les clics
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    SDL_Rect rect = atomic_get_render_rect(atomic);
    int relative_mouse_x = mouse_x - rect.x;
    int relative_mouse_y = mouse_y - rect.y;
    
    // 🆕 DEBUG: Confirmer les coordonnées de clic
    printf("🖱️ Click: abs(%d,%d) -> rel(%d,%d)\n", 
           mouse_x, mouse_y, relative_mouse_x, relative_mouse_y);
    
    int clicked_id = plateau_find_intersection_at_mouse(data, relative_mouse_x, relative_mouse_y);
    
    if (clicked_id >= 0) {
        printf("🎯 Intersection %d clicked\n", clicked_id);
        
        Intersection* clicked = &data->board->nodes[clicked_id];
        
        if (vf->selected_intersection < 0) {
            // No piece selected - try to select this piece
            if (clicked->piece && clicked->piece->owner == data->player1->logical_color) {
                vf->selected_intersection = clicked_id;
                plateau_calculate_valid_moves(data, clicked_id);
                printf("🎯 Piece selected at intersection %d\n", clicked_id);
            } else {
                plateau_show_error_feedback(data, 4); // Wrong piece
                printf("❌ Cannot select intersection %d (no piece or wrong owner)\n", clicked_id);
            }
        } else {
            // Piece already selected - try to move
            if (clicked_id == vf->selected_intersection) {
                // Deselect
                vf->selected_intersection = -1;
                vf->valid_count = 0;
                printf("🔄 Piece deselected\n");
            } else if (vf->valid_destinations[clicked_id]) {
                // Valid move - execute it
                printf("✅ Valid move from %d to %d\n", vf->selected_intersection, clicked_id);
                // TODO: Execute move with rules.c
                vf->selected_intersection = -1;
                vf->valid_count = 0;
            } else {
                // Invalid move - show error
                plateau_show_error_feedback(data, 3); // Not adjacent (simplified)
                printf("❌ Invalid move to intersection %d\n", clicked_id);
            }
        }
    } else {
        printf("❌ No intersection found at click position\n");
    }
}

// 🔧 Remove duplicate plateau_init_visual_feedback function at line 290

// 🔧 Remove duplicate plateau_init_test_players function (lines 517-535)

// 🔧 Modifier les fonctions existantes

// 🔧 Modifier plateau_custom_render pour inclure le retour visuel
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
    
    // Dessiner le fond du plateau (mat)
    SDL_SetRenderDrawColor(renderer, PLATEAU_BG_R, PLATEAU_BG_G, PLATEAU_BG_B, 255);
    SDL_RenderFillRect(renderer, &rect);
    
    // Dessiner la bordure
    SDL_SetRenderDrawColor(renderer, LINE_COLOR_R, LINE_COLOR_G, LINE_COLOR_B, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Dessiner tous les éléments du plateau
    plateau_draw_all_lines(data);
    plateau_draw_all_intersections(data);
    plateau_draw_all_pieces(data);
    plateau_draw_coordinates(data);
    
    // Update visual feedback
    plateau_update_visual_feedback(data, 0.016f); // ~60 FPS
    
    // Render visual feedback layers
    plateau_render_hover_feedback(data);
    plateau_render_selection_feedback(data);
    plateau_render_valid_destinations(data);
    plateau_render_error_feedback(data);
    
    //printf("🎨 Plateau rendered: %d intersections, %d pieces\n", NODES, data->board->piece_count);
}

// === FONCTIONS PUBLIQUES ===

UINode* ui_plateau_container(UITree* tree, const char* id) {
    return ui_plateau_container_with_players(tree, id, NULL, NULL);
}

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
            render_data->board = board;
            render_data->renderer = NULL;
            render_data->show_intersections = true;
            render_data->show_coordinates = false;
            render_data->player1 = player1;
            render_data->player2 = player2;
            render_data->texture_black = NULL;
            render_data->texture_brown = NULL;
            
            // Initialiser les joueurs de test si pas fournis
            if (!player1 || !player2) {
                plateau_init_test_players(render_data);
            } else {
                // 🔧 FIX: Initialiser le visual feedback même avec des joueurs fournis
                plateau_init_visual_feedback(render_data);
            }
            
            // Attacher les données au composant
            atomic_set_custom_data(plateau_container->element, "plateau_data", render_data);
            atomic_set_custom_data(plateau_container->element, "board", board);
            
            // Définir le rendu personnalisé
            atomic_set_custom_render(plateau_container->element, plateau_custom_render);
            
            // 🆕 Set up mouse event handlers
            atomic_set_click_handler(plateau_container->element, plateau_handle_mouse_click);
            // 🔧 FIX: Use proper handler name from atomic.h
            atomic_set_hover_handler(plateau_container->element, plateau_handle_mouse_move);
            
            ui_log_event("UIComponent", "Create", id, "Plateau container created with players and textures");
            printf("✅ Plateau container '%s' créé avec joueurs :\n", id);
            printf("   🎯 Damier %dx%d intersections\n", ROWS, COLS);
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

UINode* ui_plateau_container_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* plateau = ui_plateau_container(tree, id);
    if (plateau) {
        SET_SIZE(plateau, width, height);
        ui_log_event("UIComponent", "Style", id, "Plateau container size customized");
    }
    return plateau;
}

void ui_plateau_set_show_intersections(UINode* plateau, bool show) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->show_intersections = show;
        ui_log_event("UIComponent", "PlateauConfig", plateau->id, 
                    show ? "Intersection display enabled" : "Intersection display disabled");
    }
}

void ui_plateau_set_show_coordinates(UINode* plateau, bool show) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->show_coordinates = show;
        ui_log_event("UIComponent", "PlateauConfig", plateau->id, 
                    show ? "Coordinate display enabled" : "Coordinate display disabled");
    }
}

Board* ui_plateau_get_board(UINode* plateau) {
    if (!plateau) return NULL;
    
    return (Board*)atomic_get_custom_data(plateau->element, "board");
}

void ui_plateau_update_from_board(UINode* plateau, Board* new_board) {
    if (!plateau || !new_board) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->board = new_board;
        atomic_set_custom_data(plateau->element, "board", new_board);
        ui_log_event("UIComponent", "PlateauUpdate", plateau->id, "Board state updated");
    }
}

// 🆕 NOUVELLES FONCTIONS pour les joueurs
void ui_plateau_set_players(UINode* plateau, GamePlayer* player1, GamePlayer* player2) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->player1 = player1;
        data->player2 = player2;
        ui_log_event("UIComponent", "PlateauConfig", plateau->id, "Players updated");
        printf("👥 Joueurs mis à jour pour le plateau '%s'\n", plateau->id);
    }
}

// 🆕 NEW: Set up mouse handlers for the plateau
void ui_plateau_set_mouse_handlers(UINode* plateau) {
    if (!plateau || !plateau->element) {
        printf("❌ Invalid plateau for mouse handlers setup\n");
        return;
    }
    
    // Handlers are already set up in ui_plateau_container_with_players
    // This function is mainly for external setup if needed
    printf("🖱️ Mouse handlers already configured for plateau '%s'\n", 
           plateau->id ? plateau->id : "NoID");
}

// 🆕 NEW: Update visual feedback (called from external systems)
void ui_plateau_update_visual_feedback(UINode* plateau, float delta_time) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data && data->visual_state) {
        plateau_update_visual_feedback(data, delta_time);
    }
}

// 🆕 NEW: Set game logic reference
void ui_plateau_set_game_logic(UINode* plateau, void* game_logic) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    if (data) {
        data->game_logic = game_logic;
        printf("🧠 Game logic connected to plateau '%s'\n", plateau->id ? plateau->id : "NoID");
    }
}

// 🆕 NEW: Get game logic reference
void* ui_plateau_get_game_logic(UINode* plateau) {
    if (!plateau) return NULL;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    return data ? data->game_logic : NULL;
}

void ui_plateau_cleanup(UINode* plateau) {
    if (!plateau) return;
    
    printf("🧹 [PLATEAU_CLEANUP] Début du nettoyage pour '%s'\n", plateau->id);
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    Board* board = (Board*)atomic_get_custom_data(plateau->element, "board");
    
    if (data) {
        // 🔧 FIX: Nettoyer les joueurs de test si créés localement
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
        
        // 🔧 FIX: Les textures sont gérées par le système de référence, pas besoin de les détruire ici
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
    
    // 🔧 FIX: Nettoyer les custom_data pour éviter les pointeurs pendants
    atomic_set_custom_data(plateau->element, "plateau_data", NULL);
    atomic_set_custom_data(plateau->element, "board", NULL);
    
    ui_log_event("UIComponent", "PlateauCleanup", plateau->id, "Plateau and players cleaned up completely");
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
