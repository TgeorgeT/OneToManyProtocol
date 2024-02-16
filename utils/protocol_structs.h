#include <deque>
#include <iostream>
#include <mutex>

#ifndef structs
#define structs
struct channel
{
    int32_t socket;
    int32_t channel_number;
    sockaddr_in cli_addr;
    std::deque<std::string> receive_que;
    std::mutex receive_que_lock;

    channel(int32_t socket, int32_t channel_number, const sockaddr_in &cli_addr)
        : socket(socket), channel_number(channel_number), cli_addr(cli_addr)
    {
    }
};

struct packet_data
{
    int32_t flags;
    int32_t ack;
    int32_t seq;
    int32_t payload_len;
    int32_t checksum;
};

struct packet
{
    packet_data data;
    // const char *payload;
};

#endif