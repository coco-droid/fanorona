#include "p2p.h"
#include <stdio.h>

static P2PStatus current_status = P2P_DISCONNECTED;

bool p2p_host(unsigned short port, MoveRecv move_cb, StatusChanged status_cb) {
    (void)port; (void)move_cb; (void)status_cb;
    printf("P2P hosting not implemented yet\n");
    return false;
}

bool p2p_join(const char *ip, unsigned short port, MoveRecv move_cb, StatusChanged status_cb) {
    (void)ip; (void)port; (void)move_cb; (void)status_cb;
    printf("P2P join not implemented yet\n");
    return false;
}

void p2p_send_move(int fx, int fy, int tx, int ty) {
    (void)fx; (void)fy; (void)tx; (void)ty;
    printf("P2P send move not implemented yet\n");
}

void p2p_send_chat(const char *message) {
    (void)message;
    printf("P2P chat not implemented yet\n");
}

P2PStatus p2p_get_status(void) {
    return current_status;
}

void p2p_disconnect(void) {
    current_status = P2P_DISCONNECTED;
}

void p2p_update(void) {
    // Network update loop
}
