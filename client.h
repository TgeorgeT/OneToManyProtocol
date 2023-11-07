#include <cstring>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

using std::cout;

struct sockaddr_in serv_addr;
struct sockaddr_in client_addr;

int32_t server_sockfd;
int32_t client_sockfd;

void init_client(uint16_t local_port)
{
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(local_port);
}

int connect_to_server(const char *server_ip, uint16_t server_port, uint16_t local_port)
{
    if ((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        cout << "Socket error";
        return -1;
    }

    serv_addr.sin_port = htons(server_port);

    cout << serv_addr.sin_port << "\n";

    char *message = new char[5];
    sprintf(message, "%d", local_port);

    ssize_t bytes_sent = sendto(server_sockfd, message, strlen(message), 0,
                                (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    delete message;

    if (bytes_sent <= 0)
    {
        sprintf("%s\n", "Error sending initilization packet");
        return -1;
    }

    return 0;
}
