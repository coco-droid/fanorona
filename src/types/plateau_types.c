#include "plateau_types.h"
#include "../logic/logic.h"
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // 🆕 For AI animation

// Import PlateauRenderData definition
#include "../ui/ui_components.h"
#include "../ui/ui_tree.h"
#include "../ui/native/atomic.h"
#include "../window/window.h"
#include "../pions/pions.h"
#include "../ai/ai.h" // 🆕 For AI functions
#include "../utils/asset_manager.h" // 🆕 For config access

// Full definition needed for logic functions
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
    void* intersection_elements[NODES];
} PlateauRenderData;

// === AI ANIMATION STATE ===
typedef struct AIAnimationState {
    bool ai_is_moving;
    float ai_move_delay;
    Move pending_ai_move;
    bool showing_capture_preview;
    float capture_preview_timer;
    int captured_pieces[MAX_CAPTURE_LIST];
    int capture_count;
    int consecutive_ai_moves;
} AIAnimationState;

static AIAnimationState g_ai_animation = {false, 0.0f, {0}, false, 0.0f, {0}, 0, 0};


// Forward declaration for animation stub
static void animate_piece_capture(PlateauRenderData* data, int piece_id) {
    (void)data;
    (void)piece_id;
}

void log_board_state(Board* board, const char* context) {
    if (!board) return;
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ ETAT DU PLATEAU - %s\n", context);
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
    printf("║ Pieces: Blanc=%d  Noir=%d\n", white_count, black_count);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
}

bool has_additional_captures(PlateauRenderData* data, int from_id) {
    if (!data || !data->board) return false;
    Intersection* from = &data->board->nodes[from_id];
    if (!from->piece) return false;
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

void apply_capture(PlateauRenderData* data, Move* move) {
    if (!data || !move || !move->is_capture) return;
    printf("\n┌────────────────────────────────────────────────────────┐\n");
    printf("│ CAPTURE DETECTEE\n");
    printf("├────────────────────────────────────────────────────────┤\n");
    printf("│ Mouvement: intersection %d → %d\n", move->from_id, move->to_id);
    printf("│ Nombre de captures: %d\n", move->capture_count);
    printf("│ Pieces capturees: ");
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
            printf("   Suppression piece %d a l'intersection %d\n",
                captured->id, cap_id);
            captured->alive = 0;
            data->board->nodes[cap_id].piece = NULL;
            animate_piece_capture(data, cap_id);
        }
    }
}

void apply_move_to_board(PlateauRenderData* data, int from_id, int to_id) {
    if (!data || !data->board) return;
    Intersection* from = &data->board->nodes[from_id];
    Intersection* to = &data->board->nodes[to_id];
    if (!from->piece || !from->piece->alive) {
        printf("[APPLY_MOVE] Pas de piece a deplacer sur %d\n", from_id);
        return;
    }
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ MOUVEMENT APPLIQUE\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ Piece %d: intersection %d → %d\n", from->piece->id, from_id, to_id);
    printf("║ Proprietaire: %s\n", from->piece->owner == WHITE ? "Blanc" : "Noir");
    Move move;
    int capture_type = detect_capture(data->board, from_id, to_id, &move);
    bool is_capture = move.is_capture && move.capture_count > 0;
    if (is_capture) {
        printf("║ TYPE: CAPTURE (%s)\n",
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
        printf("║ TYPE: PAIKA (deplacement simple)\n");
    }
    to->piece = from->piece;
    from->piece = NULL;
    to->piece->r = to->r;
    to->piece->c = to->c;
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    log_board_state(data->board, "APRES MOUVEMENT");
    if (is_capture) {
        bool more_captures = has_additional_captures(data, to_id);
        if (more_captures) {
            printf("\n┌────────────────────────────────────────────────────────┐\n");
            printf("│ CAPTURES SUPPLEMENTAIRES DISPONIBLES\n");
            printf("│ Chaine de captures activee\n");
            printf("│ Le tour continue pour le meme joueur\n");
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
            printf("│ Aucune capture supplementaire possible\n");
            printf("│ Fin de la chaine de captures\n");
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
        printf("│ CHANGEMENT DE TOUR\n");
        printf("│ Avant: %s\n", before->name);
        printf("│ Apres: %s\n", after->name);
        printf("└────────────────────────────────────────────────────────┘\n\n");
    }
}

void calculate_valid_destinations(PlateauRenderData* data, int piece_id) {
    if (!data || !data->visual_state || !data->board) return;
    if (data->visual_state->valid_destinations) {
        free(data->visual_state->valid_destinations);
        data->visual_state->valid_destinations = NULL;
    }
    data->visual_state->valid_count = 0;
    Intersection* intersection = &data->board->nodes[piece_id];
    if (!intersection->piece || !intersection->piece->alive) return;
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ CALCUL DES DESTINATIONS VALIDES\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ Piece a l'intersection: %d\n", piece_id);
    printf("║ Proprietaire: %s\n",
        intersection->piece->owner == WHITE ? "Blanc" : "Noir");
    printf("║ En chaine de captures: %s\n",
        data->visual_state->in_capture_chain ? "OUI" : "NON");
    Move possible_moves[MAX_MOVES];
    int move_count = generate_moves(data->board, intersection->piece->owner, possible_moves, MAX_MOVES);
    printf("║ Coups generes au total: %d\n", move_count);
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
                    printf("║   Coup %d→%d rejete (meme direction)\n",
                        piece_id, possible_moves[i].to_id);
                    continue;
                }
                if (is_position_visited(possible_moves[i].to_id,
                    data->visual_state->visited_positions,
                    data->visual_state->visited_count)) {
                    printf("║   Coup %d→%d rejete (position deja visitee)\n",
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
                printf("║   Destination valide: %d %s\n",
                    possible_moves[i].to_id,
                    possible_moves[i].is_capture ? "(CAPTURE)" : "(PAIKA)");
            }
        }
    }
    data->visual_state->valid_destinations = destinations;
    data->visual_state->valid_count = dest_count;
    printf("║ Total destinations valides: %d\n", dest_count);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
}

bool is_valid_destination(PlateauRenderData* data, int from_id, int to_id) {
    if (!data || !data->board || !data->visual_state) return false;
    
    GameLogic* logic = (GameLogic*)data->game_logic;
    if (!logic) return false;
    
    // 🔧 SIMPLIFIED: Utiliser la couleur logique du joueur actuel
    GamePlayer* current_player = game_logic_get_current_player_info(logic);
    if (!current_player) return false;
    
    Player current_logical_color = current_player->logical_color;
    
    // Delegate to rules.c for authoritative validation
    return is_move_valide(
        data->board, from_id, to_id, current_logical_color,
        data->visual_state->in_capture_chain ? &data->visual_state->last_capture_direction : NULL,
        data->visual_state->visited_positions,
        data->visual_state->visited_count,
        data->visual_state->in_capture_chain ? 1 : 0
    );
}

// 🆕 Check game over and update GameLogic state + visual feedback
bool check_and_handle_game_over(PlateauRenderData* data) {
    if (!data || !data->board || !data->game_logic) return false;
    
    GameLogic* logic = (GameLogic*)data->game_logic;
    Player winner = check_game_over(data->board);
    
    if (winner != NOBODY) {
        logic->game_finished = true;
        logic->winner = winner;
        logic->state = GAME_STATE_GAME_OVER;
        
        GamePlayer* winner_player = (winner == logic->player1->logical_color) ? 
                                     logic->player1 : logic->player2;
        
        printf("\n╔═══════════════════════════════════════════════════════════╗\n");
        printf("║ PARTIE TERMINEE!\n");
        printf("║ Vainqueur: %s\n", winner_player->name);
        printf("║ Pieces restantes - Blanc: %d, Noir: %d\n", 
               count_alive_pieces(data->board, WHITE),
               count_alive_pieces(data->board, BLACK));
        printf("╚═══════════════════════════════════════════════════════════╝\n");
        
        // Clear selection visual state
        if (data->visual_state) {
            data->visual_state->selected_intersection = -1;
            data->visual_state->hovered_intersection = -1;
            if (data->visual_state->valid_destinations) {
                free(data->visual_state->valid_destinations);
                data->visual_state->valid_destinations = NULL;
            }
            data->visual_state->valid_count = 0;
        }
        
        return true;
    }
    
    return false;
}

// 🆕 Moved from plateau_cnt.c
static AIEngine* get_or_create_ai_engine(PlateauRenderData* data) {
    if (!data || !data->game_logic) return NULL;

    GameLogic* logic = (GameLogic*)data->game_logic;
    if (logic->mode != GAME_MODE_VS_AI) return NULL;

    // Use static to prevent multiple creations
    static AIEngine* cached_ai = NULL;
    if (cached_ai) return cached_ai;

    AIDifficulty difficulty = config_get_ai_difficulty();
    bool ai_is_white = config_is_ai_white();
    Player ai_player = ai_is_white ? WHITE : BLACK;

    cached_ai = ai_create(AI_TYPE_MINIMAX, difficulty, ai_player);
    if (cached_ai) {
        printf("🤖 AI Engine créé: difficulté %d, joue %s\n",
               difficulty, ai_is_white ? "Blanc" : "Noir");
    }

    return cached_ai;
}

// === PIECE ANIMATION SYSTEM ===
PieceAnimationManager g_piece_manager = {0};  // Remove 'static' to match extern declaration
static bool g_manager_initialized = false;

static void plateau_logical_to_screen_coords(PlateauRenderData* data, int r, int c, float* x, float* y) {
    *x = (float)(data->offset_x + c * data->cell_width);
    *y = (float)(data->offset_y + r * data->cell_height);
}

static float ease_out_cubic(float t) {
    return 1.0f - powf(1.0f - t, 3.0f);
}

static float ease_in_out_cubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

void piece_animation_manager_init(PieceAnimationManager* manager) {
    if (!manager) return;
    
    manager->active_count = 0;
    manager->animation_in_progress = false;
    manager->global_speed_multiplier = 1.0f;
    
    for (int i = 0; i < 10; i++) {
        manager->animations[i].is_active = false;
        manager->animations[i].on_complete = NULL;
    }
    
    printf("✨ Piece Animation Manager initialisé\n");
}

static void on_piece_animation_complete(PieceAnimation* anim) {
    if (!anim) return;
    
    printf("Animation de piece terminee: %d → %d (duree: %.2fs)\n", 
           anim->from_intersection, anim->to_intersection, anim->elapsed_time);
    
    anim->is_active = false;
}

bool piece_animation_start(PlateauRenderData* data, int from_id, int to_id, bool is_capture,
                          const int* captured_pieces, int capture_count) {
    if (!data || !data->visual_state) return false;
    
    // Initialize manager once
    if (!g_manager_initialized) {
        piece_animation_manager_init(&g_piece_manager);
        g_manager_initialized = true;
    }
    
    PieceAnimationManager* anim_manager = &g_piece_manager;
    
    // Trouver un slot libre
    int slot = -1;
    for (int i = 0; i < 10; i++) {
        if (!anim_manager->animations[i].is_active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf("Pas de slot d'animation disponible\n");
        return false;
    }
    
    PieceAnimation* anim = &anim_manager->animations[slot];
    
    // Configuration de l'animation
    anim->piece_id = data->board->nodes[from_id].piece ? data->board->nodes[from_id].piece->id : -1;
    anim->from_intersection = from_id;
    anim->to_intersection = to_id;
    anim->is_capture_move = is_capture;
    anim->capture_count = capture_count;
    
    // Copier les pièces capturées
    for (int i = 0; i < capture_count && i < MAX_CAPTURE_LIST; i++) {
        anim->captured_pieces[i] = captured_pieces[i];
    }
    
    // Calculer positions start/end
    int from_r, from_c, to_r, to_c;
    rc_from_id(from_id, &from_r, &from_c);
    rc_from_id(to_id, &to_r, &to_c);
    
    plateau_logical_to_screen_coords(data, from_r, from_c, &anim->start_x, &anim->start_y);
    plateau_logical_to_screen_coords(data, to_r, to_c, &anim->end_x, &anim->end_y);
    
    anim->current_x = anim->start_x;
    anim->current_y = anim->start_y;
    anim->progress = 0.0f;
    anim->elapsed_time = 0.0f;
    
    // 🎨 Durée variable selon le type de mouvement et la distance - ACCÉLÉRÉE
    float distance = sqrtf(powf(anim->end_x - anim->start_x, 2) + powf(anim->end_y - anim->start_y, 2));
    // 🔧 FIX: Vitesse d'animation augmentée (durée réduite)
    float base_duration = is_capture ? 0.6f : 0.4f; // Captures un peu plus lentes que paika
    anim->duration = base_duration + (distance / 500.0f) * 0.2f; // Facteur distance réduit
    
    // Appliquer le multiplicateur de vitesse
    anim->duration /= anim_manager->global_speed_multiplier;
    
    anim->is_active = true;
    anim->on_complete = on_piece_animation_complete;
    
    anim_manager->active_count++;
    anim_manager->animation_in_progress = true;
    
    printf("Animation de piece demarree:\n");
    printf("   %d → %d (%.1f,%.1f) → (%.1f,%.1f)\n", 
           from_id, to_id, anim->start_x, anim->start_y, anim->end_x, anim->end_y);
    printf("   Duree: %.2fs %s\n", anim->duration, is_capture ? "(CAPTURE)" : "(PAIKA)");
    printf("   Distance: %.1f pixels\n", distance);
    
    return true;
}

void piece_animation_update(PlateauRenderData* data, float delta_time) {
    if (!data || !data->visual_state) return;
    
    PieceAnimationManager* manager = &g_piece_manager;
    if (!manager->animation_in_progress) return;
    
    int completed_count = 0;
    bool any_active = false;
    
    for (int i = 0; i < 10; i++) {
        PieceAnimation* anim = &manager->animations[i];
        if (!anim->is_active) continue;
        
        any_active = true;
        anim->elapsed_time += delta_time;
        
        // Calculer le progrès avec easing
        float raw_progress = anim->elapsed_time / anim->duration;
        if (raw_progress >= 1.0f) {
            raw_progress = 1.0f;
            completed_count++;
        }
        
        // Appliquer l'easing (plus fluide)
        anim->progress = anim->is_capture_move ? 
            ease_in_out_cubic(raw_progress) :  // Captures plus dramatiques
            ease_out_cubic(raw_progress);      // Mouvements normaux plus fluides
        
        // Interpoler la position
        anim->current_x = anim->start_x + (anim->end_x - anim->start_x) * anim->progress;
        anim->current_y = anim->start_y + (anim->end_y - anim->start_y) * anim->progress;
        
        // Animation terminée
        if (raw_progress >= 1.0f) {
            if (anim->on_complete) {
                anim->on_complete(anim);
            }
            manager->active_count--;
        }
    }
    
    // Mettre à jour l'état global
    manager->animation_in_progress = any_active;
    
    // Log occasionnel du progres
    static float debug_timer = 0.0f;
    debug_timer += delta_time;
    if (debug_timer > 2.0f && any_active) {
        printf("Animations en cours: %d (completees cette frame: %d)\n", 
               manager->active_count, completed_count);
        debug_timer = 0.0f;
    }
}

bool piece_animation_is_active(PlateauRenderData* data) {
    if (!data || !data->visual_state) return false;
    return g_piece_manager.animation_in_progress;
}

void piece_animation_set_speed(PlateauRenderData* data, float speed_multiplier) {
    if (!data || !data->visual_state) return;
    
    g_piece_manager.global_speed_multiplier = speed_multiplier;
    printf("Vitesse d'animation définie à %.1fx\n", speed_multiplier);
}

void piece_animation_clear_all(PlateauRenderData* data) {
    if (!data || !data->visual_state) return;
    
    for (int i = 0; i < 10; i++) {
        g_piece_manager.animations[i].is_active = false;
    }
    g_piece_manager.active_count = 0;
    g_piece_manager.animation_in_progress = false;
    printf("🧹 Toutes les animations de pièces effacées\n");
}

// 🔧 MODIFICATION: execute_animated_move maintenant utilise le nouveau système
void execute_animated_move(PlateauRenderData* data, int from_id, int to_id) {
    if (!data || !data->board) return;
    
    printf("🎬 [ANIMATE_MOVE] Démarrage animation améliorée: %d → %d\n", from_id, to_id);
    
    // Detect capture BEFORE movement
    Move move;
    detect_capture(data->board, from_id, to_id, &move);
    bool is_capture = move.is_capture && move.capture_count > 0;
    
    // Start animation BEFORE applying movement
    bool animation_started = piece_animation_start(data, from_id, to_id, is_capture, 
                                                  move.captured_ids, move.capture_count);
    
    if (!animation_started) {
        printf("❌ Échec du démarrage de l'animation, application directe\n");
    }
    
    apply_move_to_board(data, from_id, to_id);
    
    // 🆕 Check game over after move
    if (check_and_handle_game_over(data)) {
        piece_animation_clear_all(data);
        g_ai_animation.ai_is_moving = false;
        return;
    }
    
    printf("✅ [ANIMATE_MOVE] Mouvement appliqué avec animation fluide\n");

    // 🔧 FIX: Only trigger AI after a small delay to let animations complete
    if (is_ai_turn(data) && !g_ai_animation.ai_is_moving) {
        printf("🤖 [ANIMATE_MOVE] Tour de l'IA détecté - démarrage avec délai\n");
        // Small delay to let human move animation finish
        g_ai_animation.ai_move_delay = 0.3f; // 🔧 FIX: Délai réduit (0.8 -> 0.3) car on attend l'animation
        g_ai_animation.ai_is_moving = true;

        // Calculate AI move immediately but delay execution
        AIEngine* ai = get_or_create_ai_engine(data);
        if (ai) {
            Move ai_move = ai_find_best_move(ai, data->board);
            g_ai_animation.pending_ai_move = ai_move;

            // Setup capture preview if needed
            if (ai_move.is_capture && ai_move.capture_count > 0) {
                g_ai_animation.showing_capture_preview = true;
                g_ai_animation.capture_preview_timer = 1.5f;
                g_ai_animation.capture_count = ai_move.capture_count;
                for (int i = 0; i < ai_move.capture_count; i++) {
                    g_ai_animation.captured_pieces[i] = ai_move.captured_ids[i];
                }
            }
        }
    }
}

// 🆕 Moved from plateau_cnt.c
void update_ai_animation(PlateauRenderData* data, float delta_time) {
    if (!g_ai_animation.ai_is_moving) return;

    // 🔧 FIX: Synchronisation stricte - Attendre la fin des animations visuelles avant de continuer
    if (piece_animation_is_active(data)) return;

    // Update capture preview timer
    if (g_ai_animation.showing_capture_preview) {
        g_ai_animation.capture_preview_timer -= delta_time;
        if (g_ai_animation.capture_preview_timer <= 0.0f) {
            g_ai_animation.showing_capture_preview = false;
        }
    }

    // Update move delay
    g_ai_animation.ai_move_delay -= delta_time;

    if (g_ai_animation.ai_move_delay <= 0.0f) {
        Move ai_move = g_ai_animation.pending_ai_move;

        g_ai_animation.consecutive_ai_moves++;

        printf("\n┌────────────────────────────────────────────────────────┐\n");
        printf("│ IA JOUE SON COUP #%d\n", g_ai_animation.consecutive_ai_moves);
        printf("│ Mouvement: %d → %d\n", ai_move.from_id, ai_move.to_id);
        printf("│ Capture: %s (%d piece(s))\n",
               ai_move.is_capture ? "OUI" : "NON", ai_move.capture_count);
        printf("└────────────────────────────────────────────────────────┘\n");

        execute_animated_move(data, ai_move.from_id, ai_move.to_id);

        if (ai_move.is_capture && ai_move.capture_count > 0) {
            printf("Verification des captures supplementaires depuis %d...\n", ai_move.to_id);

            bool more_captures = has_additional_captures(data, ai_move.to_id);

            if (more_captures) {
                printf("\n┌────────────────────────────────────────────────────────┐\n");
                printf("│ CHAINE DE CAPTURES DETECTEE!\n");
                printf("│ L'IA DOIT continuer a capturer\n");
                printf("│ L'IA va jouer un autre coup immediatement\n");
                printf("└────────────────────────────────────────────────────────┘\n\n");

                // L'IA DOIT rejouer immédiatement
                AIEngine* ai = get_or_create_ai_engine(data);
                if (ai) {
                    Move next_ai_move = ai_find_best_move(ai, data->board);

                    if (next_ai_move.from_id != -1 && next_ai_move.to_id != -1) {
                        // Programmer le prochain coup de l'IA avec un délai court
                        g_ai_animation.pending_ai_move = next_ai_move;
                        g_ai_animation.ai_move_delay = 0.4f; // 🔧 FIX: Délai réduit (1.0 -> 0.4) entre coups en chaîne
                        g_ai_animation.ai_is_moving = true;

                        // Setup capture preview si nécessaire
                        if (next_ai_move.is_capture && next_ai_move.capture_count > 0) {
                            g_ai_animation.showing_capture_preview = true;
                            g_ai_animation.capture_preview_timer = 1.5f;
                            g_ai_animation.capture_count = next_ai_move.capture_count;
                            for (int i = 0; i < next_ai_move.capture_count; i++) {
                                g_ai_animation.captured_pieces[i] = next_ai_move.captured_ids[i];
                            }
                        }

                        printf("✅ Prochain coup IA programmé: %d → %d\n",
                               next_ai_move.from_id, next_ai_move.to_id);
                        return; // Ne pas réinitialiser l'état, continuer la chaîne
                    }
                }
            } else {
                printf("\n┌────────────────────────────────────────────────────────┐\n");
                printf("│ AUCUNE CAPTURE SUPPLEMENTAIRE\n");
                printf("│ Fin de la séquence de l'IA\n");
                printf("│ Total coups IA consécutifs: %d\n", g_ai_animation.consecutive_ai_moves);
                printf("│ Maintenant c'est au tour du joueur\n");
                printf("└────────────────────────────────────────────────────────┘\n\n");
            }
        }

        // Réinitialiser l'état d'animation IA (fin de séquence)
        g_ai_animation.ai_is_moving = false;
        g_ai_animation.showing_capture_preview = false;
        g_ai_animation.capture_count = 0;
        g_ai_animation.consecutive_ai_moves = 0; // Reset du compteur

        // 🔧 FIX: Reset visual selection après séquence IA complète
        if (data->visual_state) {
            data->visual_state->selected_intersection = -1;
            if (data->visual_state->valid_destinations) {
                free(data->visual_state->valid_destinations);
                data->visual_state->valid_destinations = NULL;
            }
            data->visual_state->valid_count = 0;
        }
    }
}

// 🆕 Moved from plateau_cnt.c
bool is_ai_turn(PlateauRenderData* data) {
    if (!data || !data->game_logic) return false;

    GameLogic* logic = (GameLogic*)data->game_logic;
    if (logic->mode != GAME_MODE_VS_AI) return false;
    if (logic->game_finished) return false; // 🆕 No AI turn if game is over

    GamePlayer* current = game_logic_get_current_player_info(logic);
    return current && current->type == PLAYER_TYPE_AI;
}

// 🆕 Moved from plateau_cnt.c
void execute_ai_move(PlateauRenderData* data) {
    if (!data || !data->board || !data->game_logic) return;

    // Don't start new AI move if one is already in progress
    if (g_ai_animation.ai_is_moving) return;

    AIEngine* ai = get_or_create_ai_engine(data);
    if (!ai) {
        printf("❌ Impossible de récupérer l'AI Engine\n");
        return;
    }

    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ DEBUT DE SEQUENCE IA\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║ L'IA commence a calculer son premier coup...\n");

     // L'IA calcule le meilleur coup
     Move best_move = ai_find_best_move(ai, data->board);

     if (best_move.from_id == -1 || best_move.to_id == -1) {
        printf("║ ❌ Aucun coup valide trouvé par l'IA\n");
        printf("╚═══════════════════════════════════════════════════════════╝\n");
        return;
     }

    printf("║ Premier coup choisi: %d → %d\n", best_move.from_id, best_move.to_id);
    printf("╚═══════════════════════════════════════════════════════════╝\n");

     // Store move for delayed execution
     g_ai_animation.pending_ai_move = best_move;
     g_ai_animation.ai_move_delay = 0.5f; // 🔧 FIX: Délai réduit (1.5 -> 0.5) au début du tour
     g_ai_animation.ai_is_moving = true;
     g_ai_animation.consecutive_ai_moves = 0; // 🆕 Reset du compteur au début de séquence

     // Show capture preview if it's a capture move
     if (best_move.is_capture && best_move.capture_count > 0) {
         g_ai_animation.showing_capture_preview = true;
         g_ai_animation.capture_preview_timer = 2.0f; // 2 seconds preview
         g_ai_animation.capture_count = best_move.capture_count;

         for (int i = 0; i < best_move.capture_count; i++) {
             g_ai_animation.captured_pieces[i] = best_move.captured_ids[i];
         }

        printf("Previsualisation des captures: %d piece(s)\n", best_move.capture_count);
     }

    printf("Coup programme avec delai d'animation de %.1fs\n\n", g_ai_animation.ai_move_delay);
}

// 🆕 Getter for AI animation state, needed by plateau_cnt.c
bool get_ai_capture_preview_state(int* count, const int** pieces, float* timer) {
    if (g_ai_animation.showing_capture_preview) {
        *count = g_ai_animation.capture_count;
        *pieces = g_ai_animation.captured_pieces;
        *timer = g_ai_animation.capture_preview_timer;
        return true;
    }
    return false;
}
