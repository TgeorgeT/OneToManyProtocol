#include "client.h"
#include <iostream>
#include <arpa/inet.h>

void Client::reliable_connect_to_one(const char *server_ip, uint16_t server_port)
{
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(server_port);

    printf("Creating reliable channel\n");
    channel = new reliable_channel(
        server_sockfd,
        1, serv_addr);

    printf("Creating reliable channel\n");
    channel->start_reliable_communication();


    channel->send("1");
    std::string port = channel->receive();

    printf("Creating reliable channel\n");

    channel->dest_addr.sin_port = htons(stoi(port));
    channel->send("ok");

    std::cout << "am terminat\n";
}

// void client::reliable_connect_to_server(const char *server_ip, uint16_t server_port)
// {
//     char send_buffer[MAX_TRANSMITTED_LEN];
//     char recv_buffer[MAX_TRANSMITTED_LEN];
//     XDR xdr_send, xdr_recv;
//     xdrmem_create(&xdr_send, send_buffer, sizeof(send_buffer), XDR_ENCODE);
//     xdrmem_create(&xdr_recv, recv_buffer, sizeof(recv_buffer), XDR_DECODE);

//     serv_addr.sin_family = AF_INET;
//     inet_pton(AF_INET, server_ip, &(serv_addr.sin_addr));
//     serv_addr.sin_port = htons(server_port);

//     char message[2] = {'1', '\0'};

//     for (int i = 0; i < MAX_CONNECTION_RETRIES; i++)
//     {
//         ssize_t bytes_sent = sendto(server_sockfd, message, strlen(message), 0,
//                                     (struct sockaddr *)&serv_addr, sizeof(serv_addr));

//         std::cout << "bytes_sent = " << bytes_sent << "\n";
//         if (bytes_sent <= 0)
//         {
//             printf("Error sending data\n");
//             continue;
//         }
//         socklen_t len = sizeof(struct sockaddr_in);

//         int bytes_received = recvfrom(server_sockfd, recv_buffer, MAX_TRANSMITTED_LEN, 0, (sockaddr *)&serv_addr, &len);

//         if (bytes_received < 0)
//         {
//             printf("recvfrom failed");
//             continue;
//         }

//         std::cout << "bytes_received = " << bytes_received << "\n";

//         if (bytes_received > 0)
//             break;
//     }
//     if (i == MAX_CONNECTION_RETRIES)
//     {

//         xdr_destroy(&xdr_send);
//         xdr_destroy(&xdr_recv);
//         return;
//     }

//     // ssize_t bytes_sent = sendto(server_sockfd, message, strlen(message), 0,
//     //                             (struct sockaddr *)&serv_addr, sizeof(serv_addr));

//     // std::cout << "bytes_sent = " << bytes_sent << "\n";
//     // if (bytes_sent <= 0)
//     // {
//     //     xdr_destroy(&xdr_recv);
//     //     xdr_destroy(&xdr_send);
//     //     printf("Error sending data\n");
//     //     return;
//     // }
//     // socklen_t len = sizeof(struct sockaddr_in);

//     // int bytes_received = recvfrom(server_sockfd, recv_buffer, MAX_TRANSMITTED_LEN, 0, (sockaddr *)&serv_addr, &len);

//     // std::cout << "bytes_received = " << bytes_received << "\n";

//     packet_data data;
//     bzero(&data, sizeof(data));
//     data.payload = new char[2];

//     if (!xdr_packet_data(&xdr_recv, &data))
//     {
//         printf("Error deserializing data\n");
//         delete[] data.payload;
//         xdr_destroy(&xdr_recv);
//         xdr_destroy(&xdr_send);
//         return;
//     }
//     xor_swap(data.ack, data.seq);
//     if (!xdr_packet_data(&xdr_send, &data))
//     {
//         printf("Error serializing data\n");
//         delete[] data.payload;
//         xdr_destroy(&xdr_recv);
//         xdr_destroy(&xdr_send);
//         return;
//     }
//     int bytes = sendto(server_sockfd, send_buffer, xdr_getpos(&xdr_send), 0, (struct sockaddr *)&serv_addr, sizeof(sockaddr_in));
//     std::cout << ntohs(serv_addr.sin_port) << "\n";

//     if (bytes < 0)
//     {
//         printf("sendto failed");
//         delete[] data.payload;
//         xdr_destroy(&xdr_recv);
//         xdr_destroy(&xdr_send);
//         return;
//     }
//     printf("bytes: %d\n", bytes);
//     delete[] data.payload;
//     xdr_destroy(&xdr_recv);
//     xdr_destroy(&xdr_send);
//     create_reliable_channel(server_sockfd, serv_addr);
// }
void Client::create_reliable_channel(int sockfd, struct sockaddr_in addr)
{
    printf("Creating reliable channel\n");
    channel = new reliable_channel(
        sockfd,
        1, addr);
    channel->start_reliable_communication();
}
