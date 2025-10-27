#ifndef NETWORK_H
#define NETWORK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <stdbool.h>
#include "protocol.h"

// === NETWORK CONFIGURATION ===
#define NETWORK_DISCOVERY_PORT 8888
#define NETWORK_GAME_PORT 8889
#define NETWORK_MAX_PACKET_SIZE 4096
#define NETWORK_PING_INTERVAL 5000  // ms
#define NETWORK_TIMEOUT 15000        // ms
#define NETWORK_MAX_LOBBY_GAMES 16

// === CONNECTION STATE ===
typedef enum {
    NET_STATE_DISCONNECTED,
    NET_STATE_DISCOVERING,
    NET_STATE_CONNECTING,
    NET_STATE_CONNECTED,
    NET_STATE_HOSTING,
    NET_STATE_ERROR
} NetworkState;

// === LOBBY GAME ENTRY ===
typedef struct {
    char game_name[64];
    char host_ip[32];
    uint16_t host_port;
    GameStatus status;
    Uint32 last_seen;  // Timestamp for timeout
    bool active;
} LobbyGameEntry;

// === NETWORK MANAGER ===
typedef struct NetworkManager {
    NetworkState state;
    
    // SDL_net objects
    UDPsocket udp_socket;
    TCPsocket tcp_socket;
    SDLNet_SocketSet socket_set;
    
    // Thread management
    SDL_Thread* network_thread;
    SDL_mutex* mutex;
    bool thread_running;
    
    // Connection info
    char local_player_name[64];
    char remote_player_name[64];
    IPaddress local_address;
    IPaddress remote_address;
    
    // Lobby data
    LobbyGameEntry lobby_games[NETWORK_MAX_LOBBY_GAMES];
    int lobby_game_count;
    
    // Packet queue (bridge to game)
    ProtocolMessage* packet_queue[256];
    int queue_head;
    int queue_tail;
    int queue_count;
    
    // Connection monitoring
    Uint32 last_ping_sent;
    Uint32 last_pong_received;
    bool connection_alive;
    
    // Error handling
    char last_error[256];
} NetworkManager;

// === INITIALIZATION & CLEANUP ===
NetworkManager* network_create(void);
void network_destroy(NetworkManager* net);
bool network_init(NetworkManager* net);
void network_shutdown(NetworkManager* net);

// === CONNECTION MANAGEMENT ===
bool network_host_game(NetworkManager* net, const char* game_name, const char* player_name);
bool network_discover_games(NetworkManager* net, const char* player_name);
bool network_connect_to_game(NetworkManager* net, const char* host_ip, uint16_t port, const char* player_name);
void network_disconnect(NetworkManager* net);

// === STATE QUERIES ===
NetworkState network_get_state(NetworkManager* net);
bool network_is_connected(NetworkManager* net);
bool network_is_hosting(NetworkManager* net);
bool network_connection_alive(NetworkManager* net);
const char* network_get_last_error(NetworkManager* net);

// === LOBBY MANAGEMENT ===
int network_get_lobby_game_count(NetworkManager* net);
const LobbyGameEntry* network_get_lobby_game(NetworkManager* net, int index);
void network_refresh_lobby(NetworkManager* net);

// === MESSAGE SENDING ===
bool network_send_move(NetworkManager* net, int from_id, int to_id, Player player);
bool network_send_state_sync(NetworkManager* net, const NetworkGameState* state);
bool network_send_message(NetworkManager* net, ProtocolMessage* msg);

// === MESSAGE RECEIVING (Bridge) ===
ProtocolMessage* network_poll_message(NetworkManager* net);
bool network_has_pending_messages(NetworkManager* net);

// === PING/PONG MANAGEMENT ===
void network_update(NetworkManager* net, float delta_time);
Uint32 network_get_ping(NetworkManager* net);

#endif // NETWORK_H
