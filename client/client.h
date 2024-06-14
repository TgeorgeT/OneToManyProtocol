#ifndef CLIENT_H
#define CLIENT_H

#include "rpc/xdr.h"
#include "../channel/channel.h"
#include "../utils/protocol_consts.h"

class Client
{
public:
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    int32_t server_sockfd;
    int32_t client_sockfd;

    reliable_channel *channel;
    char buf[MAX_TRANSMITTED_LEN];
    int len;

    void
    create_reliable_channel(int sockfd, struct sockaddr_in addr);

public:
    void init(uint16_t local_port);
    void reliable_connect_to_one(const char *server_ip, uint16_t server_port);
    int connect_to_server(const char *server_ip, uint16_t server_port);
    std::string receive();
    void send(std::string s);
    void close();
};

#endif