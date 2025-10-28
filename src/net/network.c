#include "network.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === INITIALIZATION & CLEANUP ===
NetworkManager* network_create(void) {
    NetworkManager* net = (NetworkManager*)calloc(1, sizeof(NetworkManager));
    if (!net) return NULL;
    
    net->state = NET_STATE_DISCONNECTED;
    net->udp_socket = NULL;
    net->tcp_socket = NULL;
    net->socket_set = NULL;
    net->network_thread = NULL;
    net->mutex = NULL;
    net->thread_running = false;
    net->lobby_game_count = 0;
    net->queue_head = 0;
    net->queue_tail = 0;
    net->queue_count = 0;
    net->connection_alive = false;
    strcpy(net->last_error, "No error");
    
    printf("🌐 NetworkManager created (stub implementation)\n");
    return net;
}

void network_destroy(NetworkManager* net) {
    if (!net) return;
    
    network_disconnect(net);
    
    if (net->mutex) {
        SDL_DestroyMutex(net->mutex);
    }
    
    free(net);
    printf("🗑️ NetworkManager destroyed\n");
}

bool network_init(NetworkManager* net) {
    if (!net) return false;
    
    // Initialize SDL_net
    if (SDLNet_Init() < 0) {
        snprintf(net->last_error, sizeof(net->last_error), "SDL_net init failed: %s", SDLNet_GetError());
        return false;
    }
    
    net->mutex = SDL_CreateMutex();
    if (!net->mutex) {
        snprintf(net->last_error, sizeof(net->last_error), "Mutex creation failed");
        return false;
    }
    
    printf("✅ Network initialized (stub mode)\n");
    return true;
}

void network_shutdown(NetworkManager* net) {
    if (!net) return;
    
    network_disconnect(net);
    SDLNet_Quit();
    
    printf("🔌 Network shutdown\n");
}

// === CONNECTION MANAGEMENT ===
bool network_host_game(NetworkManager* net, const char* game_name, const char* player_name) {
    if (!net) return false;
    
    strncpy(net->local_player_name, player_name, sizeof(net->local_player_name) - 1);
    net->state = NET_STATE_HOSTING;
    
    printf("🏠 Hosting game '%s' as '%s' (stub)\n", game_name, player_name);
    return true;
}

bool network_discover_games(NetworkManager* net, const char* player_name) {
    if (!net) return false;
    
    strncpy(net->local_player_name, player_name, sizeof(net->local_player_name) - 1);
    net->state = NET_STATE_DISCOVERING;
    
    // Simulate finding some games
    net->lobby_game_count = 3;
    strcpy(net->lobby_games[0].game_name, "Partie de test 1");
    strcpy(net->lobby_games[0].host_ip, "192.168.1.100");
    net->lobby_games[0].host_port = NETWORK_GAME_PORT;
    net->lobby_games[0].status = GAME_STATUS_WAITING;
    net->lobby_games[0].active = true;
    
    strcpy(net->lobby_games[1].game_name, "Partie de test 2");
    strcpy(net->lobby_games[1].host_ip, "192.168.1.101");
    net->lobby_games[1].host_port = NETWORK_GAME_PORT;
    net->lobby_games[1].status = GAME_STATUS_WAITING;
    net->lobby_games[1].active = true;
    
    strcpy(net->lobby_games[2].game_name, "Partie de test 3");
    strcpy(net->lobby_games[2].host_ip, "192.168.1.102");
    net->lobby_games[2].host_port = NETWORK_GAME_PORT;
    net->lobby_games[2].status = GAME_STATUS_IN_PROGRESS;
    net->lobby_games[2].active = true;
    
    printf("🔍 Discovering games as '%s' (found %d games)\n", player_name, net->lobby_game_count);
    return true;
}

bool network_connect_to_game(NetworkManager* net, const char* host_ip, uint16_t port, const char* player_name) {
    if (!net) return false;
    
    strncpy(net->local_player_name, player_name, sizeof(net->local_player_name) - 1);
    strncpy(net->remote_player_name, "Remote Player", sizeof(net->remote_player_name) - 1);
    net->state = NET_STATE_CONNECTED;
    net->connection_alive = true;
    
    printf("🔗 Connected to %s:%d as '%s' (stub)\n", host_ip, port, player_name);
    return true;
}

void network_disconnect(NetworkManager* net) {
    if (!net) return;
    
    net->thread_running = false;
    
    if (net->network_thread) {
        SDL_WaitThread(net->network_thread, NULL);
        net->network_thread = NULL;
    }
    
    if (net->udp_socket) {
        SDLNet_UDP_Close(net->udp_socket);
        net->udp_socket = NULL;
    }
    
    if (net->tcp_socket) {
        SDLNet_TCP_Close(net->tcp_socket);
        net->tcp_socket = NULL;
    }
    
    if (net->socket_set) {
        SDLNet_FreeSocketSet(net->socket_set);
        net->socket_set = NULL;
    }
    
    net->state = NET_STATE_DISCONNECTED;
    net->connection_alive = false;
    
    printf("🔌 Network disconnected\n");
}

// === STATE QUERIES ===
NetworkState network_get_state(NetworkManager* net) {
    return net ? net->state : NET_STATE_ERROR;
}

bool network_is_connected(NetworkManager* net) {
    return net && net->state == NET_STATE_CONNECTED;
}

bool network_is_hosting(NetworkManager* net) {
    return net && net->state == NET_STATE_HOSTING;
}

bool network_connection_alive(NetworkManager* net) {
    return net && net->connection_alive;
}

const char* network_get_last_error(NetworkManager* net) {
    return net ? net->last_error : "Network manager is NULL";
}

// === LOBBY MANAGEMENT ===
int network_get_lobby_game_count(NetworkManager* net) {
    return net ? net->lobby_game_count : 0;
}

const LobbyGameEntry* network_get_lobby_game(NetworkManager* net, int index) {
    if (!net || index < 0 || index >= net->lobby_game_count) {
        return NULL;
    }
    return &net->lobby_games[index];
}

void network_refresh_lobby(NetworkManager* net) {
    if (!net) return;
    
    // Simulate refreshing the lobby (in real implementation, send discovery packets)
    printf("🔄 Refreshing lobby (stub)\n");
}

// === MESSAGE SENDING ===
bool network_send_move(NetworkManager* net, int from_id, int to_id, Player player) {
    if (!net || !network_is_connected(net)) return false;
    
    printf("📤 Sending move: %d -> %d (player %d) (stub)\n", from_id, to_id, player);
    return true;
}

bool network_send_state_sync(NetworkManager* net, const NetworkGameState* state) {
    if (!net || !network_is_connected(net) || !state) return false;
    
    printf("📤 Sending state sync (stub)\n");
    return true;
}

bool network_send_message(NetworkManager* net, ProtocolMessage* msg) {
    if (!net || !msg) return false;
    
    printf("📤 Sending message type %d (stub)\n", msg->header.type);
    return true;
}

// === MESSAGE RECEIVING ===
ProtocolMessage* network_poll_message(NetworkManager* net) {
    if (!net || net->queue_count == 0) return NULL;
    
    // Simulate no messages for now
    printf("📥 Polling message (stub - no messages)\n");
    return NULL;
}

bool network_has_pending_messages(NetworkManager* net) {
    if (!net) return false;
    
    // Simulate no pending messages
    return false;
}

// === PING/PONG MANAGEMENT ===
void network_update(NetworkManager* net, float delta_time) {
    if (!net) return;
    
    // Suppress unused parameter warning
    (void)delta_time;
    
    // Update connection monitoring
    Uint32 now = SDL_GetTicks();
    
    // Simulate connection checks
    if (net->state == NET_STATE_CONNECTED) {
        if (now - net->last_pong_received > NETWORK_TIMEOUT) {
            net->connection_alive = false;
            net->state = NET_STATE_ERROR;
            strcpy(net->last_error, "Connection timeout");
        }
    }
    
    // Update last_pong_received to prevent immediate timeout
    if (net->last_pong_received == 0) {
        net->last_pong_received = now;
    }
}

Uint32 network_get_ping(NetworkManager* net) {
    if (!net) return 0;
    
    // Simulate 50ms ping
    return 50;
}
