#include "server.h"
#include "rpc/xdr.h"
#include "../utils/xdr_serialization.h"

// handle flagclients
// add socket timeout
void Server::handle_new_reliable_connection(struct sockaddr_in client_addr)
{
    char ip_str[INET_ADDRSTRLEN];
    // sleep(2);
    if (inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str)) != NULL)
    {
        std::string ip_port_str = std::string(ip_str) + ":" + std::to_string(ntohs(client_addr.sin_port));
        {
            std::unique_lock<std::mutex> lock(universal_lock);
            if (ipport_to_channel.find(ip_port_str) != ipport_to_channel.end())
            {
                return;
            }
        }

        pending_connections.insert(ip_port_str);
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            std::cout << "socket creation failed\n";
            return;
        }
        struct sockaddr_in new_client;
        new_client.sin_family = AF_INET;
        new_client.sin_addr.s_addr = INADDR_ANY;
        new_client.sin_port = 0;
        if (bind(sockfd, (struct sockaddr *)&new_client, sizeof(new_client)) < 0)
        {
            std::cout << "Bind failed:" << strerror(errno) << "\n";
            return;
        }

        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
        {
            std::cout << "Error getting socket name" << std::endl;
            return;
        }

        reliable_channel *new_channel = new reliable_channel(
            sockfd,
            ++channel_count,
            client_addr);
        new_channel->last_received_seq = 1;
        // cout << "created reliable channel object\n";

        new_channel->start_reliable_communication();

        new_channel->send(std::to_string(ntohs(sin.sin_port)));

        // cout << "sent string\n";
        std::string s = new_channel->receive();
        // cout << "received string\n";

        // std::cout << s << "\n";

        if (s == "ok")
        {
            std::unique_lock<std::mutex> lock(universal_lock);
            ipport_to_channel[ip_port_str] = new_channel->channel_number;
            channels[new_channel->channel_number] = new_channel;
            std::cout << "new channel: " << ip_port_str << "\n";
            std::cout.flush();
            channels_cv.notify_all();
        }
    }
    else
    {
        std::cout << "inet_ntop failed" << "\n";
    }
}

// void Server::handle_new_reliable_connection(struct sockaddr_in cli_addr)
// {
//     packet_data p;
//     bzero(&p, sizeof(p));
//     p.ack = 1;

//     int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockfd < 0)
//     {
//         printf("Socket creation failed: %s\n", strerror(errno));
//         return;
//     }

//     struct sockaddr_in new_client;
//     new_client.sin_family = AF_INET;
//     new_client.sin_addr.s_addr = INADDR_ANY;
//     new_client.sin_port = 0;

//     if (bind(sockfd, (struct sockaddr *)&new_client, sizeof(new_client)) < 0)
//     {
//         printf("Bind failed: %s\n", strerror(errno));
//         return;
//     }

//     char ipStr[INET_ADDRSTRLEN];

//     const char *result = inet_ntop(AF_INET, &cli_addr.sin_addr, ipStr, sizeof(ipStr));

//     timeval timeout = {10, 0};
//     if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
//     {
//         printf("setsockopt failed: %s\n", strerror(errno));
//         return;
//     }

//     p.payload = new char[2];
//     p.payload[0] = '1';
//     tr
//         p.payload[1] = '\0';

//     p.payload_len = 1;
//     p.seq = p.payload_len;

//     XDR xdr_send;
//     char send_buffer[50];
//     memset(send_buffer, 0, sizeof(send_buffer));
//     xdrmem_create(&xdr_send, send_buffer, sizeof(send_buffer), XDR_ENCODE);

//     if (!xdr_packet_data(&xdr_send, &p))
//     {
//         printf("Error serializing connection data\n");
//     }
//     else
//     {
//         int buf_len = xdr_getpos(&xdr_send);

//         int b;

//         if ((b = sendto(sockfd, send_buffer, buf_len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr))) < 0)
//         {
//             printf("sendto failed: %s\n", strerror(errno));
//         }
//         else
//         {
//             printf("Sent %d bytes\n", b);
//             XDR xdr_recv;
//             char recv_buffer[50];
//             memset(recv_buffer, 0, sizeof(recv_buffer));
//             xdrmem_create(&xdr_recv, recv_buffer, sizeof(recv_buffer), XDR_DECODE);

//             printf("Waiting for data\n");

//             if (recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, nullptr, nullptr) < 0)
//             {
//                 printf("recvfrom failed: %s\n", strerror(errno));
//             }
//             else
//             {
//                 if (!xdr_packet_data(&xdr_recv, &p))
//                 {
//                     printf("Error deserializing connection data\n");
//                 }
//                 else
//                 {
//                     if (p.ack != p.seq)
//                     {
//                         printf("ACK not equal to SEQ\n");
//                     }
//                     else
//                     {
//                         printf("Handshake successful\n");

//                         timeout = {SOCK_TIMEOUT, 0};

//                         if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
//                         {
//                             printf("remove timeout failed: %s\n", strerror(errno));
//                             return;
//                         }
//                         else
//                         {
//                             create_new_reliable_channel(sockfd, cli_addr);
//                         }
//                     }
//                 }
//                 xdr_destroy(&xdr_recv);
//             }
//         }
//         xdr_destroy(&xdr_send);
//     }
//     delete[] p.payload;
// }

// TO DO: ADD LOCKS
void Server::create_new_reliable_channel(int32_t client_socket, sockaddr_in cli_addr)
{
    std::unique_lock<std::mutex> lock(universal_lock);

    printf("%d\n", client_socket);
    reliable_channel *new_channel = new reliable_channel(
        client_socket,
        ++channel_count,
        cli_addr);
    printf("Channel number: %d\n", new_channel->channel_number);
    ip_to_channel[cli_addr.sin_addr.s_addr] = new_channel->channel_number;
    // FD_SET(client_socket, &(this->clients_sockets_set));
    // client_sockets.insert(client_socket);
    channels[new_channel->channel_number] = new_channel;
    new_channel->start_reliable_communication();
    cout << "Started reliable communication\n";
    // new_channel->receive_thread->join();
}
