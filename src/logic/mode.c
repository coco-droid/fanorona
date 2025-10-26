#include "logic.h"
#include "../config.h"
#include "../utils/log_console.h"
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
    
    printf("🧠 GameLogic créée\n");
    return logic;
}

void game_logic_destroy(GameLogic* logic) {
    if (logic) {
        printf("🧹 Destruction de GameLogic\n");
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
    
    // 🔧 FIX: Joueur 1 joue TOUJOURS en premier, peu importe sa couleur
    if (!logic->player1) {
        // Le joueur 1 garde sa couleur de pièce choisie
        logic->player1 = player_create(
            config_get_player1_name(), 
            WHITE,  // Logique: premier = WHITE pour les règles
            config_get_player1_piece_color(),  // Visuel: noir/brun/blanc selon choix
            PLAYER_TYPE_HUMAN, 
            1
        );
    }
    if (!logic->player2) {
        // Le joueur 2 garde sa couleur de pièce choisie
        logic->player2 = player_create(
            config_get_player2_name(), 
            BLACK,  // Logique: second = BLACK pour les règles
            config_get_player2_piece_color(),  // Visuel: noir/brun/blanc selon choix
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
            printf("   👤 Joueur 1 (joue en premier): %s (%s)\n", 
                   logic->player1->name, piece_color_to_string(logic->player1->piece_color));
            printf("   👤 Joueur 2: %s (%s)\n", 
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
    
    logic->current_player = PLAYER_1;  // 🔧 FIX: Toujours le joueur 1 commence
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
    
    printf("🔄 Changement de tour: %s → %s\n", current->name, next->name);
    
    // Changer le joueur actuel
    logic->current_player = (logic->current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
    logic->turn_number++;
    
    // Mettre à jour les flags de tour
    current->is_current_turn = false;
    next->is_current_turn = true;
    
    // Déterminer le nouvel état selon le type du prochain joueur
    switch (next->type) {
        case PLAYER_TYPE_HUMAN:
            logic->state = GAME_STATE_WAITING_INPUT;
            printf("⌨️ C'est au tour de %s (Humain, %s)\n", 
                   next->name, next->logical_color == WHITE ? "Blanc" : "Noir");
            break;
            
        case PLAYER_TYPE_AI:
            logic->state = GAME_STATE_AI_THINKING;
            printf("🤖 L'IA %s commence à réfléchir (Difficulté: %s)...\n", 
                   next->name, config_difficulty_to_string(config_get_ai_difficulty()));
            break;
            
        case PLAYER_TYPE_ONLINE:
            logic->state = GAME_STATE_ONLINE_WAITING;
            printf("📡 En attente du joueur distant %s...\n", next->name);
            break;
    }
}

// 🆕 MISE À JOUR DE LA LOGIQUE
void game_logic_update(GameLogic* logic, float delta_time) {
    if (!logic) return;
    
    logic->total_game_time += delta_time;
    
    // Mettre à jour le temps de réflexion du joueur actuel
    GamePlayer* current = game_logic_get_current_player_info(logic);
    if (current) {
        current->thinking_time += delta_time;
    }
    
    // Gestion des actions selon l'état
    switch (logic->state) {
        case GAME_STATE_AI_THINKING:
            // 🆕 SIMULATION SIMPLE IA
            if (current && current->thinking_time > 2.0f) { // IA réfléchit 2 secondes
                printf("💡 [IA SIMULATION] L'IA a trouvé son coup optimal !\n");
                printf("🎮 [IA SIMULATION] L'IA joue son coup...\n");
                
                // Simuler que l'IA a joué, passer au tour suivant
                game_logic_switch_turn(logic);
            }
            break;
            
        case GAME_STATE_ONLINE_WAITING:
            // 🆕 SIMULATION MULTIJOUEUR
            if (current && current->thinking_time > 5.0f) { // Timeout après 5 secondes
                printf("⏰ [MULTIJOUEUR SIMULATION] Simulation d'un coup reçu du serveur\n");
                printf("📥 [MULTIJOUEUR SIMULATION] Coup du joueur distant appliqué\n");
                
                // Simuler qu'on a reçu un coup, passer au tour suivant
                game_logic_switch_turn(logic);
            }
            break;
            
        default:
            break;
    }
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

// 🆕 DEBUG DE LA LOGIQUE
void game_logic_debug_print(GameLogic* logic) {
    if (!logic) return;
    
    printf("\n=== 🧠 DEBUG GAME LOGIC ===\n");
    printf("État: %s\n", game_logic_state_to_string(logic->state));
    printf("Mode: %s\n", config_mode_to_string(logic->mode));
    printf("Tour: %d\n", logic->turn_number);
    printf("Joueur actuel: %s (%s)\n", 
           logic->current_player == PLAYER_1 ? "Joueur 1" : "Joueur 2",
           logic->current_player == PLAYER_1 ? logic->player1->name : logic->player2->name);
    
    printf("\n👤 Joueur 1 (joue en premier):\n");
    printf("   Nom: %s\n", logic->player1->name);
    printf("   Type: %s\n", game_logic_player_type_to_string(logic->player1->type));
    printf("   Couleur visuelle: %s\n", piece_color_to_string(logic->player1->piece_color));
    printf("   Son tour: %s\n", logic->player1->is_current_turn ? "Oui" : "Non");
    printf("   Captures: %d\n", logic->player1->captures_made);
    printf("   Temps de réflexion: %.1fs\n", logic->player1->thinking_time);
    
    printf("\n👤 Joueur 2:\n");
    printf("   Nom: %s\n", logic->player2->name);
    printf("   Type: %s\n", game_logic_player_type_to_string(logic->player2->type));
    printf("   Couleur visuelle: %s\n", piece_color_to_string(logic->player2->piece_color));
    printf("   Son tour: %s\n", logic->player2->is_current_turn ? "Oui" : "Non");
    printf("   Captures: %d\n", logic->player2->captures_made);
    printf("   Temps de réflexion: %.1fs\n", logic->player2->thinking_time);
    
    printf("\nTemps total: %.1fs\n", logic->total_game_time);
    printf("Partie terminée: %s\n", logic->game_finished ? "Oui" : "Non");
    printf("========================\n\n");
}
