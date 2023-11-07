#include <cstring>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

using std::cout;

struct sockaddr_in serv_addr;

int32_t server_sockfd;

void init_server(uint16_t port)
{
    server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

void handle_new_connection(struct sockaddr_in cli_addr)
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
    int n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));

    cout << n << "\n";
}

void listen_server()
{
    int n;
    struct sockaddr_in cli_addr;
    char *buffer = new char[1024];
    socklen_t len;
    while (1)
    {
        bzero(&cli_addr, sizeof(cli_addr));
        n = recvfrom(server_sockfd, (char *)buffer, 1024, 0, (struct sockaddr *)&cli_addr,
                     &len);
        handle_new_connection(cli_addr);
    }
}
