#include "channel.h"
#include <iostream>
#include <unistd.h>
#include "../utils/xdr_serialization.h"
#include "../utils/protocol_structs.h"
#include "../utils/helper_functions.h"
#include "../utils/protocol_consts.h"
#include <string.h>
#include <arpa/inet.h>

using std::cout;

reliable_channel::reliable_channel(int32_t socket, int32_t channel_number, const sockaddr_in &dest_addr)
    : channel(socket, channel_number, dest_addr)
{
    send_buffer = new char[MAX_TRANSMITTED_LEN];
    receive_buffer = new char[MAX_TRANSMITTED_LEN];

    last_received_seq = 1;
    last_seq = 1;

    max_window_size = 128;

    last_ack_time = std::chrono::system_clock::now();
}
void reliable_channel::start_reliable_communication()
{
    send_thread = new std::thread(&reliable_channel::send_function, this);
    send_thread->detach();
    receive_thread = new std::thread(&reliable_channel::receive_function, this);
    receive_thread->detach();
}

void create_packet(std::string data, int seq, int ack, packet_data *p, char *payload)
{
    p->checksum = 0;
    p->flags = 0;
    p->payload_len = data.size();
    p->payload = payload;
    memcpy(payload, data.c_str(), data.size());
    p->seq = seq;
    p->ack = ack;
}

void reliable_channel::serialize_and_send(packet_data *p)
{
    xdrmem_create(&send_xdr, send_buffer, MAX_TRANSMITTED_LEN, XDR_ENCODE);
    if (!xdr_packet_data(&send_xdr, p))
    {
        std::cout
            << "Error serializing data\n";
        acks_to_send.push_front(p->ack);
    }

    // char ipStr[INET_ADDRSTRLEN];

    // const char *result = inet_ntop(AF_INET, &dest_addr.sin_addr, ipStr, sizeof(ipStr));

    // if (result == NULL)
    // {
    //     perror("inet_ntop failed");
    // }
    // else
    // {
    //     printf("IP Address: %s\n", ipStr);
    // }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getsockname(socket, (struct sockaddr *)&addr, &addr_len) < 0)
    {
        perror("getsockname failed");
        exit(EXIT_FAILURE);
    }

    // std::cout << ntohs(dest_addr.sin_port) << "\n";
    int bytes = sendto(socket, send_buffer, xdr_getpos(&send_xdr), 0, (struct sockaddr *)&dest_addr, sizeof(sockaddr_in));
}

void reliable_channel::send_function()
{
    cout << "send function\n";
    packet_data p;
    char payload[MAX_TRANSMITTED_LEN - 24];
    int last_received_seq;

    for (;;)
    {
        // sleep(1);
        std::unique_lock<std::mutex>
            lock1(acks_to_send_lock, std::defer_lock);
        std::unique_lock<std::mutex> lock2(window_lock, std::defer_lock);
        std::lock(lock1, lock2);
        if (acks_to_send.empty())
        {
            last_received_seq = this->last_received_seq;
            std::chrono::duration<double> diff = std::chrono::system_clock::now() - this->last_ack_time;
            if (window.size() == max_window_size && diff.count() > 0.05)
            {
                // cout << "window full\n"
                //      << window.front().second << "\n";
                for (std::pair<std::string, int> message : window)
                {
                    create_packet(message.first, message.second, last_received_seq, &p, payload);
                    serialize_and_send(&p);
                }
                continue;
            }
            {
                std::unique_lock<std::mutex> lock3(send_que_lock);
                // cout << "send_que size: " << send_que.size() << "\n";
                if (!send_que.empty())
                {
                    std::string message = send_que.front();
                    send_que.pop_front();
                    last_seq++;
                    create_packet(message, last_seq, last_received_seq, &p, payload);
                    serialize_and_send(&p);
                    window.push_back(std::make_pair(message, last_seq));
                }
            }
            if (diff.count() > 0.05 && !window.empty())
            {
                for (std::pair<std::string, int> message : window)
                {
                    create_packet(message.first, message.second, last_received_seq, &p, payload);
                    serialize_and_send(&p);
                }
            }

            continue;
        }
        // cout << "sending ack : " << acks_to_send.front() << "\n";
        int ack = acks_to_send.front();
        acks_to_send.pop_front();

        create_packet("", last_seq, ack, &p, payload);
        serialize_and_send(&p);
    }
}

void reliable_channel::receive_function()
{
    packet_data p;

    p.payload = new char[MAX_TRANSMITTED_LEN - 24];
    int n;
    cout << "receive function\n";
    for (;;)
    {
        // sleep(1);
        memset(receive_buffer, 0, MAX_TRANSMITTED_LEN);
        n = recvfrom(socket, receive_buffer, MAX_TRANSMITTED_LEN, 0, nullptr, nullptr);

        xdrmem_create(&receive_xdr, receive_buffer, MAX_TRANSMITTED_LEN, XDR_DECODE);
        if (n < 0)
        {
            std::cout << "Error receiving from socket\n";
            continue;
        }
        if (!xdr_packet_data(&receive_xdr, &p))
        {
            std::cout << "Error deserializing data\n";
            continue;
        }
        // cout << "received seq " << p.seq;
        // cout << "am primit ack: " << p.ack << "\n";

        {

            std::unique_lock<std::mutex> lock(window_lock);
            while (!window.empty() && p.ack >= window.front().second)
            {
                // cout << "am scos din window\n"
                //      << window.front().second << "\n";
                window.pop_front();
                this->last_ack_time = std::chrono::system_clock::now();
            }
        }

        {
            std::unique_lock<std::mutex> lock1(acks_to_send_lock, std::defer_lock);
            std::unique_lock<std::mutex> lock2(receive_que_lock, std::defer_lock);
            std::lock(lock1, lock2);
            // std::cout << "am primit seq: " << p.seq << "\n";
            if (p.seq == last_received_seq + 1)
            {
                // cout << "received seq " << p.seq << " last_received_seq " << last_received_seq << "\n";
                last_received_seq = p.seq;

                // cout << "\nreceived:\n-----------------------------------------\n"
                //      << std::string(p.payload, p.payload_len) << "\n------------------------------------------\n";

                receive_que.push_back(std::string(p.payload, p.payload_len));
                acks_to_send.push_front(p.seq);
                receive_que_cv.notify_one();
            }
            else if (p.seq <= last_received_seq && p.payload_len != 0)
            {
                acks_to_send.push_front(p.seq);
            }
        }
    }
}

void reliable_channel::send(std::string message)
{
    std::unique_lock<std::mutex> lock(send_que_lock);

    send_que.push_back(message);
}

std::string reliable_channel::receive()
{
    std::unique_lock<std::mutex> lock1(receive_que_lock);
    receive_que_cv.wait(lock1, [this]
                        { return !receive_que.empty(); });
    // printf("received----------------------------------------------\n");
    std::string message = receive_que.front();
    // cout << "\n-----------------------------------------\n"
    //      << message << "\n------------------------------------------\n";
    receive_que.pop_front();
    return message;
}