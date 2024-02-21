#include "server.h"
#include "rpc/xdr.h"
#include "../utils/xdr_serialization.h"

// handle flags
// add socket timeout

void Server::handle_new_reliable_connection(struct sockaddr_in cli_addr)
{

    printf("Handling new reliable connection\n");
    packet_data p;
    p.ack = 1;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed: %s\n", strerror(errno));
        return;
    }

    struct sockaddr_in new_client;
    new_client.sin_family = AF_INET;
    new_client.sin_addr.s_addr = INADDR_ANY;
    new_client.sin_port = 0;

    if (bind(sockfd, (struct sockaddr *)&new_client, sizeof(new_client)) < 0)
    {
        printf("Bind failed: %s\n", strerror(errno));
        return;
    }

    timeval timeout = {5, 0};
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        printf("setsockopt failed: %s\n", strerror(errno));
        return;
    }

    p.payload = new char[2];
    p.payload[0] = '1';
    p.payload[1] = '\0';

    p.payload_len = 1;
    p.seq = p.payload_len;

    XDR xdr_send;
    char send_buffer[50];
    xdrmem_create(&xdr_send, send_buffer, sizeof(send_buffer), XDR_ENCODE);

    if (!xdr_packet_data(&xdr_send, &p))
    {
        printf("Error serializing connection data\n");
    }
    else
    {
        int buf_len = xdr_getpos(&xdr_send);

        if (sendto(sockfd, send_buffer, buf_len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
        {
            printf("sendto failed: %s\n", strerror(errno));
        }
        else
        {
            XDR xdr_recv;
            char recv_buffer[50];
            xdrmem_create(&xdr_recv, recv_buffer, sizeof(recv_buffer), XDR_DECODE);

            printf("Waiting for data\n");

            if (recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, nullptr, nullptr) < 0)
            {
                printf("recvfrom failed: %s\n", strerror(errno));
            }
            else
            {
                printf("Received data\n");
                if (!xdr_packet_data(&xdr_recv, &p))
                {
                    printf("Error deserializing connection data\n");
                }
                else
                {
                    if (p.ack != p.seq)
                    {
                        printf("ACK not equal to SEQ\n");
                    }
                    else
                    {
                        printf("Handshake successful\n");

                        timeout = {0, 0};

                        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
                        {
                            printf("remove timeout failed: %s\n", strerror(errno));
                            return;
                        }
                        else
                        {
                            // create_new_reliable_channel(...);
                        }
                    }
                }
                xdr_destroy(&xdr_recv);
            }
        }
        xdr_destroy(&xdr_send);
    }
    delete[] p.payload;
    close(sockfd);
}