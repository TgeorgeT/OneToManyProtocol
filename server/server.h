#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include <atomic>
#include <cstring>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <thread>
#include "../channel/channel.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../thread_pool/thread_pool.h"
#include <mutex>
#include "../utils/protocol_consts.h"
#include <sys/select.h>

const uint16_t initial_channel_number = 0;

using std::cout;
using std::thread;
using std::unique_lock;
using std::unordered_map;
using std::unordered_set;
using std::vector;

// HOW TO HANDLE CLOSE
// TO DO: ADD LOCK ON CREATING NEW CHANNEL
// TO DO: ADD LOCK ON FD_SET
// TO DO: DEAL WITH STD::MUTEX and copy constructor
// VALGRIND

class Server
{
    // TO DO: make private
public:
    std::atomic<uint16_t> channel_count;
    int32_t server_sockfd;
    thread listen_thread, receive_thread;
    unordered_map<uint32_t, channel *> channels;
    unordered_map<uint32_t, uint32_t> ip_to_channel;
    unordered_map<std::string, uint32_t> ipport_to_channel;
    thread_pool *senders, *receivers;
    unordered_set<int32_t> client_sockets;
    fd_set clients_sockets_set;
    timeval select_timeval;

    std::unordered_set<std::string> pending_connections;

    std::mutex universal_lock;
    std::condition_variable channels_cv;

    // std::mutex modify_sock_set;
    // std::mutex modify_channels_send_lock1;
    // std::mutex modify_channels_send_lockn;
    // std::mutex modify_channels_receive_lock1;
    // std::mutex modify_channels_receive_lockn;
    void listen_server();
    void create_new_unreliable_channel(int32_t client_socket, sockaddr_in cli_addr);
    void create_new_reliable_channel(int32_t client_socket, sockaddr_in cli_addr);
    void handle_new_connection(struct sockaddr_in client_addr);
    void handle_new_reliable_connection(struct sockaddr_in cli_addr);
    void handle_receive_packet();
    void receiver_function(const char *buf, sockaddr_in sender_addr);

public:
    Server();

    void init(uint16_t port);
    void listen();
    void send_to_all(const char *buf, size_t length);
    void send_to_channel(uint32_t channel_number, const char *buf, size_t length);
    void send_to_many_by_ipports(std::vector<std::string> ipports, std::vector<std::string> messages);
    std::vector<std::string> receive_from_channels_by_ipports(std::vector<std::string> ipports);
    void send_to_channel_by_ipport(std::string ipport, std::string messages);
    std::string receive_from_channel(uint32_t channel_number);
    std::string receive_from_channel_by_ipport(std::string ipport);
    void wait_for_ipport(std::string ipport);
    void wait_for_ipport_list(std::vector<std::string> ipport_list);
    void close_by_ippport(std::string ipport);
    void stop()
    {
        exit(0);
    }

    unordered_map<uint32_t, channel *> get_channels()
    {
        return this->channels;
    }
};

#endif