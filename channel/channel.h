#ifndef CHANNEL_H
#define CHANNEL_H

#include <deque>
#include <iostream>
#include <mutex>
#include <rpc/xdr.h>
#include "../utils/protocol_consts.h"
#include <queue>
#include <thread>
#include "../utils/protocol_structs.h"
#include <chrono>
#include <condition_variable>
#include <atomic>

class channel
{
public:
    int32_t socket;
    int32_t channel_number;
    struct sockaddr_in dest_addr;
    std::deque<std::string> receive_que;
    std::mutex receive_que_lock;

    channel(int32_t socket, int32_t channel_number, const sockaddr_in &dest_addr)
        : socket(socket), channel_number(channel_number), dest_addr(dest_addr) {}

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
    void receive_function();

public:
    void send(std::string);
    std::string receive();
    void serialize_and_send(packet_data *p);
    std::mutex send_que_lock;
    std::mutex acks_to_send_lock;
    std::mutex window_lock;

    std::condition_variable receive_que_cv;

    std::thread *send_thread;
    std::thread *receive_thread;

    std::deque<std::string> send_que;
    std::deque<std::uint32_t> received_ack;
    std::deque<std::uint32_t> acks_to_send;
    std::deque<std::pair<std::string, int>> window;
    std::deque<std::string> packekts_to_send;

    std::chrono::system_clock::time_point last_ack_time;

    char *send_buffer;
    char *receive_buffer;
    XDR send_xdr;
    XDR receive_xdr;

    std::atomic<int> last_received_seq;
    int last_seq;
    uint32_t max_window_size;

    reliable_channel(int32_t socket, int32_t channel_number, const sockaddr_in &dest_addr);
    void start_reliable_communication();

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
