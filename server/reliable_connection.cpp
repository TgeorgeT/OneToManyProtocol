#include "server.h"
#include "rpc/xdr.h"
#include "../utils/xdr_serialization.h"

// handle flags

void Server::handle_new_reliable_connection(struct sockaddr_in cli_addr)
{
    packet_data p;
    p.ack = 1;

    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in *new_client = new sockaddr_in;

    bzero(new_client, sizeof(&new_client));

    new_client->sin_family = AF_INET;
    new_client->sin_addr.s_addr = INADDR_ANY;
    new_client->sin_port = 0;

    if (bind(sockfd, (struct sockaddr *)new_client, sizeof(*new_client)) < 0)
    {
        printf("New connection bind error: %s\n", strerror(errno));
    }

    socklen_t len = sizeof(new_client);

    if (getsockname(sockfd, (struct sockaddr *)new_client, &len) == -1)
    {
        printf("getsockname error:%s\n", strerror(errno));
    }

    p.payload = new char[10];

    p.payload_len = strlen(p.payload);
    p.seq = p.payload_len;

    XDR xdr_send;
    char send_buffer[50];
    xdrmem_create(&xdr_send, send_buffer, sizeof(send_buffer), XDR_ENCODE);

    if (!xdr_packet_data(&xdr_send, &p))
    {
        printf("Error serializing connection data\n");
        xdr_destroy(&xdr_send);
        delete (p.payload);
        close(sockfd);
    }

    int seq = p.seq;

    int buf_len = xdr_getpos(&xdr_send);

    if (sendto(sockfd, send_buffer, buf_len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
    {
        printf("sendto failed\n");
        delete (p.payload);
        close(sockfd);
    }

    XDR xdr_recv;
    char recv_buffer[50];
    xdrmem_create(&xdr_recv, recv_buffer, sizeof(recv_buffer), XDR_DECODE);

    if (recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, nullptr, nullptr) < 0)
    {
        printf("recvfrom failed\n");
        delete (p.payload);
        close(sockfd);
    }

    if (!xdr_packet_data(&xdr_recv, &p))
    {
        printf("Error deserializing connection data\n");
        xdr_destroy(&xdr_recv);
        delete (p.payload);
        close(sockfd);
    }

    if (p.ack != seq)
    {
        printf("ACK not equal to SEQ\n");
        delete (p.payload);
        close(sockfd);
        return;
    }

    printf("am facut hanshake\n");

    // create_new_reliable_channel(sockfd, cli_addr, p.seq);
}