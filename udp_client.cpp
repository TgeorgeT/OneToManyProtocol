#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <strings.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include "protocol_structs.h"
using std::cout;

int main()
{
    packet p = {{123, 1, 1}, nullptr};

    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(4001);

    cout << sizeof(p);

    sendto(sockfd, (void *)&p, sizeof(p), 0, (sockaddr *)&serv_addr, sizeof(sockaddr_in));
}