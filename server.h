#include <cstring>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <thread>
#include "protocol_structs.h"
#include <unordered_map>
#include <vector>
#include "thread_pool.h"

const uint16_t initial_channel_number = 0;

using std::cout;
using std::thread;
using std::unordered_map;
using std::vector;

// HOW TO HANDLE CLOSE

class Server
{
    // TO DO: make private
public:
    uint16_t channel_count;
    int32_t server_sockfd;
    thread listen_thread;
    unordered_map<uint32_t, channel> channels;
    unordered_map<uint32_t, uint32_t> ip_to_channel;
    thread_pool *senders;

    void
    listen_server();

    void create_new_channel(int32_t client_socket, sockaddr_in cli_addr);

    void handle_new_connection(const char *buf, size_t buf_size, struct sockaddr_in client_addr);

public:
    Server();

    void init_server(uint16_t port);

    void listen();

    void send_to_all(const char *buf, size_t *length);
    void send_to_channel(const char *buf, size_t *length, uint32_t channel_number);
};

Server::Server() : channel_count(initial_channel_number)
{
    this->senders = new thread_pool(10);
};

void Server::send_to_all(const char *buf, size_t *length)
{

    for (auto it = channels.begin(); it != channels.end(); ++it)
    {
        senders->enqueue([this, it, buf, length]
                         { sendto(it->second.socket, buf, *length, 0, (const sockaddr *)&((it->second).cli_addr), sizeof((it->second).cli_addr)); });
    }
}

void Server::send_to_channel(const char *buf, size_t *length, uint32_t channel_number)
{
    auto it = channels.find(channel_number);
    if (it == channels.end())
    {
        printf("Canalul nu exista\n");
        return;
    }

    senders->enqueue([this, it, buf, length]
                     { sendto(it->second.socket, buf, *length, 0, (const sockaddr *)&((it->second).cli_addr), sizeof((it->second).cli_addr)); });
}

void Server::create_new_channel(int32_t client_socket, sockaddr_in cli_addr)
{
    channel new_channel = {
        client_socket,
        ++channel_count,
        cli_addr};
    ip_to_channel[cli_addr.sin_addr.s_addr] = new_channel.channel_number;
    channels[new_channel.channel_number] = new_channel;
}

void Server::listen_server()
{
    int n;
    struct sockaddr_in cli_addr;
    char *buffer = new char[10];
    socklen_t len;
    for (;;)
    {
        cout << "e bine\n";
        bzero(&cli_addr, sizeof(cli_addr));
        len = sizeof(struct sockaddr_in);
        n = recvfrom(server_sockfd, buffer, 10, 0, (struct sockaddr *)&cli_addr,
                     &len);
        buffer[n] = '\0';
        if (n > 0)
        {
            this->handle_new_connection(buffer, strlen(buffer), cli_addr);
            cout << channels.size() << "\n";
        }
    }
}

void Server::init_server(uint16_t port)
{
    this->server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in serv_addr;

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    bind(this->server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

void Server::listen()
{
    this->listen_thread = thread(&Server::listen_server, this);
    this->listen_thread.detach();
}

void Server::handle_new_connection(const char *client_message, size_t message_len, struct sockaddr_in cli_addr)
{

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

    socklen_t len;

    if (getsockname(sockfd, (struct sockaddr *)new_client, &len) == -1)
    {
        printf("Getsockname error:%s\n", strerror(errno));
    }

    cout << "client message= " << client_message << "\n";

    char *buf = new char[6];
    sprintf(buf, "%d", ntohs(cli_addr.sin_port));
    printf("buf = %s\n", buf);
    printf("strlen buf = %d\n", strlen(buf));
    int n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
    cout << "n = " << n << "\n";
    create_new_channel(sockfd, cli_addr);
}