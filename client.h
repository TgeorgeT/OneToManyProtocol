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

int init_client(uint16_t local_port)
{
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(local_port);

    if ((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Error on binding client socket: %s\n", strerror(errno));
        return -1;
    }

    bind(client_sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
    return 0;
}

int connect_to_server(const char *server_ip, uint16_t server_port)
{

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(server_port);

    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(server_port);
    cout << serv_addr.sin_port << "\n";

    char *message = new char[10];
    sprintf(message, "%d", ntohs(client_addr.sin_port));

    ssize_t bytes_sent = sendto(client_sockfd, message, strlen(message), 0,
                                (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    delete message;
    printf("%d\n", bytes_sent);
    if (bytes_sent <= 0)
    {
        throw std::runtime_error("error_initialization");
    }

    char *buf = new char[10];

    socklen_t len = sizeof(struct sockaddr_in);

    ssize_t bytes_received = recvfrom(client_sockfd, buf, 10, 0,
                                      (struct sockaddr *)&serv_addr, &len);
    cout << "port ="
         << ntohs(serv_addr.sin_port) << "\n";
    printf("bytes: %d\n", bytes_received);
    buf[bytes_received] = '\0';
    return 0;
}

int receive_from_server(char *buf, const size_t *len)
{
    int n = recvfrom(client_sockfd, buf, 100, 0, NULL, NULL);
    return n;
}

void send_to_server(const char *buf, size_t len)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);
    cout << buf << "\n";
    int n = sendto(client_sockfd, buf, len, 0, (struct sockaddr *)&serv_addr, addr_len);
}

void unreliable_announce_end()
{
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int n = sendto(client_sockfd, "unreliable_end", 14, 0, (struct sockaddr *)&serv_addr, addr_len);
}
