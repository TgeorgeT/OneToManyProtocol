#ifndef STRUCTS_H
#define STRUCTS_H

#include <deque>
#include <iostream>
#include <mutex>

struct packet_data
{
    int32_t flags;
    int32_t ack;
    int32_t seq;
    int32_t checksum;
    int32_t payload_len;
    char *payload;
};

#endif