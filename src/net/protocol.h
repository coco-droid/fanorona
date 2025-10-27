#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "../types.h"

// Fallback if types.h doesn't define NODES
#ifndef NODES
#define NODES 45  // Standard Fanorona board has 45 nodes
#endif

// === PROTOCOL VERSION ===
#define PROTOCOL_VERSION 1

// === MESSAGE TYPES ===
typedef enum {
    MSG_DISCOVER_REQUEST = 1,
    MSG_DISCOVER_REPLY,
    MSG_CONNECT_REQUEST,
    MSG_CONNECT_ACCEPT,
    MSG_CONNECT_REJECT,
    MSG_MOVE,
    MSG_STATE_SYNC,
    MSG_CHAT,
    MSG_PING,
    MSG_PONG,
    MSG_DISCONNECT
} ProtocolMessageType;

// === GAME STATUS ===
typedef enum {
    GAME_STATUS_WAITING,
    GAME_STATUS_IN_PROGRESS,
    GAME_STATUS_FINISHED
} GameStatus;

// === PROTOCOL HEADER ===
typedef struct {
    uint8_t version;
    uint8_t type;
    uint16_t payload_size;
    uint32_t timestamp;
} ProtocolHeader;

// === PAYLOADS ===

// Discovery
typedef struct {
    char player_name[64];
} DiscoverRequestPayload;

typedef struct {
    char game_name[64];
    char host_ip[32];
    uint16_t host_port;
    GameStatus status;
} DiscoverReplyPayload;

// Move
typedef struct {
    int from_id;
    int to_id;
    Player player;
} MovePayload;

// State synchronization
typedef struct {
    int board[NODES];          // Piece IDs at each position (-1 = empty)
    Player current_player;
    int turn_number;
    bool game_finished;
    Player winner;
} NetworkGameState;

// Chat
typedef struct {
    char message[256];
} ChatPayload;

// === PROTOCOL MESSAGE ===
typedef struct {
    ProtocolHeader header;
    void* payload;
} ProtocolMessage;

// === PROTOCOL FUNCTIONS ===

// Message lifecycle
ProtocolMessage* protocol_message_create(ProtocolMessageType type);
void protocol_message_destroy(ProtocolMessage* msg);

// Serialization
bool protocol_message_serialize(ProtocolMessage* msg, void* buffer, size_t buffer_size, size_t* out_size);
ProtocolMessage* protocol_message_deserialize(const void* buffer, size_t buffer_size);

// Helper creators
ProtocolMessage* protocol_create_discover_request(const char* player_name);
ProtocolMessage* protocol_create_discover_reply(const char* game_name, const char* host_ip, 
                                                 uint16_t host_port, GameStatus status);
ProtocolMessage* protocol_create_move(int from_id, int to_id, Player player);
ProtocolMessage* protocol_create_state_sync(const NetworkGameState* state);
ProtocolMessage* protocol_create_ping(void);
ProtocolMessage* protocol_create_pong(void);

// Payload extractors
bool protocol_extract_discover_request(ProtocolMessage* msg, DiscoverRequestPayload* out);
bool protocol_extract_discover_reply(ProtocolMessage* msg, DiscoverReplyPayload* out);
bool protocol_extract_move(ProtocolMessage* msg, MovePayload* out);
bool protocol_extract_state_sync(ProtocolMessage* msg, NetworkGameState* out);

// Utilities
const char* protocol_message_type_to_string(ProtocolMessageType type);

#endif // PROTOCOL_H
