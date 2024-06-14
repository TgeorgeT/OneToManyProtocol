#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int sockfd;

void handle_sigint(int sig)
{
    printf("\nCaught signal %d, closing socket and exiting...\n", sig);
    close(sockfd);
    exit(0);
}

int main()
{
    char buffer[BUFFER_SIZE+1];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Register signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Filling server information
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP server is running on port %d. Press Ctrl+C to stop.\n", SERVER_PORT);

    while (1)
    {
        // Receive data from client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        buffer[n] = '\0'; // Null-terminate the received data
        printf("Received from client: %s\n", buffer);

        // Send the same data back to the client
        sendto(sockfd, buffer, n, 0, (struct sockaddr *)&client_addr, addr_len);
    }
}

