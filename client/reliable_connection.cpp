#include "client.h"

void client::create_reliable_channel(int sockfd, struct sockaddr_in addr)
{
    printf("Creating reliable channel\n");
    channel = new reliable_channel(
        sockfd,
        1, addr);
    channel->start_reliable_communication();
}