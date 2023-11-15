#include <cstring>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <thread>
#include "protocol_structs.h"
#include <vector>

using std::cout;
using std::thread;
using std::vector;

class Server
{
private:
    static int32_t channel_count = 0;

    int32_t server_sockfd;
    thread listen_thread;
    vector<channel> channels;

    void listen_server();

    void create_new_channel(int32_t client_socket, in_addr client_ip);

    void handle_new_connection(const char *buf, struct sockaddr_in client_addr);

public:
    Server(){};

    void init_server(uint16_t port);

    void listen();
};

void Server::listen_server()
{
    int n;
    struct sockaddr_in cli_addr;
    char *buffer = new char[10];
    socklen_t len;
    while (1)
    {
        cout << "e bine\n";
        bzero(&cli_addr, sizeof(cli_addr));
        len = sizeof(struct sockaddr_in);
        n = recvfrom(server_sockfd, buffer, 10, 0, (struct sockaddr *)&cli_addr,
                     &len);
        buffer[n] = '\0';
        if (n > 0)
        {
            this->handle_new_connection(buffer, cli_addr);
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

void Server::handle_new_connection(const char *client_message, struct sockaddr_in cli_addr)
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

    char *buf = new char[6];
    sprintf(buf, "%d", ntohs(new_client->sin_port));
    printf("buf = %s\n", buf);
    int n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));

    create_new_channel(sockfd, cli_addr.sin_addr);
}