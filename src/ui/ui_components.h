#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "ui_tree.h"
#include "animation.h"  // 🆕 AJOUT: Support pour les animations
#include "../config.h"  // 🔧 FIX: Include config.h for AvatarID type
#include "../sound/sound.h"  // 🆕 AJOUT: Support pour les sons
#include <SDL2/SDL_ttf.h>

// Forward declarations
typedef struct GamePlayer GamePlayer;
typedef struct SceneManager SceneManager; // 🆕 AJOUT

// === FONCTIONS DE CRÉATION DE COMPOSANTS ===

// Syntaxe simplifiée pour créer des composants
UINode* ui_div(UITree* tree, const char* id);
UINode* ui_text(UITree* tree, const char* id, const char* content);
UINode* ui_image(UITree* tree, const char* id, SDL_Texture* texture);

// === FONCTIONS FLUIDES (CHAÎNABLES) ===

// Positionnement et taille
UINode* ui_set_position(UINode* node, int x, int y);
UINode* ui_set_size(UINode* node, int width, int height);
UINode* ui_set_z_index(UINode* node, int z_index);

// Style fluide
UINode* ui_set_background(UINode* node, const char* color);
UINode* ui_set_background_image(UINode* node, const char* image_path);
UINode* ui_set_background_size(UINode* node, const char* size);
UINode* ui_set_background_repeat(UINode* node, const char* repeat);
UINode* ui_set_color(UINode* node, const char* color);
UINode* ui_add_class(UINode* node, const char* class_name);

// Alignement et positionnement
UINode* ui_set_align(UINode* node, const char* horizontal, const char* vertical);
UINode* ui_center(UINode* node); // Centre automatiquement
UINode* ui_center_x(UINode* node); // Centre horizontalement
UINode* ui_center_y(UINode* node); // Centre verticalement

// 🆕 NOUVELLES FONCTIONS pour la gestion de l'overflow
UINode* ui_set_overflow(UINode* node, const char* overflow);
UINode* ui_set_overflow_visible(UINode* node);   // Les enfants peuvent déborder
UINode* ui_set_overflow_hidden(UINode* node);    // Les enfants sont contraints
UINode* ui_set_overflow_scroll(UINode* node);    // Avec scroll (futur)
UINode* ui_set_overflow_auto(UINode* node);      // Automatique

// Fonctions utilitaires pour l'overflow
bool ui_is_child_overflowing(UINode* parent, UINode* child);
void ui_constrain_all_children(UINode* parent);

// 🆕 NOUVELLES FONCTIONS pour align-self
UINode* ui_set_align_self(UINode* node, const char* align_self);
UINode* ui_set_align_self_center_x(UINode* node);
UINode* ui_set_align_self_center_y(UINode* node);
UINode* ui_set_align_self_center_both(UINode* node);

// Flexbox
UINode* ui_set_flex_direction(UINode* node, const char* direction);
UINode* ui_set_justify_content(UINode* node, const char* justify);
UINode* ui_set_align_items(UINode* node, const char* align);
UINode* ui_set_flex_gap(UINode* node, int gap);
UINode* ui_set_display_flex(UINode* node);

// Texte et polices
UINode* ui_set_font(UINode* node, const char* font_path, int size);
UINode* ui_set_text_color(UINode* node, const char* color);
UINode* ui_set_text_align(UINode* node, const char* align);
UINode* ui_set_text_style(UINode* node, bool bold, bool italic);
UINode* ui_set_font_size(UINode* node, int size);  // 🔧 FIX: UINode* au lieu de void
void ui_set_text_size(UINode* node, int size);     // 🔧 AJOUTÉ: Déclaration de la fonction void

// Événements fluides
UINode* ui_on_click(UINode* node, void (*callback)(UINode*, void*));
UINode* ui_on_hover(UINode* node, void (*callback)(UINode*, void*));

// Hiérarchie fluide
UINode* ui_append_to(UINode* child, UINode* parent);
UINode* ui_append(UINode* parent, UINode* child);

// Button component function
UINode* ui_button(UITree* tree, const char* id, const char* text, void (*onClick)(UINode* node, void* user_data), void* user_data);

// 🆕 NOUVELLES FONCTIONS: Intégration des sons dans les composants UI
void ui_button_enable_sound_feedback(UINode* button);  // Active les sons click/hover automatiques
void ui_button_set_click_sound(UINode* button, SoundEffect sound);  // Son personnalisé au clic
void ui_button_set_hover_sound(UINode* button, SoundEffect sound);  // Son personnalisé au survol

// === CONTAINER COMPONENT ===

// Créer un container avec style modal (fond noir transparent, bordure orange)
// + Logo et sous-titre "Stratégie et Tradition" par défaut
UINode* ui_container(UITree* tree, const char* id);

// 🆕 Version étendue permettant de masquer la barre inférieure (cog/pause)
UINode* ui_container_extended(UITree* tree, const char* id, bool show_bottom_bar);

// Container avec taille spécifiée
UINode* ui_container_with_size(UITree* tree, const char* id, int width, int height);

// Container centré automatiquement
UINode* ui_container_centered(UITree* tree, const char* id, int width, int height);

// 🆕 NOUVELLES FONCTIONS pour le contenu par défaut
void ui_container_add_default_logo(UINode* container);
void ui_container_add_default_subtitle(UINode* container);

// 🆕 Définir le SceneManager global pour les composants UI internes
void ui_set_global_scene_manager(SceneManager* manager);

// Ajouter du contenu au container (positionnement automatique sous le sous-titre)
void ui_container_add_content(UINode* container, UINode* content);

// Ajouter un en-tête orange au container
void ui_container_add_header(UINode* container, const char* header_text);

// Basculer entre style modal et normal
void ui_container_set_modal_style(UINode* container, bool is_modal);

// === NEON BUTTON COMPONENT ===

// 🔧 NOTE: Les neon buttons sont implémentés dans neon_btn.c (pas dans ui_components.c)
// Créer un bouton avec effet neon hover automatique
UINode* ui_neon_button(UITree* tree, const char* id, const char* text, 
                       void (*onClick)(UINode* node, void* user_data), void* user_data);

// Personnaliser la couleur de lueur
void ui_neon_button_set_glow_color(UINode* neon_btn, int r, int g, int b);

// Contrôler la vitesse d'animation
void ui_neon_button_set_animation_speed(UINode* neon_btn, float speed);

// Mettre à jour toutes les animations neon (appelé par le système)
void ui_neon_button_update_all(UITree* tree, float delta_time);

// Button styling functions
void ui_button_set_style(UINode* button, const char* bg_color, const char* text_color, const char* border_color);
void ui_button_set_hover_style(UINode* button, const char* hover_bg_color, const char* hover_text_color);
void ui_button_set_pressed_style(UINode* button, const char* pressed_bg_color, const char* pressed_text_color);

// 🆕 NOUVELLES FONCTIONS pour l'enregistrement des événements
void ui_button_register_events(UINode* button, UITree* tree);
void ui_tree_register_all_events(UITree* tree);

// === NOUVELLES FONCTIONS DE DEBUGGING ===

// Activer/désactiver les logs d'événements
void ui_set_event_logging(bool enabled);
bool ui_is_event_logging_enabled(void);

// 🆕 HITBOX VISUALIZATION SYSTEM
// Contrôler l'affichage des hitboxes (rectangles rouges transparents avec bordure bleue)
void ui_set_hitbox_visualization(bool enabled);
bool ui_is_hitbox_visualization_enabled(void);

// Logs spécifiques pour le debugging
void ui_log_event(const char* source, const char* event_type, const char* element_id, const char* message);

// === GESTION Z-INDEX IMPLICITE ===

// Calculer et assigner les z-index automatiquement
void ui_calculate_implicit_z_index(UITree* tree);
void ui_node_set_implicit_z_index(UINode* node, int base_z_index);

// Obtenir le z-index effectif (explicite ou implicite)
int ui_node_get_effective_z_index(UINode* node);

// === CORRECTIONS BOUTONS ===

// Définir l'image de fond spécifiquement pour les boutons
void ui_button_set_background_image(UINode* button, const char* image_path);

// Corriger l'affichage du texte sur les boutons
void ui_button_fix_text_rendering(UINode* button);

// Calculer la position optimale pour le texte du bouton
void ui_button_calculate_text_position(UINode* button);

// Police par défaut
TTF_Font* ui_get_default_font(void);
void ui_cleanup_fonts(void);

// Fonctions de texte avec TTF
UINode* ui_text_with_font(UITree* tree, const char* id, const char* content, TTF_Font* font);
void ui_set_text_font(UINode* node, TTF_Font* font);
void ui_set_text_size(UINode* node, int size);

// === HELPERS POUR STYLE CSS-LIKE ===

typedef struct {
    int x, y, width, height, z_index;
    char* background_color;
    char* background_image;
    char* color;
    char* display;
    char* align_horizontal;
    char* align_vertical;
    char* flex_direction;
    char* justify_content;
    char* align_items;
    int flex_gap;
    char* font_path;
    int font_size;
    char* text_align;
    bool text_bold;
    bool text_italic;
} UIStyle;

void ui_apply_style(UINode* node, const UIStyle* style);

// === DEBUGGING DU TEXTE ===

// Déboguer le rendu du texte
void ui_debug_text_rendering(UINode* node, const char* context);

// === NOUVELLES FONCTIONS POUR FEEDBACK VISUEL ===

// Gestion des états visuels des boutons
void ui_button_set_pressed_state(UINode* button, bool pressed);
void ui_button_set_hover_state(UINode* button, bool hovered);
void ui_button_reset_visual_state(UINode* button);

// Animation et transitions
void ui_button_animate_click(UINode* button, int duration_ms);
void ui_button_animate_hover(UINode* button, bool entering);

// Couleurs d'état prédéfinies
void ui_button_apply_success_style(UINode* button);   // Vert pour actions positives
void ui_button_apply_danger_style(UINode* button);    // Rouge pour actions dangereuses
void ui_button_apply_info_style(UINode* button);      // Bleu pour informations
void ui_button_apply_warning_style(UINode* button);   // Orange pour avertissements

// Effets visuels avancés
void ui_button_add_glow_effect(UINode* button, const char* color);
void ui_button_add_shadow_effect(UINode* button, int offset_x, int offset_y);
void ui_button_remove_all_effects(UINode* button);

// 🆕 Mouse event handlers for plateau
void ui_plateau_set_mouse_handlers(UINode* plateau);
void ui_plateau_update_visual_feedback(UINode* plateau, float delta_time);
void ui_plateau_register_events(UINode* plateau, EventManager* event_manager);

// 🆕 DEBUG FUNCTIONS for plateau events
void ui_plateau_debug_intersections(UINode* plateau);
void ui_plateau_debug_current_selection(UINode* plateau);
void ui_plateau_debug_visual_state(UINode* plateau);

// 🆕 Game state integration
void ui_plateau_set_game_logic(UINode* plateau, void* game_logic);
void* ui_plateau_get_game_logic(UINode* plateau);

// === NOUVELLES FONCTIONS POUR FEEDBACK VISUEL AVEC SCALE ===

// Gestion des états visuels des boutons avec effet de scale
void ui_button_set_pressed_state(UINode* button, bool pressed);
void ui_button_set_hover_state(UINode* button, bool hovered);
void ui_button_reset_visual_state(UINode* button);

// 🆕 NOUVELLES FONCTIONS pour l'effet de scale
void ui_button_set_scale(UINode* button, float scale_factor);
void ui_button_animate_scale_to(UINode* button, float target_scale, float duration);
float ui_button_get_current_scale(UINode* button);

// Effet de scale prédéfini pour les différents états
void ui_button_scale_hover(UINode* button);     // Scale 105% (agrandissement hover)
void ui_button_scale_pressed(UINode* button);   // Scale 97% (effet enfoncé)
void ui_button_scale_normal(UINode* button);    // Scale 100% (taille normale)

// === MACROS POUR FEEDBACK VISUEL ===

#define BUTTON_PRESSED(btn) ui_button_set_pressed_state(btn, true)
#define BUTTON_RELEASED(btn) ui_button_set_pressed_state(btn, false)
#define BUTTON_HOVER_ON(btn) ui_button_set_hover_state(btn, true)
#define BUTTON_HOVER_OFF(btn) ui_button_set_hover_state(btn, false)
#define BUTTON_RESET(btn) ui_button_reset_visual_state(btn)

// 🆕 NOUVELLES MACROS pour l'effet de scale
#define BUTTON_SCALE_HOVER(btn) ui_button_scale_hover(btn)
#define BUTTON_SCALE_PRESSED(btn) ui_button_scale_pressed(btn)
#define BUTTON_SCALE_NORMAL(btn) ui_button_scale_normal(btn)

// Styles prédéfinis
#define BUTTON_SUCCESS(btn) ui_button_apply_success_style(btn)
#define BUTTON_DANGER(btn) ui_button_apply_danger_style(btn)
#define BUTTON_INFO(btn) ui_button_apply_info_style(btn)
#define BUTTON_WARNING(btn) ui_button_apply_warning_style(btn)

// === MACROS POUR SYNTAXE SIMPLIFIÉE ===

#define UI_BUTTON(tree, id, label) ui_button(tree, id, label)
#define UI_DIV(tree, id) ui_div(tree, id)
#define UI_TEXT(tree, id, content) ui_text(tree, id, content)
#define UI_IMAGE(tree, id, texture) ui_image(tree, id, texture)
#define UI_CONTAINER(tree, id) ui_container(tree, id)
#define UI_CONTAINER_CENTERED(tree, id, w, h) ui_container_centered(tree, id, w, h)

// 🔧 FIX: Ajouter les macros manquantes
#define FLEX_ROW(node) ui_set_flex_direction(node, "row")
#define FLEX_COLUMN(node) ui_set_flex_direction(node, "column")
#define APPEND(parent, child) ui_append(parent, child)

// 🆕 NOUVELLES MACROS pour align-self
#define ALIGN_SELF_X(node) ui_set_align_self_center_x(node)
#define ALIGN_SELF_Y(node) ui_set_align_self_center_y(node)
#define ALIGN_SELF_BOTH(node) ui_set_align_self_center_both(node)

// 🆕 NOUVELLES MACROS pour overflow
#define SET_OVERFLOW(node, type) ui_set_overflow(node, type)
#define OVERFLOW_HIDDEN(node) ui_set_overflow_hidden(node)
#define OVERFLOW_VISIBLE(node) ui_set_overflow_visible(node)
#define OVERFLOW_SCROLL(node) ui_set_overflow_scroll(node)
#define OVERFLOW_AUTO(node) ui_set_overflow_auto(node)

// Macro de debugging
#define DEBUG_TEXT(node) ui_debug_text_rendering(node, #node)

// === SIDEBAR COMPONENT ===

// Créer une sidebar complète avec titre, joueurs et boutons
UINode* ui_sidebar(UITree* tree, const char* id);

// Fonctions helper pour les sections de la sidebar
void ui_sidebar_add_player_containers(UINode* sidebar, GamePlayer* player1, GamePlayer* player2);
UINode* ui_sidebar_create_player_info(UITree* tree, const char* id, GamePlayer* player);
void ui_sidebar_add_control_buttons(UINode* sidebar);
UINode* ui_sidebar_create_control_button(UITree* tree, const char* id, const char* icon, const char* text, bool is_prominent);

// 🆕 Mise à jour de l'indicateur de tour
void ui_sidebar_update_current_turn(UINode* sidebar, GamePlayer* current_player);

// 🆕 NOUVELLE FONCTION: Mettre à jour le timer d'un joueur
void ui_sidebar_update_player_timer(UINode* sidebar, GamePlayer* player);

// 🔧 FIX: Add capture update function
void ui_sidebar_update_player_captures(UINode* sidebar, GamePlayer* player);

// === CNT_PLAYABLE COMPONENT ===

// Créer une zone de jeu (2/3 de l'écran)
UINode* ui_cnt_playable(UITree* tree, const char* id);

// Ajouter la zone de jeu centrale
void ui_cnt_playable_add_game_area(UINode* playable_container);

// Container de jeu avec taille spécifiée
UINode* ui_cnt_playable_with_size(UITree* tree, const char* id, int width, int height);

// === PLATEAU COMPONENT ===

// Forward declaration pour Board
typedef struct Board Board;

// Créer un container de plateau avec damier Fanorona et joueurs
UINode* ui_plateau_container(UITree* tree, const char* id);
UINode* ui_plateau_container_with_players(UITree* tree, const char* id, GamePlayer* player1, GamePlayer* player2);

// Plateau avec taille personnalisée
UINode* ui_plateau_container_with_size(UITree* tree, const char* id, int width, int height);

// Configuration d'affichage
void ui_plateau_set_show_intersections(UINode* plateau, bool show);
void ui_plateau_set_show_coordinates(UINode* plateau, bool show);

// Accès aux données du plateau
Board* ui_plateau_get_board(UINode* plateau);
void ui_plateau_update_from_board(UINode* plateau, Board* new_board);

// 🆕 NOUVELLES FONCTIONS pour les joueurs
void ui_plateau_set_players(UINode* plateau, GamePlayer* player1, GamePlayer* player2);
GamePlayer* ui_plateau_get_player1(UINode* plateau);
GamePlayer* ui_plateau_get_player2(UINode* plateau);

// 🆕 NOUVELLE FONCTION: Utiliser un plateau partagé (ex: depuis GameLogic)
void ui_plateau_set_shared_board(UINode* plateau, Board* board);

// 🆕 Mouse event handlers for plateau
void ui_plateau_set_mouse_handlers(UINode* plateau);
void ui_plateau_update_visual_feedback(UINode* plateau, float delta_time);
void ui_plateau_register_events(UINode* plateau, EventManager* event_manager);

// 🆕 DEBUG FUNCTIONS for plateau events
void ui_plateau_debug_intersections(UINode* plateau);
void ui_plateau_debug_current_selection(UINode* plateau);
void ui_plateau_debug_visual_state(UINode* plateau);

// 🆕 Game state integration
void ui_plateau_set_game_logic(UINode* plateau, void* game_logic);
void* ui_plateau_get_game_logic(UINode* plateau);

// Nettoyage des ressources
void ui_plateau_cleanup(UINode* plateau);

// 🆕 NOUVELLE FONCTION: Destruction complète du container
void ui_plateau_container_destroy(UINode* plateau_container);

// === TEXT INPUT COMPONENT ===

// Créer un champ de texte avec placeholder et curseur clignotant
UINode* ui_text_input(UITree* tree, const char* id, const char* placeholder);

// Configurer la longueur maximale
void ui_text_input_set_max_length(UINode* input, int max_length);

// Obtenir/définir le texte
const char* ui_text_input_get_text(UINode* input);
void ui_text_input_set_text(UINode* input, const char* text);

// 🆕 NOUVELLE FONCTION: Définir le placeholder dynamiquement
void ui_text_input_set_placeholder(UINode* input, const char* placeholder);

// 🆕 NOUVELLE FONCTION: Lier un text input à un ID de scène
void ui_text_input_set_scene_id(UINode* input, const char* scene_input_id);

// 🆕 NOUVELLE FONCTION: Récupérer le texte par ID de scène (depuis le registre global)
const char* ui_text_input_get_text_by_id(const char* scene_input_id);

// 🆕 NOUVELLE FONCTION: Nettoyer le registre global des text inputs
void ui_text_input_cleanup_registry(void);

// === MACROS POUR LES NOUVEAUX COMPOSANTS ===

#define UI_SIDEBAR(tree, id) ui_sidebar(tree, id)
#define UI_CNT_PLAYABLE(tree, id) ui_cnt_playable(tree, id)
#define UI_PLATEAU(tree, id) ui_plateau_container(tree, id)
#define UI_PLATEAU_SIZED(tree, id, w, h) ui_plateau_container_with_size(tree, id, w, h)

// === MACROS MANQUANTES ===

#define SET_POS(node, x, y) ui_set_position(node, x, y)
#define SET_SIZE(node, w, h) ui_set_size(node, w, h)
#define SET_BG(node, color) ui_set_background(node, color)
#define SET_BG_IMG(node, path) ui_set_background_image(node, path)
#define SET_BG_SIZE(node, size) ui_set_background_size(node, size)
#define SET_BG_REPEAT(node, repeat) ui_set_background_repeat(node, repeat)
#define CENTER(node) ui_center(node)
#define CENTER_X(node) ui_center_x(node)
#define CENTER_Y(node) ui_center_y(node)

// === NOUVELLES FONCTIONS D'ANIMATION UI ===
UINode* ui_animate_fade_in(UINode* node, float duration);
UINode* ui_animate_fade_out(UINode* node, float duration);
UINode* ui_animate_slide_in_left(UINode* node, float duration, float distance);
UINode* ui_animate_slide_in_right(UINode* node, float duration, float distance);
UINode* ui_animate_slide_out_left(UINode* node, float duration, float distance);
UINode* ui_animate_shake_x(UINode* node, float duration, float intensity);
UINode* ui_animate_pulse(UINode* node, float duration);
UINode* ui_stop_animations(UINode* node);

// === NOUVELLES FONCTIONS D'ANIMATION DE PIECES ===

// Animation de déplacement de pièce
void ui_plateau_animate_piece_move(UINode* plateau, int from_id, int to_id);

// Animation de capture (disparition)
void ui_plateau_animate_piece_capture(UINode* plateau, int piece_id);

// Animation de placement (apparition)
void ui_plateau_animate_piece_placement(UINode* plateau, int intersection_id);

// Animation de sélection (pulse)
void ui_plateau_animate_piece_selection(UINode* plateau, int piece_id);

// Animations de fin de jeu
void ui_plateau_animate_victory_dance(UINode* plateau, int winning_player);
void ui_plateau_animate_defeat_fade(UINode* plateau, int losing_player);

// Animation d'apparition initiale
void ui_plateau_animate_initial_wave(UINode* plateau);

// === MACROS POUR ANIMATIONS DE PIECES ===

#define ANIMATE_PIECE_MOVE(plateau, from, to) ui_plateau_animate_piece_move(plateau, from, to)
#define ANIMATE_PIECE_CAPTURE(plateau, id) ui_plateau_animate_piece_capture(plateau, id)
#define ANIMATE_PIECE_PLACE(plateau, id) ui_plateau_animate_piece_placement(plateau, id)
#define ANIMATE_PIECE_SELECT(plateau, id) ui_plateau_animate_piece_selection(plateau, id)
#define ANIMATE_VICTORY(plateau, player) ui_plateau_animate_victory_dance(plateau, player)
#define ANIMATE_DEFEAT(plateau, player) ui_plateau_animate_defeat_fade(plateau, player)
#define ANIMATE_INITIAL_WAVE(plateau) ui_plateau_animate_initial_wave(plateau)

// === AVATAR SELECTOR COMPONENT ===

// Créer un sélecteur d'avatar avec avatar principal et 6 mini-avatars cliquables
UINode* ui_avatar_selector(UITree* tree, const char* id);

// Définir un callback appelé lors du changement d'avatar
void ui_avatar_selector_set_callback(UINode* selector, 
                                     void (*callback)(AvatarID avatar, void* user_data),
                                     void* user_data);

// Obtenir l'avatar actuellement sélectionné
AvatarID ui_avatar_selector_get_selected(UINode* selector);

// Définir l'avatar sélectionné programmatiquement
void ui_avatar_selector_set_selected(UINode* selector, AvatarID avatar);

// Remettre la sélection à AVATAR_WARRIOR
void ui_avatar_selector_reset(UINode* selector);

// 🆕 NOUVEAU HELPER: Réinitialiser aux paramètres par défaut
void ui_avatar_selector_reset_to_defaults(UINode* selector);

// Enregistrer tous les événements des mini-avatars dans l'EventManager
void ui_avatar_selector_register_events(UINode* selector, EventManager* event_manager);

// Mettre à jour les animations du composant
void ui_avatar_selector_update(UINode* selector, float delta_time);

// 🆕 AI INTEGRATION FUNCTIONS
void ui_plateau_set_ai_mode(UINode* plateau, bool enable_ai);
bool ui_plateau_is_ai_mode(UINode* plateau);
void ui_plateau_trigger_ai_turn(UINode* plateau);
bool ui_plateau_is_ai_thinking(UINode* plateau);

// === MACROS POUR AVATAR SELECTOR ===

#define UI_AVATAR_SELECTOR(tree, id) ui_avatar_selector(tree, id)
#define AVATAR_RESET_DEFAULTS(selector) ui_avatar_selector_reset_to_defaults(selector)

// === NOUVELLES FONCTIONS D'ANIMATION DE PIECES AMÉLIORÉES ===

// Configurer la vitesse des animations de pièces
void ui_plateau_set_animation_speed(UINode* plateau, float speed_multiplier);

// Vérifier si des animations sont en cours
bool ui_plateau_has_active_animations(UINode* plateau);

// Arrêter toutes les animations de pièces
void ui_plateau_clear_animations(UINode* plateau);

// === MACROS POUR ANIMATIONS DE PIECES AMÉLIORÉES ===

#define PLATEAU_ANIMATION_SLOW(plateau) ui_plateau_set_animation_speed(plateau, 0.5f)
#define PLATEAU_ANIMATION_NORMAL(plateau) ui_plateau_set_animation_speed(plateau, 1.0f)
#define PLATEAU_ANIMATION_FAST(plateau) ui_plateau_set_animation_speed(plateau, 2.0f)
#define PLATEAU_CLEAR_ANIMATIONS(plateau) ui_plateau_clear_animations(plateau)
#define PLATEAU_HAS_ANIMATIONS(plateau) ui_plateau_has_active_animations(plateau)

// === CONFIRM MODAL COMPONENT ===

// Créer une modale de confirmation (Overlay + Boîte + Boutons Oui/Non)
// Masquée par défaut (display: none). Utiliser atomic_set_style_display(node->element, "flex") pour afficher.
UINode* ui_confirm_modal(UITree* tree, const char* id, const char* title, const char* message, 
                         void (*on_yes)(void*, SDL_Event*), void (*on_no)(void*, SDL_Event*));

#endif // UI_COMPONENTS_H