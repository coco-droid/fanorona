#include "protocol.h"
#include <string.h>
#include <stdio.h>

// === PROTOCOL IMPLEMENTATION ===

const char* protocol_message_type_to_string(ProtocolMessageType type) {
    switch (type) {
        case MSG_DISCOVER_REQUEST: return "DISCOVER_REQUEST";
        case MSG_DISCOVER_REPLY: return "DISCOVER_REPLY";
        case MSG_CONNECT_REQUEST: return "CONNECT_REQUEST";
        case MSG_CONNECT_ACCEPT: return "CONNECT_ACCEPT";
        case MSG_CONNECT_REJECT: return "CONNECT_REJECT";
        case MSG_MOVE: return "MOVE";
        case MSG_STATE_SYNC: return "STATE_SYNC";
        case MSG_CHAT: return "CHAT";
        case MSG_PING: return "PING";
        case MSG_PONG: return "PONG";
        case MSG_DISCONNECT: return "DISCONNECT";
        default: return "UNKNOWN";
    }
}

ProtocolMessage* protocol_message_create(ProtocolMessageType type) {
    ProtocolMessage* msg = (ProtocolMessage*)calloc(1, sizeof(ProtocolMessage));
    if (!msg) return NULL;
    
    msg->header.version = PROTOCOL_VERSION;
    msg->header.type = type;
    msg->header.timestamp = SDL_GetTicks();
    msg->header.payload_size = 0;
    msg->payload = NULL;
    
    return msg;
}

void protocol_message_destroy(ProtocolMessage* msg) {
    if (!msg) return;
    if (msg->payload) {
        free(msg->payload);
    }
    free(msg);
}

bool protocol_message_serialize(ProtocolMessage* msg, void* buffer, size_t buffer_size, size_t* out_size) {
    if (!msg || !buffer || !out_size) return false;
    
    size_t required_size = sizeof(ProtocolHeader) + msg->header.payload_size;
    if (buffer_size < required_size) return false;
    
    // Copy header
    memcpy(buffer, &msg->header, sizeof(ProtocolHeader));
    
    // Copy payload if present
    if (msg->header.payload_size > 0 && msg->payload) {
        memcpy((char*)buffer + sizeof(ProtocolHeader), msg->payload, msg->header.payload_size);
    }
    
    *out_size = required_size;
    return true;
}

ProtocolMessage* protocol_message_deserialize(const void* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < sizeof(ProtocolHeader)) return NULL;
    
    ProtocolMessage* msg = (ProtocolMessage*)calloc(1, sizeof(ProtocolMessage));
    if (!msg) return NULL;
    
    // Copy header
    memcpy(&msg->header, buffer, sizeof(ProtocolHeader));
    
    // Validate version
    if (msg->header.version != PROTOCOL_VERSION) {
        free(msg);
        return NULL;
    }
    
    // Copy payload if present
    if (msg->header.payload_size > 0) {
        if (buffer_size < sizeof(ProtocolHeader) + msg->header.payload_size) {
            free(msg);
            return NULL;
        }
        
        msg->payload = malloc(msg->header.payload_size);
        if (!msg->payload) {
            free(msg);
            return NULL;
        }
        
        memcpy(msg->payload, (const char*)buffer + sizeof(ProtocolHeader), msg->header.payload_size);
    }
    
    return msg;
}

// === HELPER MESSAGE CREATORS ===

ProtocolMessage* protocol_create_discover_request(const char* player_name) {
    ProtocolMessage* msg = protocol_message_create(MSG_DISCOVER_REQUEST);
    if (!msg) return NULL;
    
    DiscoverRequestPayload payload;
    strncpy(payload.player_name, player_name, sizeof(payload.player_name) - 1);
    payload.player_name[sizeof(payload.player_name) - 1] = '\0';
    
    msg->header.payload_size = sizeof(DiscoverRequestPayload);
    msg->payload = malloc(msg->header.payload_size);
    if (!msg->payload) {
        free(msg);
        return NULL;
    }
    
    memcpy(msg->payload, &payload, sizeof(DiscoverRequestPayload));
    return msg;
}

ProtocolMessage* protocol_create_discover_reply(const char* game_name, const char* host_ip, 
                                                 uint16_t host_port, GameStatus status) {
    ProtocolMessage* msg = protocol_message_create(MSG_DISCOVER_REPLY);
    if (!msg) return NULL;
    
    DiscoverReplyPayload payload;
    strncpy(payload.game_name, game_name, sizeof(payload.game_name) - 1);
    payload.game_name[sizeof(payload.game_name) - 1] = '\0';
    strncpy(payload.host_ip, host_ip, sizeof(payload.host_ip) - 1);
    payload.host_ip[sizeof(payload.host_ip) - 1] = '\0';
    payload.host_port = host_port;
    payload.status = status;
    
    msg->header.payload_size = sizeof(DiscoverReplyPayload);
    msg->payload = malloc(msg->header.payload_size);
    if (!msg->payload) {
        free(msg);
        return NULL;
    }
    
    memcpy(msg->payload, &payload, sizeof(DiscoverReplyPayload));
    return msg;
}

ProtocolMessage* protocol_create_move(int from_id, int to_id, Player player) {
    ProtocolMessage* msg = protocol_message_create(MSG_MOVE);
    if (!msg) return NULL;
    
    MovePayload payload;
    payload.from_id = from_id;
    payload.to_id = to_id;
    payload.player = player;
    
    msg->header.payload_size = sizeof(MovePayload);
    msg->payload = malloc(msg->header.payload_size);
    if (!msg->payload) {
        free(msg);
        return NULL;
    }
    
    memcpy(msg->payload, &payload, sizeof(MovePayload));
    return msg;
}

ProtocolMessage* protocol_create_state_sync(const NetworkGameState* state) {
    ProtocolMessage* msg = protocol_message_create(MSG_STATE_SYNC);
    if (!msg || !state) return NULL;
    
    msg->header.payload_size = sizeof(NetworkGameState);
    msg->payload = malloc(msg->header.payload_size);
    if (!msg->payload) {
        free(msg);
        return NULL;
    }
    
    memcpy(msg->payload, state, sizeof(NetworkGameState));
    return msg;
}

ProtocolMessage* protocol_create_ping(void) {
    return protocol_message_create(MSG_PING);
}

ProtocolMessage* protocol_create_pong(void) {
    return protocol_message_create(MSG_PONG);
}

// === PAYLOAD EXTRACTORS ===

bool protocol_extract_discover_request(ProtocolMessage* msg, DiscoverRequestPayload* out) {
    if (!msg || !out || msg->header.type != MSG_DISCOVER_REQUEST) return false;
    if (msg->header.payload_size != sizeof(DiscoverRequestPayload)) return false;
    
    memcpy(out, msg->payload, sizeof(DiscoverRequestPayload));
    return true;
}

bool protocol_extract_discover_reply(ProtocolMessage* msg, DiscoverReplyPayload* out) {
    if (!msg || !out || msg->header.type != MSG_DISCOVER_REPLY) return false;
    if (msg->header.payload_size != sizeof(DiscoverReplyPayload)) return false;
    
    memcpy(out, msg->payload, sizeof(DiscoverReplyPayload));
    return true;
}

bool protocol_extract_move(ProtocolMessage* msg, MovePayload* out) {
    if (!msg || !out || msg->header.type != MSG_MOVE) return false;
    if (msg->header.payload_size != sizeof(MovePayload)) return false;
    
    memcpy(out, msg->payload, sizeof(MovePayload));
    return true;
}

bool protocol_extract_state_sync(ProtocolMessage* msg, NetworkGameState* out) {
    if (!msg || !out || msg->header.type != MSG_STATE_SYNC) return false;
    if (msg->header.payload_size != sizeof(NetworkGameState)) return false;
    
    memcpy(out, msg->payload, sizeof(NetworkGameState));
    return true;
}
