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

// DEAL WITH RECVFROM TIMEOUT

void client::init_client(uint16_t local_port)
{
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(local_port);

    if ((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw std::runtime_error("server_init_error\n");
    }

    bind(client_sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
}

void client::reliable_connect_to_server(const char *server_ip, uint16_t server_port)
{

    xdrmem_create(&xdr_send, send_buffer, sizeof(send_buffer), XDR_ENCODE);
    xdrmem_create(&xdr_recv, recv_buffer, sizeof(recv_buffer), XDR_DECODE);

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(server_port);

    char message[2];

    message[0] = '1';
    message[1] = '\0';

    ssize_t bytes_sent = sendto(client_sockfd, message, strlen(message), 0,
                                (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (bytes_sent <= 0)
    {
        throw std::runtime_error("connection_error");
    }
    socklen_t len = sizeof(struct sockaddr_in);

    int bytes_received = recvfrom(client_sockfd, recv_buffer, MAX_TRANSMITTED_LEN, 0, (struct sockaddr *)&serv_addr, &len);
    if (bytes_received < 0)
    {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
    }

    packet_data data;
    data.payload = new char[10];

    if (!xdr_packet_data(&xdr_recv, &data))
    {
        printf("Error deserializing data\n");
        delete[] data.payload;
        xdr_destroy(&xdr_recv);
        xdr_destroy(&xdr_send);
    }
    xor_swap(data.ack, data.seq);
    if (!xdr_packet_data(&xdr_send, &data))
    {
        printf("Error serializing data\n");
        delete[] data.payload;
        xdr_destroy(&xdr_recv);
        xdr_destroy(&xdr_send);
    }
    int bytes = sendto(client_sockfd, send_buffer, xdr_getpos(&xdr_send), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

int client::connect_to_server(const char *server_ip, uint16_t server_port)
{

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(server_port);

    char message[2];
    message[0] = '0';
    message[1] = '\0';

    ssize_t bytes_sent = sendto(client_sockfd, message, strlen(message), 0,
                                (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("%d\n", bytes_sent);
    if (bytes_sent <= 0)
    {
        throw std::runtime_error("connection_error");
    }

    char *buf = new char[10];

    socklen_t len = sizeof(struct sockaddr_in);

    ssize_t bytes_received = recvfrom(client_sockfd, buf, 10, 0,
                                      (struct sockaddr *)&serv_addr, &len);
    std::cout << "port ="
              << ntohs(serv_addr.sin_port) << "\n";
    printf("bytes: %d\n", bytes_received);
    buf[bytes_received] = '\0';
    return 0;
}

int client::receive_from_server(char *buf, size_t len)
{
    int n = recvfrom(client_sockfd, buf, len, 0, NULL, NULL);
    return n;
}

void client::send_to_server(const char *buf, size_t len)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int n = sendto(client_sockfd, buf, len, 0, (struct sockaddr *)&serv_addr, addr_len);
}

void client::unreliable_announce_end()
{
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int n = sendto(client_sockfd, "unreliable_end", 14, 0, (struct sockaddr *)&serv_addr, addr_len);
}