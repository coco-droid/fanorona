#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../types.h"
#include "../plateau/plateau.h"
#include "../pions/pions.h"
#include "../logic/rules.h"  // 🔧 FIX: Include rules.h for Move and MAX_CAPTURE_LIST
#include "../net/protocol.h"

// === TAILLES DES PAQUETS ===
#define SERIALIZER_PLAYER_SIZE 256      // Profil joueur
#define SERIALIZER_MOVE_SIZE 64         // Un coup
#define SERIALIZER_BOARD_SIZE 2048      // État complet du plateau
#define SERIALIZER_GAME_STATE_SIZE 512  // État de la partie

// === STRUCTURES DE SÉRIALISATION ===

// 🆕 Profil joueur sérialisable (256 bytes)
typedef struct {
    char name[64];
    uint8_t logical_color;      // WHITE=1, BLACK=-1
    uint8_t piece_color;        // PIECE_COLOR_*
    uint8_t player_type;        // PLAYER_TYPE_*
    uint8_t player_number;      // 1 ou 2
    uint8_t avatar_id;          // AvatarID
    int32_t pieces_remaining;
    int32_t captures_made;
    float total_time;
    float thinking_time;
    uint8_t is_current_turn;
    uint8_t has_mandatory_capture;
    uint8_t padding[150];       // Réservé pour extensions futures
} SerializedPlayer;

// 🆕 Coup sérialisable (64 bytes)
typedef struct {
    int32_t from_id;
    int32_t to_id;
    uint8_t player;             // WHITE ou BLACK
    uint8_t is_capture;
    uint8_t capture_count;
    uint8_t padding1;
    int32_t captured_ids[MAX_CAPTURE_LIST];
    uint32_t timestamp;
    uint8_t padding2[16];
} SerializedMove;

// 🆕 État du plateau sérialisable (2048 bytes)
typedef struct {
    // Tableau simplifié: -1=vide, 0=blanc, 1=noir
    int8_t piece_positions[NODES];
    uint8_t piece_alive[NODES];
    int32_t piece_count;
    uint32_t board_hash;        // Hash Zobrist pour validation
    uint8_t padding[1920];
} SerializedBoard;

// 🆕 État complet de la partie (512 bytes)
typedef struct {
    uint8_t game_state;         // GameState enum
    uint8_t game_mode;          // GameMode enum
    uint8_t current_player;     // PLAYER_1 ou PLAYER_2
    uint8_t game_finished;
    int8_t winner;              // NOBODY, WHITE, BLACK
    int32_t turn_number;
    float total_game_time;
    SerializedPlayer player1;
    SerializedPlayer player2;
    uint32_t checksum;          // CRC32 pour validation
} SerializedGameState;

// === FONCTIONS DE SÉRIALISATION ===

// 🆕 Joueurs
bool serialize_player(const GamePlayer* player, SerializedPlayer* out);
bool deserialize_player(const SerializedPlayer* data, GamePlayer* out);

// 🆕 Coups
bool serialize_move(const Move* move, Player player, SerializedMove* out);
bool deserialize_move(const SerializedMove* data, Move* out, Player* player);

// 🆕 Plateau
bool serialize_board(const Board* board, SerializedBoard* out);
bool deserialize_board(const SerializedBoard* data, Board* out);

// 🆕 État complet du jeu
bool serialize_game_state(const void* game_logic, SerializedGameState* out);
bool deserialize_game_state(const SerializedGameState* data, void* game_logic);

// === CONVERSION BINAIRE (RÉSEAU) ===

// 🆕 Buffer binaire → Structure
size_t serialize_player_to_buffer(const GamePlayer* player, uint8_t* buffer, size_t buffer_size);
size_t serialize_move_to_buffer(const Move* move, Player player, uint8_t* buffer, size_t buffer_size);
size_t serialize_board_to_buffer(const Board* board, uint8_t* buffer, size_t buffer_size);
size_t serialize_game_state_to_buffer(const void* game_logic, uint8_t* buffer, size_t buffer_size);

// 🆕 Structure → Buffer binaire
bool deserialize_player_from_buffer(const uint8_t* buffer, size_t buffer_size, GamePlayer* out);
bool deserialize_move_from_buffer(const uint8_t* buffer, size_t buffer_size, Move* out, Player* player);
bool deserialize_board_from_buffer(const uint8_t* buffer, size_t buffer_size, Board* out);
bool deserialize_game_state_from_buffer(const uint8_t* buffer, size_t buffer_size, void* game_logic);

// === UTILITAIRES ===

// 🆕 Validation
uint32_t serializer_compute_checksum(const void* data, size_t size);
bool serializer_validate_checksum(const void* data, size_t size, uint32_t expected);
uint32_t serializer_compute_board_hash(const Board* board);

// 🆕 Debug
void serializer_debug_print_player(const SerializedPlayer* player);
void serializer_debug_print_move(const SerializedMove* move);
void serializer_debug_print_board(const SerializedBoard* board);
void serializer_debug_print_game_state(const SerializedGameState* state);

// === INTÉGRATION PROTOCOL.H ===

// 🆕 Conversion vers/depuis ProtocolMessage
ProtocolMessage* serializer_create_player_sync_message(const GamePlayer* player);
ProtocolMessage* serializer_create_move_message(const Move* move, Player player);
ProtocolMessage* serializer_create_board_sync_message(const Board* board);
ProtocolMessage* serializer_create_full_state_message(const void* game_logic);

bool serializer_extract_player_from_message(ProtocolMessage* msg, GamePlayer* out);
bool serializer_extract_move_from_message(ProtocolMessage* msg, Move* out, Player* player);
bool serializer_extract_board_from_message(ProtocolMessage* msg, Board* out);
bool serializer_extract_game_state_from_message(ProtocolMessage* msg, void* game_logic);

#endif // SERIALIZER_H
