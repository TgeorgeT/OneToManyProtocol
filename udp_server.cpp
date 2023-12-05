#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <strings.h>
#include <iostream>
#include "protocol_structs.h"
#include <string.h>
using std::cout;

int main()
{
    int sockfd;
    char buffer[1024];
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(4001);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    int n;
    socklen_t len;
    while (1)
    {
        n = recvfrom(sockfd, (char *)buffer, 1024, 0, (struct sockaddr *)&cliaddr,
                     &len);
        buffer[n] = '\0';

        packet_data *pd;

        cout << n << "\n";
        memcpy(pd, buffer, sizeof(packet_data));

        cout << pd->seq << "\n";
        cout << pd->ack << "\n";
        cout << sizeof(packet_data) << "\n";
        // cout << pd->flags << "\n";

        // nr_packet *np = (nr_packet *)&buffer;
        // cout << np->number << "\n";
    }
}