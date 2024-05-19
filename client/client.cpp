#include "client.h"
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../utils/protocol_structs.h"
#include "../utils/xdr_serialization.h"
#include <rpc/xdr.h>
#include "../utils/helper_functions.h"
#include "../utils/protocol_consts.h"
#include "../channel/channel.h"

// DEAL WITH RECVFROM TIMEOUT

void client::init_client(uint16_t local_port)
{
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(local_port);
    channel = nullptr;

    if ((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw std::runtime_error("server_init_error\n");
    }

    bind(server_sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
}

int client::connect_to_server(const char *server_ip, uint16_t server_port)
{

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(server_port);

    char message[2];
    message[0] = '0';
    message[1] = '\0';

    ssize_t bytes_sent = sendto(server_sockfd, message, strlen(message), 0,
                                (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("%d\n", bytes_sent);
    if (bytes_sent <= 0)
    {
        throw std::runtime_error("connection_error");
    }

    char *buf = new char[10];

    socklen_t len = sizeof(struct sockaddr_in);

    ssize_t bytes_received = recvfrom(server_sockfd, buf, 10, 0,
                                      (struct sockaddr *)&serv_addr, &len);
    std::cout << "port ="
              << ntohs(serv_addr.sin_port) << "\n";
    printf("bytes: %d\n", bytes_received);
    buf[bytes_received] = '\0';
    return 0;
}

std::string client::receive()
{
    if (channel)
    {
        return channel->receive();
    }
    int n = recvfrom(server_sockfd, buf, len, 0, NULL, NULL);
    buf[n] = '\0';
    return std::string(buf);
}

void client::send(std::string s)
{
    if (channel)
    {
        channel->send(s);
        return;
    }
    strcpy(buf, s.c_str());
    len = s.size();
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int n = sendto(server_sockfd, buf, len, 0, (struct sockaddr *)&serv_addr, addr_len);
}

void client::close()
{
    if (channel)
    {
        channel->close();
        return;
    }
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int n = sendto(server_sockfd, "unreliable_end", 14, 0, (struct sockaddr *)&serv_addr, addr_len);
}