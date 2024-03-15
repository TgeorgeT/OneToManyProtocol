#ifndef CHANNEL_H
#define CHANNEL_H

#include <deque>
#include <iostream>
#include <mutex>
#include <rpc/xdr.h>
#include "../utils/protocol_consts.h"
#include <queue>
#include <thread>

class channel
{
public:
    int32_t socket;
    int32_t channel_number;
    struct sockaddr_in cli_addr;
    std::deque<std::string> receive_que;
    std::mutex receive_que_lock;

    channel(int32_t socket, int32_t channel_number, const sockaddr_in &cli_addr)
        : socket(socket), channel_number(channel_number), cli_addr(cli_addr) {}

    virtual bool get_type() const = 0;

    virtual ~channel() = default;

    channel(const channel &) = delete;
    channel &operator=(const channel &) = delete;
    channel(channel &&) = delete;
    channel &operator=(channel &&) = delete;
};

class unreliable_channel : public channel
{
public:
    using channel::channel;
    bool get_type() const override
    {
        return false;
    }
};

class reliable_channel : public channel
{

private:
    void send_function();

public:
    std::mutex send_que_lock;

    std::thread *send_thread;
    std::thread *receive_thread;

    std::deque<std::string> send_que;

    char *send_buffer;
    char *receive_buffer;
    XDR send_xdr;
    XDR receive_xdr;

    uint32_t last_ack;
    uint32_t last_seq;

    reliable_channel(int32_t socket, int32_t channel_number, const sockaddr_in &cli_addr)
        : channel(socket, channel_number, cli_addr)
    {
        send_buffer = new char[MAX_TRANSMITTED_LEN];
        receive_buffer = new char[MAX_TRANSMITTED_LEN];
        xdrmem_create(&send_xdr, send_buffer, MAX_TRANSMITTED_LEN, XDR_ENCODE);
        xdrmem_create(&receive_xdr, receive_buffer, MAX_TRANSMITTED_LEN, XDR_DECODE);

        send_thread = new std::thread(&reliable_channel::send_function, this);
        // receive_thread = new std::thread(&reliable_channel::receive_function, this);
    }

    ~reliable_channel() override
    {
        delete[] send_buffer;
        delete[] receive_buffer;
        xdr_destroy(&send_xdr);
        xdr_destroy(&receive_xdr);
    }

    bool get_type() const override
    {
        return true;
    }
};

#endif
