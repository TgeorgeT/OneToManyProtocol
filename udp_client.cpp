#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <strings.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include "protocol_structs.h"
#include "utils.h"
#include "protocol_consts.h"
using std::cout;

int main()
{
    packet p = {{123, 1, 1, 1, 5}};

    cout << sizeof(p) << "\n";

    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(4001);

    char seralized [PACKET_SIZE];

    serialize_packet(&p, seralized, sizeof(p));

    cout << (int)seralized[0] << " " << (int)seralized[1] << "\n";

    int *nb;

    int n = sendto(sockfd, (void *)&seralized, 20, 0, (sockaddr *)&serv_addr, sizeof(sockaddr_in));
    cout << n << "\n";

}
