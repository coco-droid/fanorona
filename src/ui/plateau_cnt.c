#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"
#include "../utils/asset_manager.h"
#include "../plateau/plateau.h"
#include "../pions/pions.h"
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

typedef struct PlateauRenderData {
    Board* board;
    SDL_Renderer* renderer;
    int offset_x;
    int offset_y;
    int cell_width;
    int cell_height;
    bool show_intersections;
    bool show_coordinates;
} PlateauRenderData;

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

// Dessiner un pion
static void plateau_draw_piece(PlateauRenderData* data, int r, int c, Player owner) {
    int x, y;
    plateau_logical_to_screen(data, r, c, &x, &y);
    
    if (owner == WHITE) {
        // Pion blanc avec bordure noire
        plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS, 
                                 WHITE_PIECE_R, WHITE_PIECE_G, WHITE_PIECE_B, 255);
        plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS - 2, 
                                 BLACK_PIECE_R, BLACK_PIECE_G, BLACK_PIECE_B, 255);
        plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS - 3, 
                                 WHITE_PIECE_R, WHITE_PIECE_G, WHITE_PIECE_B, 255);
    } else if (owner == BLACK) {
        // Pion noir avec bordure blanche
        plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS, 
                                 WHITE_PIECE_R, WHITE_PIECE_G, WHITE_PIECE_B, 255);
        plateau_draw_filled_circle(data->renderer, x, y, PIECE_RADIUS - 2, 
                                 BLACK_PIECE_R, BLACK_PIECE_G, BLACK_PIECE_B, 255);
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

// Fonction de rendu personnalisée pour le plateau
static void plateau_custom_render(AtomicElement* element, SDL_Renderer* renderer) {
    if (!element || !renderer) return;
    
    // Récupérer les données du plateau
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(element, "plateau_data");
    if (!data) return;
    
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
    
    printf("🎨 Plateau rendered: %d intersections, %d pieces\n", NODES, data->board->piece_count);
}

// === FONCTIONS PUBLIQUES ===

UINode* ui_plateau_container(UITree* tree, const char* id) {
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
            
            // Attacher les données au composant
            atomic_set_custom_data(plateau_container->element, "plateau_data", render_data);
            atomic_set_custom_data(plateau_container->element, "board", board);
            
            // Définir le rendu personnalisé
            atomic_set_custom_render(plateau_container->element, plateau_custom_render);
            
            ui_log_event("UIComponent", "Create", id, "Plateau container created with board and custom renderer");
            printf("✅ Plateau container '%s' créé :\n", id);
            printf("   🎯 Damier %dx%d intersections\n", ROWS, COLS);
            printf("   🎨 Rendu personnalisé avec fond mat\n");
            printf("   ⚫ %d pions initialisés\n", board->piece_count);
            printf("   📏 Taille par défaut : %dx%d\n", PLATEAU_VISUAL_WIDTH, PLATEAU_VISUAL_HEIGHT);
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

void ui_plateau_cleanup(UINode* plateau) {
    if (!plateau) return;
    
    PlateauRenderData* data = (PlateauRenderData*)atomic_get_custom_data(plateau->element, "plateau_data");
    Board* board = (Board*)atomic_get_custom_data(plateau->element, "board");
    
    if (data) {
        free(data);
    }
    
    if (board) {
        board_free(board);
        free(board);
    }
    
    ui_log_event("UIComponent", "PlateauCleanup", plateau->id, "Plateau resources cleaned up");
}
