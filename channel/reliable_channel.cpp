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
    last_received_seq = 0;
    last_seq = 0;
    max_window_size = 128;
    last_ack_time = std::chrono::system_clock::now();
    active = 1;
}
void reliable_channel::start_reliable_communication()
{
    send_thread = new std::thread(&reliable_channel::send_function, this);
    send_thread->detach();
    receive_thread = new std::thread(&reliable_channel::receive_function, this);
    receive_thread->detach();
}

void reliable_channel::create_packet(std::string data, int seq, int ack, packet_data *p, char *payload)
{
    p->checksum = 0;
    p->flags = 0;
    p->seq = seq;
    if (p->seq == -1)
    {
        cout << "sending fin flag\n";
        p->seq = fin_ack_no;
        p->flags |= FIN_FLAG;
    }
    p->payload_len = data.size();
    payload[0] = '\0';
    p->payload = payload;
    memcpy(payload, data.c_str(), data.size());
    // cout << "created packet with payload len = " << p->payload_len << " and payload =" << p->payload << ";\n";
    p->ack = ack;
}

void reliable_channel::serialize_and_send(packet_data *p)
{
    xdrmem_create(&send_xdr, send_buffer, MAX_TRANSMITTED_LEN, XDR_ENCODE);
    if (!xdr_packet_data(&send_xdr, p))
    {
        // cout << "Error serializing" << p->payload_len << "\n";
        acks_to_send.push_front(p->ack);
    }
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getsockname(socket, (struct sockaddr *)&addr, &addr_len) < 0)
    {
        // cout << ("getsockname failed");
        return;
    }

    // std::cout << "sending" << "ack: " << p->ack << "seq: " << p->seq << " " << "payload: " << p->payload << "payload_len: " << p->payload_len << "\n";
    int bytes = sendto(socket, send_buffer, xdr_getpos(&send_xdr), 0, (struct sockaddr *)&dest_addr, sizeof(sockaddr_in));
}

void reliable_channel::send_function()
{
    packet_data p;
    char payload[MAX_TRANSMITTED_LEN - 24];
    int last_received_seq;

    for (;;)
    {
        std::unique_lock<std::mutex> lock1(acks_to_send_lock, std::defer_lock);
        std::unique_lock<std::mutex> lock2(window_lock, std::defer_lock);
        // sleep(1);
        std::lock(lock1, lock2);
        if (acks_to_send.empty())
        {
            last_received_seq = this->last_received_seq;
            std::chrono::duration<double> diff = std::chrono::system_clock::now() - this->last_ack_time;
            if (window.size() == max_window_size)
            {
                if (diff.count() > WINDOW_RESEND_TIME)
                {
                    this->last_ack_time = std::chrono::system_clock::now();
                    for (std::pair<std::string, int> message : window)
                    {
                        p.payload[0] = '\0';
                        create_packet(message.first, message.second, last_received_seq, &p, payload);
                        p.payload[0] = '\0';
                        // cout << "sending4\n";
                        serialize_and_send(&p);
                    }
                }
                continue;
            }
            {
                std::unique_lock<std::mutex> lock3(send_que_lock);
                if (send_que.empty() && window.empty() && sent_fin && got_fin)
                {
                    std::unique_lock<std::mutex> lock4(close_lock);
                    closed_send_thread = 1;
                    wait_for_close.notify_all();
                    return;
                }
                if (!send_que.empty())
                {
                    std::string message = send_que.front();
                    send_que.pop_front();
                    last_seq++;
                    // std::cout << "sending message " << message << "\n";
                    payload[0] = '\0';
                    create_packet(message, last_seq, last_received_seq, &p, payload);
                    // cout << "sending3\n";
                    serialize_and_send(&p);
                    // cout << "pushing to window: " << message << " " << last_seq << "\n";
                    window.push_back(std::make_pair(message, last_seq));
                }
                else if (active == 0 && sent_fin == 0)
                {
                    // cout << "Am intrat aici\n";
                    sent_fin = 1;
                    fin_ack_no = ++last_seq;
                    window.push_back(std::make_pair("", -1));
                    payload[0] = '\0';
                    create_packet("", -1, last_received_seq, &p, payload);
                    // cout << "sending5\n";
                    serialize_and_send(&p);

                    continue;
                }
            }
            diff = std::chrono::system_clock::now() - this->last_ack_time;
            if (diff.count() > WINDOW_RESEND_TIME && !window.empty())
            {
                // cout << "WINDOW SIZE1 " << window.size() << "\n";
                this->last_ack_time = std::chrono::system_clock::now();
                for (std::pair<std::string, int> message : window)
                {
                    create_packet(message.first, message.second, last_received_seq, &p, payload);
                    // cout << "sending6: " << message.first << "\n";
                    serialize_and_send(&p);
                }
            }
            continue;
        }
        // cout << " ADASASDADSDADADSDA\n";
        int ack = acks_to_send.front();
        create_packet("", last_seq, ack, &p, payload);
        acks_to_send.pop_front();
        // cout << "sending2\n";
        serialize_and_send(&p);
    }
}

void reliable_channel::receive_function()
{
    packet_data p;
    p.payload = new char[MAX_TRANSMITTED_LEN - 24];
    int n;
    for (;;)
    {
        if (closed_send_thread)
        {
            std::unique_lock<std::mutex> lock(close_lock);
            closed_receive_thread = 1;
            wait_for_close.notify_all();
            active = 0;
            return;
        }
        // sleep(1);
        memset(receive_buffer, 0, MAX_TRANSMITTED_LEN);
        n = recvfrom(socket, receive_buffer, MAX_TRANSMITTED_LEN, 0, nullptr, nullptr);

        xdrmem_create(&receive_xdr, receive_buffer, MAX_TRANSMITTED_LEN, XDR_DECODE);
        if (n < 0)
        {
            if (active == 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
            {
                return;
            }
            std::cout << "Error receiving from socket\n";
            continue;
        }
        if (!xdr_packet_data(&receive_xdr, &p))
        {
            std::cout << "Error deserializing data\n";
            continue;
        }
        p.payload[p.payload_len] = '\0';

        // std::cout
        // << "received" << "ack: " << p.ack << "seq: " << p.seq << " " << "payload: " << p.payload << "payload_len: " << p.payload_len << "\n";
        if (p.flags & FIN_FLAG)
        {
            cout << "Got fin flag\n";
            got_fin = 1;
        }
        {
            std::unique_lock<std::mutex> lock(window_lock);
            while (!window.empty() && p.ack >= window.front().second)
            {
                window.pop_front();
                this->last_ack_time = std::chrono::system_clock::now();
            }
        }
        {
            std::unique_lock<std::mutex> lock1(acks_to_send_lock, std::defer_lock);
            std::unique_lock<std::mutex> lock2(receive_que_lock, std::defer_lock);
            std::lock(lock1, lock2);
            if (p.seq == last_received_seq + 1)
            {
                last_received_seq = p.seq;
                // cout << "seq received" << p.seq << "\n";
                // cout << "got message: " << p.payload << "\n";
                receive_que.push_back(std::string(p.payload, p.payload_len));
                // cout << "receive que back " << receive_que.back() << "\n";
                acks_to_send.push_back(p.seq);
                receive_que_cv.notify_one();
            }
            else if (p.seq <= last_received_seq && (p.payload_len != 0 || p.flags & FIN_FLAG))
            {
                acks_to_send.push_back(p.seq);
            }
        }
    }
}

void reliable_channel::send(std::string message)
{
    if (!active)
    {
        std::cout << "Inactive channel\n";
    }
    std::unique_lock<std::mutex> lock(send_que_lock);
    send_que.push_back(message);
}

void reliable_channel::close()
{
    // cout << "Closing connection\n";
    active = 0;
    std::unique_lock<std::mutex> lock(close_lock);
    wait_for_close.wait(lock, [this]
                        { return closed_receive_thread && closed_send_thread; });
    // cout << "Conenction closed succesfuly\n";
}

std::string reliable_channel::receive()
{
    // std::cout << "in receive\n";
    std::unique_lock<std::mutex> lock(receive_que_lock);
    receive_que_cv.wait(lock, [this]
                        { return !receive_que.empty(); });
    // cout << "POPPING QUE\n";
    std::string message = receive_que.front();
    receive_que.pop_front();
    return message;
}