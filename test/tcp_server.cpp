#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>

int main(int argc, char *argv[])
{
    int num_iterations = std::stoi(argv[1]);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1000000] = {0};
    int port = 8080;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation error\n";
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        std::cerr << "Setsockopt error\n";
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind error\n";
        return 1;
    }
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "Listen error\n";
        return 1;
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        std::cerr << "Accept error\n";
        return 1;
    }

    auto time1 = std::chrono::system_clock::now();
    int total_bytes_received = 0;
    int expected_bytes = num_iterations * 1000;

    while (total_bytes_received < expected_bytes)
    {
        ssize_t bytes_received = recv(new_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            std::cerr << "Failed to receive data or connection closed\n";
            break;
        }
        total_bytes_received += bytes_received;
        std::cout << total_bytes_received << "\n";
    }

    std::chrono::duration<double> diff = std::chrono::system_clock::now() - time1;
    std::cout << "Time: " << diff.count() << " seconds\n";
    std::cout << "Bytes received: " << total_bytes_received << "\n";

    close(new_socket);
    close(server_fd);
    return 0;
}
