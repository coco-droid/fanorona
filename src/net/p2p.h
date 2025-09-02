#pragma once
#include <stdbool.h>

typedef enum {
    P2P_DISCONNECTED,
    P2P_CONNECTING,
    P2P_CONNECTED,
    P2P_ERROR
} P2PStatus;

typedef void (*MoveRecv)(int from_x, int from_y, int to_x, int to_y);
typedef void (*StatusChanged)(P2PStatus status, const char *message);

bool p2p_host(unsigned short port, MoveRecv move_cb, StatusChanged status_cb);
bool p2p_join(const char *ip, unsigned short port, MoveRecv move_cb, StatusChanged status_cb);
void p2p_send_move(int fx, int fy, int tx, int ty);
void p2p_send_chat(const char *message);
P2PStatus p2p_get_status(void);
void p2p_disconnect(void);
void p2p_update(void); // Call in main loop