#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "../logic/logic.h"  // For Player type

// === PROTOCOL CONSTANTS ===
#define PROTOCOL_VERSION 1
#define PROTOCOL_MAX_PACKET_SIZE 4096
#define PROTOCOL_MAX_PLAYER_NAME 64
#define PROTOCOL_MAX_GAME_NAME 128
#define PROTOCOL_MAX_IP_LENGTH 32

// === MESSAGE TYPES ===
typedef enum {
    MSG_DISCOVER_REQUEST = 1,
    MSG_DISCOVER_REPLY,
    MSG_DISCOVER_RESPONSE,  // Legacy alias
    MSG_CONNECT_REQUEST,
    MSG_CONNECT_ACCEPT,
    MSG_CONNECT_REJECT,
    MSG_CONNECT_RESPONSE,   // Legacy alias
    MSG_MOVE,
    MSG_STATE_SYNC,
    MSG_CHAT,
    MSG_PING,
    MSG_PONG,
    MSG_DISCONNECT,
    MSG_TYPE_COUNT
} ProtocolMessageType;

// === GAME STATUS ===
typedef enum {
    GAME_STATUS_WAITING,
    GAME_STATUS_IN_PROGRESS,
    GAME_STATUS_FINISHED,
    GAME_STATUS_CANCELLED
} GameStatus;

// === PROTOCOL HEADER ===
typedef struct {
    uint16_t version;
    ProtocolMessageType type;
    uint32_t timestamp;
    uint16_t payload_size;
    uint16_t sequence;
} ProtocolHeader;

// === PAYLOAD STRUCTURES ===
typedef struct {
    char player_name[PROTOCOL_MAX_PLAYER_NAME];
} DiscoverRequestPayload;

typedef struct {
    char game_name[PROTOCOL_MAX_GAME_NAME];
    char host_ip[PROTOCOL_MAX_IP_LENGTH];
    uint16_t host_port;
    GameStatus status;
} DiscoverReplyPayload;

typedef struct {
    char player_name[PROTOCOL_MAX_PLAYER_NAME];
} ConnectRequestPayload;

typedef struct {
    bool accepted;
    char reason[128];
} ConnectReplyPayload;

typedef struct {
    int from_id;
    int to_id;
    Player player;
    bool is_capture;
    int capture_count;
    int captured_ids[16];
} MovePayload;

typedef struct {
    uint32_t timestamp;
} PingPayload;

typedef struct {
    uint32_t timestamp;
} PongPayload;

// === NETWORK GAME STATE ===
typedef struct {
    Player current_player;
    uint32_t turn_number;
    uint32_t game_time;
    int piece_positions[45]; // NODES positions
    Player piece_owners[45];
    bool pieces_alive[45];
    GameStatus status;
} NetworkGameState;

// === MAIN MESSAGE STRUCTURE ===
typedef struct {
    ProtocolHeader header;
    void* payload;
} ProtocolMessage;

// === LEGACY COMPATIBILITY STRUCTURES ===
typedef struct {
    char player_name[64];
} DiscoverRequestData;

typedef struct {
    char game_name[64];
    char host_name[64];
    GameStatus status;
} DiscoverResponseData;

typedef struct {
    char player_name[64];
} ConnectRequestData;

typedef struct {
    bool accepted;
    char reason[128];
} ConnectResponseData;

typedef struct {
    int from_id;
    int to_id;
    Player player;
} MoveData;

typedef struct {
    NetworkGameState state;
} StateSyncData;

typedef struct {
    uint32_t timestamp;
} PingData;

typedef struct {
    uint32_t timestamp;
} PongData;

// Legacy union structure for compatibility
typedef union {
    DiscoverRequestData discover_request;
    DiscoverResponseData discover_response;
    ConnectRequestData connect_request;
    ConnectResponseData connect_response;
    MoveData move;
    StateSyncData state_sync;
    PingData ping;
    PongData pong;
} ProtocolData;

// Legacy message structure with union
typedef struct {
    struct {
        ProtocolMessageType type;
        uint16_t size;
        uint16_t sequence;
        uint32_t timestamp;
    } header;
    ProtocolData data;
} LegacyProtocolMessage;

// === FUNCTION DECLARATIONS ===

// Message lifecycle
ProtocolMessage* protocol_message_create(ProtocolMessageType type);
void protocol_message_destroy(ProtocolMessage* msg);

// Serialization
bool protocol_message_serialize(ProtocolMessage* msg, void* buffer, size_t buffer_size, size_t* out_size);
ProtocolMessage* protocol_message_deserialize(const void* buffer, size_t buffer_size);

// Message creators
ProtocolMessage* protocol_create_discover_request(const char* player_name);
ProtocolMessage* protocol_create_discover_reply(const char* game_name, const char* host_ip, 
                                                uint16_t host_port, GameStatus status);
ProtocolMessage* protocol_create_move_message(int from_id, int to_id, Player player);

// Legacy message creators
LegacyProtocolMessage* protocol_create_legacy_connect_request(const char* player_name);
LegacyProtocolMessage* protocol_create_legacy_move_message(int from_id, int to_id, Player player);

// Payload extractors
bool protocol_extract_discover_request(ProtocolMessage* msg, DiscoverRequestPayload* out);
bool protocol_extract_discover_reply(ProtocolMessage* msg, DiscoverReplyPayload* out);
bool protocol_extract_move(ProtocolMessage* msg, MovePayload* out);
bool protocol_extract_state_sync(ProtocolMessage* msg, NetworkGameState* out);

// Utilities
const char* protocol_message_type_to_string(ProtocolMessageType type);

// Legacy compatibility functions
bool protocol_validate_message(const LegacyProtocolMessage* msg);
int protocol_serialize_message(const LegacyProtocolMessage* msg, uint8_t* buffer, int buffer_size);
LegacyProtocolMessage* protocol_deserialize_message(const uint8_t* buffer, int buffer_size);

#endif // PROTOCOL_H
