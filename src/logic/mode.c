#include "logic.h"
#include "../config.h"
#include "../utils/log_console.h"
#include "../logic/rules.h"
#include "../stats/game_stats.h"  // 🆕 AJOUT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 🆕 CRÉATION ET DESTRUCTION DE LA LOGIQUE DE JEU
GameLogic* game_logic_create(void) {
    GameLogic* logic = (GameLogic*)calloc(1, sizeof(GameLogic));
    if (!logic) {
        printf("❌ Impossible d'allouer la mémoire pour GameLogic\n");
        return NULL;
    }
    
    // Initialisation par défaut
    logic->state = GAME_STATE_MENU;
    logic->mode = config_get_mode();
    logic->current_player = WHITE; // Les blancs commencent toujours
    logic->turn_number = 1;
    logic->game_finished = false;
    logic->winner = NOBODY;
    
    // 🆕 Créer le plateau de jeu (propriété de la logique)
    logic->board = (Board*)malloc(sizeof(Board));
    if (logic->board) {
        board_init(logic->board);
    }

    // 🆕 Créer le gestionnaire de statistiques
    logic->stats_manager = game_stats_create();
    if (!logic->stats_manager) {
        printf("❌ Impossible de créer le gestionnaire de statistiques\n");
        free(logic);
        return NULL;
    }
    
    printf("🧠 GameLogic créée avec système de stats\n");
    return logic;
}

void game_logic_destroy(GameLogic* logic) {
    if (logic) {
        printf("🧹 Destruction de GameLogic\n");
        
        // 🆕 Détruire le plateau
        if (logic->board) {
            board_free(logic->board);
            free(logic->board);
        }

        // 🆕 Détruire le gestionnaire de stats
        if (logic->stats_manager) {
            game_stats_destroy(logic->stats_manager);
        }
        
        free(logic);
    }
}

// 🆕 DÉMARRER UNE NOUVELLE PARTIE
void game_logic_start_new_game(GameLogic* logic) {
    if (!logic) return;
    
    printf("🎬 Démarrage d'une nouvelle partie\n");
    
    // 🆕 Créer les joueurs selon la configuration
    GameMode mode = config_get_mode();
    logic->mode = mode;
    
    // 🔧 SIMPLIFIED: Joueur 1 = WHITE logique (toujours premier), Joueur 2 = BLACK logique
    // Les couleurs visuelles sont indépendantes de la logique de jeu
    if (!logic->player1) {
        logic->player1 = player_create(
            config_get_player1_name(), 
            WHITE,  // Toujours WHITE logique (premier joueur)
            config_get_player1_piece_color(),  // Couleur visuelle choisie
            PLAYER_TYPE_HUMAN, 
            1
        );
    }
    if (!logic->player2) {
        logic->player2 = player_create(
            config_get_player2_name(), 
            BLACK,  // Toujours BLACK logique (second joueur)
            config_get_player2_piece_color(),  // Couleur visuelle choisie
            PLAYER_TYPE_HUMAN, 
            2
        );
    }
    
    // Configuration selon le mode
    switch (mode) {
        case GAME_MODE_LOCAL_MULTIPLAYER:
            printf("🎮 Mode MULTIJOUEUR LOCAL\n");
            logic->player1->type = PLAYER_TYPE_HUMAN;
            logic->player2->type = PLAYER_TYPE_HUMAN;
            printf("   👤 Joueur 1 (premier): %s (pièces %s)\n", 
                   logic->player1->name, piece_color_to_string(logic->player1->piece_color));
            printf("   👤 Joueur 2 (second): %s (pièces %s)\n", 
                   logic->player2->name, piece_color_to_string(logic->player2->piece_color));
            break;
            
        case GAME_MODE_ONLINE_MULTIPLAYER:
            printf("🌐 Mode MULTIJOUEUR EN LIGNE\n");
            logic->player1->type = PLAYER_TYPE_HUMAN;
            logic->player2->type = PLAYER_TYPE_ONLINE;
            break;
            
        case GAME_MODE_VS_AI:
            printf("🤖 Mode CONTRE IA\n");
            bool ai_is_white = config_is_ai_white();
            if (ai_is_white) {
                logic->player1->type = PLAYER_TYPE_AI;
                logic->player2->type = PLAYER_TYPE_HUMAN;
            } else {
                logic->player1->type = PLAYER_TYPE_HUMAN;
                logic->player2->type = PLAYER_TYPE_AI;
            }
            break;
            
        default:
            printf("❌ Mode non reconnu\n");
            break;
    }
    
    // 🔧 SIMPLIFIED: Joueur 1 commence toujours (WHITE logique = premier)
    logic->current_player = PLAYER_1;
    logic->player1->is_current_turn = true;
    logic->player2->is_current_turn = false;
    printf("✅ Joueur 1 (%s) commence car il a WHITE logique\n", logic->player1->name);
    
    logic->turn_number = 1;
    logic->game_finished = false;
    logic->winner = NOBODY;
    logic->total_game_time = 0.0f;
    
    // Réinitialiser les statistiques des joueurs
    logic->player1->captures_made = 0;
    logic->player1->thinking_time = 0.0f;
    logic->player1->is_current_turn = true;
    
    logic->player2->captures_made = 0;
    logic->player2->thinking_time = 0.0f;
    logic->player2->is_current_turn = false;
    
    // 🆕 Initialiser les statistiques des joueurs
    if (logic->stats_manager && logic->player1 && logic->player2) { // 🔧 FIX: Check players exist
        game_stats_init_player(logic->stats_manager, 1, logic->player1->name);
        game_stats_init_player(logic->stats_manager, 2, logic->player2->name);
        
        // Lier les stats aux joueurs
        logic->player1->stats = game_stats_get_player(logic->stats_manager, 1);
        logic->player2->stats = game_stats_get_player(logic->stats_manager, 2);
        
        // Démarrer le timer du premier joueur
        game_stats_start_turn_timer(logic->stats_manager, 1);
        
        printf("✅ Système de statistiques initialisé pour les deux joueurs\n");
    } else {
        printf("⚠️ Impossible d'initialiser les statistiques (stats_manager ou joueurs NULL)\n");
    }
    
    // Déterminer l'état initial selon le mode
    if (logic->player1->type == PLAYER_TYPE_AI) {
        logic->state = GAME_STATE_AI_THINKING;
        printf("🤖 L'IA (Joueur 1) commence à réfléchir...\n");
    } else if (logic->player1->type == PLAYER_TYPE_ONLINE) {
        logic->state = GAME_STATE_ONLINE_WAITING;
        printf("📡 En attente du joueur distant (Joueur 1)...\n");
    } else {
        logic->state = GAME_STATE_WAITING_INPUT;
        printf("⌨️ En attente du joueur humain (Joueur 1): %s\n", logic->player1->name);
    }
    
    printf("✅ Nouvelle partie démarrée en mode: %s\n", config_mode_to_string(logic->mode));
}

// 🆕 FONCTIONS DE VÉRIFICATION DU TYPE DE TOUR
bool game_logic_is_human_turn(GameLogic* logic) {
    if (!logic || logic->game_finished) return false;
    
    GamePlayer* current = game_logic_get_current_player_info(logic);
    return current && current->type == PLAYER_TYPE_HUMAN;
}

bool game_logic_is_ai_turn(GameLogic* logic) {
    if (!logic || logic->game_finished) return false;
    
    GamePlayer* current = game_logic_get_current_player_info(logic);
    return current && current->type == PLAYER_TYPE_AI;
}

bool game_logic_is_remote_turn(GameLogic* logic) {
    if (!logic || logic->game_finished) return false;
    
    GamePlayer* current = game_logic_get_current_player_info(logic);
    return current && current->type == PLAYER_TYPE_ONLINE;
}

// 🆕 OBTENIR LES INFORMATIONS DU JOUEUR ACTUEL
GamePlayer* game_logic_get_current_player_info(GameLogic* logic) {
    if (!logic) return NULL;
    
    return (logic->current_player == PLAYER_1) ? logic->player1 : logic->player2;
}

GamePlayer* game_logic_get_other_player_info(GameLogic* logic) {
    if (!logic) return NULL;
    
    return (logic->current_player == PLAYER_1) ? logic->player2 : logic->player1;
}

// 🆕 CHANGER DE TOUR
void game_logic_switch_turn(GameLogic* logic) {
    if (!logic || logic->game_finished) return;
    
    GamePlayer* current = game_logic_get_current_player_info(logic);
    GamePlayer* next = game_logic_get_other_player_info(logic);
    
    if (!current || !next) {
        printf("❌ Impossible de changer de tour: joueur NULL\n");
        return;
    }
    
    printf("🔄 Changement de tour: %s → %s\n", current->name, next->name);
    
    // 🆕 NOUVEAU: Arrêter le timer du joueur actuel ET enregistrer le temps du tour
    if (logic->stats_manager) {
        game_stats_stop_turn_timer(logic->stats_manager, current->player_number);
        printf("⏸️ Timer arrêté pour %s après %.2fs de réflexion\n", 
               current->name, current->stats ? current->stats->current_turn_time : 0.0f);
    }
    
    // 🔧 CRITICAL FIX: Sync captures from board/game state to players
    if (logic->board) {
        // Count actual pieces on board for each player using existing board structure
        int white_pieces = 0, black_pieces = 0;
        for (int i = 0; i < NODES; i++) {
            Piece* piece = logic->board->nodes[i].piece;
            if (piece && piece->alive) {
                if (piece->owner == WHITE) white_pieces++;
                else if (piece->owner == BLACK) black_pieces++;
            }
        }
        
        // Calculate captures (starting pieces - current pieces)
        int initial_pieces = 22; // Each player starts with 22 pieces
        
        if (logic->player1->logical_color == WHITE) {
            int p1_captures = initial_pieces - black_pieces; // White captured black pieces
            int p2_captures = initial_pieces - white_pieces; // Black captured white pieces
            
            if (p1_captures != logic->player1->captures_made) {
                printf("🔄 [CAPTURE_SYNC] %s: %d -> %d captures\n", 
                       logic->player1->name, logic->player1->captures_made, p1_captures);
                player_set_captures(logic->player1, p1_captures);
            }
            
            if (p2_captures != logic->player2->captures_made) {
                printf("🔄 [CAPTURE_SYNC] %s: %d -> %d captures\n", 
                       logic->player2->name, logic->player2->captures_made, p2_captures);
                player_set_captures(logic->player2, p2_captures);
            }
        } else {
            int p1_captures = initial_pieces - white_pieces; // Black captured white pieces  
            int p2_captures = initial_pieces - black_pieces; // White captured black pieces
            
            if (p1_captures != logic->player1->captures_made) {
                player_set_captures(logic->player1, p1_captures);
            }
            
            if (p2_captures != logic->player2->captures_made) {
                player_set_captures(logic->player2, p2_captures);
            }
        }
    }
    
    // Changer le joueur actuel
    logic->current_player = (logic->current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
    logic->turn_number++;
    
    // 🔧 FIX: Mettre à jour les états AVANT de démarrer le nouveau timer
    current->is_current_turn = false;
    next->is_current_turn = true;
    
    // 🆕 NOUVEAU: Démarrer le timer du prochain joueur (reset automatique à 0)
    if (logic->stats_manager) {
        game_stats_start_turn_timer(logic->stats_manager, next->player_number);
        printf("▶️ Timer démarré pour %s (nouveau tour)\n", next->name);
    }
    
    // 🆕 Vérifier fin de partie après chaque tour
    if (logic->board) {
        Player winner = check_game_over(logic->board);
        if (winner != NOBODY) {
            logic->game_finished = true;
            logic->winner = winner;
            logic->state = GAME_STATE_GAME_OVER;
            
            // 🆕 ARRÊTER TOUS LES TIMERS en fin de partie
            if (logic->stats_manager) {
                game_stats_stop_turn_timer(logic->stats_manager, 1);
                game_stats_stop_turn_timer(logic->stats_manager, 2);
                printf("⏹️ Tous les timers arrêtés - Partie terminée\n");
            }
            
            GamePlayer* winner_player = (winner == logic->player1->logical_color) ? logic->player1 : logic->player2;
            printf("🏆 PARTIE TERMINÉE! Vainqueur: %s\n", winner_player->name);
            return;
        }
    }
    
    // Déterminer le nouvel état selon le type du prochain joueur
    switch (next->type) {
        case PLAYER_TYPE_HUMAN:
            logic->state = GAME_STATE_WAITING_INPUT;
            printf("⌨️ C'est au tour de %s (Humain, %s) - Timer démarré\n", 
                   next->name, next->logical_color == WHITE ? "Blanc" : "Noir");
            break;
            
        case PLAYER_TYPE_AI:
            logic->state = GAME_STATE_AI_THINKING;
            printf("🤖 L'IA %s commence à réfléchir (Timer IA démarré)...\n", next->name);
            break;
            
        case PLAYER_TYPE_ONLINE:
            logic->state = GAME_STATE_ONLINE_WAITING;
            printf("📡 En attente du joueur distant %s (Timer réseau démarré)...\n", next->name);
            break;
    }
}

// 🆕 MISE À JOUR DE LA LOGIQUE
void game_logic_update(GameLogic* logic, float delta_time) {
    if (!logic) return;
    
    // 🆕 Check pause
    if (logic->state == GAME_STATE_PAUSED) return;
    
    logic->total_game_time += delta_time;
    
    // 🆕 PRIORITÉ: Mettre à jour les timers dans le système de stats EN PREMIER
    if (logic->stats_manager) {
        game_stats_update_timers(logic->stats_manager, delta_time);
        
        // 🔧 FIX: Synchroniser thinking_time avec current_turn_time pour compatibilité
        GamePlayer* current = game_logic_get_current_player_info(logic);
        if (current && current->stats && current->is_current_turn) {
            current->thinking_time = current->stats->current_turn_time;
        }
    }
    
    // Gestion des actions selon l'état
    switch (logic->state) {
        case GAME_STATE_AI_THINKING:
            // 🆕 SIMULATION IA avec timer géré par le système de stats
            if (logic->stats_manager) {
                GamePlayer* current = game_logic_get_current_player_info(logic);
                if (current && current->stats && current->stats->current_turn_time > 2.0f) {
                    printf("💡 [IA SIMULATION] L'IA a trouvé son coup optimal après %.2fs !\n", 
                           current->stats->current_turn_time);
                    printf("🎮 [IA SIMULATION] L'IA joue son coup...\n");
                    
                    // Simuler que l'IA a joué, passer au tour suivant
                    game_logic_switch_turn(logic);
                }
            }
            break;
            
        case GAME_STATE_ONLINE_WAITING:
            // 🆕 SIMULATION MULTIJOUEUR avec timer géré par le système de stats
            if (logic->stats_manager) {
                GamePlayer* current = game_logic_get_current_player_info(logic);
                if (current && current->stats && current->stats->current_turn_time > 5.0f) {
                    printf("⏰ [MULTIJOUEUR SIMULATION] Timeout après %.2fs\n", 
                           current->stats->current_turn_time);
                    printf("📥 [MULTIJOUEUR SIMULATION] Coup du joueur distant appliqué\n");
                    
                    // Simuler qu'on a reçu un coup, passer au tour suivant
                    game_logic_switch_turn(logic);
                }
            }
            break;
            
        default:
            break;
    }
}

// 🆕 VALIDATION D'INTERACTION SELON LE MODE ET LE TOUR
bool game_logic_can_player_interact(GameLogic* logic, Player piece_owner) {
    if (!logic || logic->game_finished) {
        return false;
    }
    
    GameMode mode = logic->mode;
    GamePlayer* current_player = game_logic_get_current_player_info(logic);
    
    if (!current_player) return false;
    
    // 🔧 SIMPLIFIED: Comparer directement avec la couleur logique du joueur actuel
    Player current_logical_color = current_player->logical_color;
    
    printf("🔍 [INTERACTION_CHECK] Pièce: %s, Joueur actuel: %s (logical: %s)\n",
           piece_owner == WHITE ? "WHITE" : "BLACK",
           current_player->name,
           current_logical_color == WHITE ? "WHITE" : "BLACK");
    
    // 🎮 MODE MULTIJOUEUR LOCAL : Simple comparaison
    if (mode == GAME_MODE_LOCAL_MULTIPLAYER) {
        bool can_interact = (piece_owner == current_logical_color);
        printf("🎮 [LOCAL_MULTIPLAYER] Interaction %s\n",
               can_interact ? "AUTORISÉE" : "REFUSÉE");
        return can_interact;
    }
    
    // 🤖 MODE VS IA : Seul le joueur humain peut interagir avec ses pièces
    if (mode == GAME_MODE_VS_AI) {
        // Trouver quel joueur est humain
        GamePlayer* human_player = NULL;
        if (logic->player1->type == PLAYER_TYPE_HUMAN) {
            human_player = logic->player1;
        } else if (logic->player2->type == PLAYER_TYPE_HUMAN) {
            human_player = logic->player2;
        }
        
        if (!human_player) return false;
        
        // L'interaction est autorisée si :
        // 1. C'est le tour du joueur humain
        // 2. La pièce appartient au joueur humain
        bool is_human_turn = (current_player == human_player);
        bool is_human_piece = (piece_owner == human_player->logical_color);
        
        bool can_interact = is_human_turn && is_human_piece;
        printf("🤖 [VS_AI] Interaction %s (tour humain: %s, pièce humaine: %s)\n",
               can_interact ? "AUTORISÉE" : "REFUSÉE",
               is_human_turn ? "OUI" : "NON",
               is_human_piece ? "OUI" : "NON");
        return can_interact;
    }
    
    // 🌐 MODE MULTIJOUEUR EN LIGNE : Seul le joueur local peut interagir avec ses pièces
    if (mode == GAME_MODE_ONLINE_MULTIPLAYER) {
        // Trouver quel joueur est local (HUMAN)
        GamePlayer* local_player = NULL;
        if (logic->player1->type == PLAYER_TYPE_HUMAN) {
            local_player = logic->player1;
        } else if (logic->player2->type == PLAYER_TYPE_HUMAN) {
            local_player = logic->player2;
        }
        
        if (!local_player) return false;
        
        // L'interaction est autorisée si :
        // 1. C'est le tour du joueur local
        // 2. La pièce appartient au joueur local
        bool is_local_turn = (current_player == local_player);
        bool is_local_piece = (piece_owner == local_player->logical_color);
        
        bool can_interact = is_local_turn && is_local_piece;
        printf("🌐 [ONLINE_MULTIPLAYER] Interaction %s (tour local: %s, pièce locale: %s)\n",
               can_interact ? "AUTORISÉE" : "REFUSÉE",
               is_local_turn ? "OUI" : "NON",
               is_local_piece ? "OUI" : "NON");
        return can_interact;
    }
    
    return false;
}

// 🆕 VÉRIFIER SI C'EST LE TOUR D'UN JOUEUR LOCAL
bool game_logic_is_local_player_turn(GameLogic* logic, int player_number) {
    if (!logic || logic->game_finished) return false;
    
    bool is_correct_turn = (player_number == 1 && logic->current_player == PLAYER_1) ||
                           (player_number == 2 && logic->current_player == PLAYER_2);
    
    if (!is_correct_turn) return false;
    
    GamePlayer* player = (player_number == 1) ? logic->player1 : logic->player2;
    return player && player->type == PLAYER_TYPE_HUMAN;
}

// 🆕 VÉRIFIER SI ON PEUT HOVER UNE PIÈCE
bool game_logic_can_hover_piece(GameLogic* logic, Player piece_owner) {
    return game_logic_can_player_interact(logic, piece_owner);
}

// 🆕 VÉRIFIER SI ON PEUT SÉLECTIONNER UNE PIÈCE
bool game_logic_can_select_piece(GameLogic* logic, Player piece_owner) {
    return game_logic_can_player_interact(logic, piece_owner);
}

// 🆕 FONCTIONS UTILITAIRES
const char* game_logic_state_to_string(GameState state) {
    switch (state) {
        case GAME_STATE_MENU:           return "Menu";
        case GAME_STATE_WAITING_INPUT:  return "Attente joueur";
        case GAME_STATE_AI_THINKING:    return "IA réfléchit";
        case GAME_STATE_ONLINE_WAITING: return "Attente réseau";
        case GAME_STATE_GAME_OVER:      return "Partie terminée";
        case GAME_STATE_PAUSED:         return "Pause";
        default:                        return "État inconnu";
    }
}

const char* game_logic_player_type_to_string(PlayerType type) {
    switch (type) {
        case PLAYER_TYPE_HUMAN:  return "Humain";
        case PLAYER_TYPE_AI:     return "IA";
        case PLAYER_TYPE_ONLINE: return "Distant";
        default:                 return "Inconnu";
    }
}

// 🆕 GESTION DE LA PAUSE
void game_logic_toggle_pause(GameLogic* logic) {
    if (!logic) return;
    
    if (logic->state == GAME_STATE_PAUSED) {
        // Reprendre le jeu
        GamePlayer* current = game_logic_get_current_player_info(logic);
        if (current->type == PLAYER_TYPE_HUMAN) {
            logic->state = GAME_STATE_WAITING_INPUT;
        } else if (current->type == PLAYER_TYPE_AI) {
            logic->state = GAME_STATE_AI_THINKING;
        } else {
            logic->state = GAME_STATE_ONLINE_WAITING;
        }
        printf("▶️ JEU REPRIS (Etat: %s)\n", game_logic_state_to_string(logic->state));
    } else {
        // Mettre en pause
        if (logic->state != GAME_STATE_GAME_OVER) {
            logic->state = GAME_STATE_PAUSED;
            printf("⏸️ JEU EN PAUSE\n");
        }
    }
}

void game_logic_set_pause(GameLogic* logic, bool paused) {
    if (!logic) return;
    bool is_paused = (logic->state == GAME_STATE_PAUSED);
    if (paused != is_paused) {
        game_logic_toggle_pause(logic);
    }
}

// 🆕 DEBUG DE LA LOGIQUE
void game_logic_debug_print(GameLogic* logic) {
    if (!logic) return;
    
    printf("\n=== DEBUG GAME LOGIC ===\n");
    printf("Etat: %s\n", game_logic_state_to_string(logic->state));
    printf("Mode: %s\n", config_mode_to_string(logic->mode));
    printf("Tour: %d\n", logic->turn_number);
    printf("Joueur actuel: %s (%s)\n", 
           logic->current_player == PLAYER_1 ? "Joueur 1" : "Joueur 2",
           logic->current_player == PLAYER_1 ? logic->player1->name : logic->player2->name);
    
    printf("\nJoueur 1 (joue en premier):\n");
    printf("   Nom: %s\n", logic->player1->name);
    printf("   Type: %s\n", game_logic_player_type_to_string(logic->player1->type));
    printf("   Couleur visuelle: %s\n", piece_color_to_string(logic->player1->piece_color));
    printf("   Son tour: %s\n", logic->player1->is_current_turn ? "Oui" : "Non");
    printf("   Captures: %d\n", logic->player1->captures_made);
    printf("   Temps de reflexion: %.1fs\n", logic->player1->thinking_time);
    
    printf("\nJoueur 2:\n");
    printf("   Nom: %s\n", logic->player2->name);
    printf("   Type: %s\n", game_logic_player_type_to_string(logic->player2->type));
    printf("   Couleur visuelle: %s\n", piece_color_to_string(logic->player2->piece_color));
    printf("   Son tour: %s\n", logic->player2->is_current_turn ? "Oui" : "Non");
    printf("   Captures: %d\n", logic->player2->captures_made);
    printf("   Temps de reflexion: %.1fs\n", logic->player2->thinking_time);
    
    printf("\nTemps total: %.1fs\n", logic->total_game_time);
    printf("Partie terminee: %s\n", logic->game_finished ? "Oui" : "Non");
    printf("========================\n\n");
}
