#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <strings.h>
#include <iostream>
#include "protocol_structs.h"
#include "utils.h"
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

        int *nb;

        cout << (int)buffer[0] << " " << (int)buffer[1] << "\n";
        cout << "n =" << n << "\n";

        // memcpy(nb, buffer, 4);

        cout << htonl(*nb) << "\n";

        cout << buffer << "\n";

        // desirialize_packet(buffer, n);
    }
}