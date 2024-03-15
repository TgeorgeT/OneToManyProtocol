#ifndef CLIENT_H
#define CLIENT_H

#include "rpc/xdr.h"
#include "../utils/protocol_consts.h"

class client
{
private:
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    char recv_buffer[MAX_TRANSMITTED_LEN];
    char send_buffer[MAX_TRANSMITTED_LEN];

    XDR xdr_recv, xdr_send;

    int32_t server_sockfd;
    int32_t client_sockfd;

public:
    void init_client(uint16_t local_port);
    void reliable_connect_to_server(const char *server_ip, uint16_t server_port);
    int connect_to_server(const char *server_ip, uint16_t server_port);
    int receive_from_server(char *buf, size_t len);
    void send_to_server(const char *buf, size_t len);
    void unreliable_announce_end();
};

#endif