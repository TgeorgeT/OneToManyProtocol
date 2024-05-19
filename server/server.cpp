#include "server.h"

using std::cout;
using std::thread;
using std::unique_lock;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void Server::close_by_ippport(std::string ipport)
{
    unique_lock<mutex> lock(universal_lock);
    if (ipport_to_channel.find(ipport) == ipport_to_channel.end())
    {
        throw std::runtime_error("channel_not_exist");
    }
    int channel_number = ipport_to_channel[ipport];
    static_cast<reliable_channel *>(channels[channel_number])->close();
}

void Server::wait_for_ipport_list(std::vector<std::string> ipport_list)
{
    for (std::string s : ipport_list)
        this->wait_for_ipport(s);
}

void Server::wait_for_ipport(std::string ipport)
{
    std::unique_lock<std::mutex> lock(universal_lock);
    channels_cv.wait(lock, [this, &ipport]
                     { return this->ipport_to_channel.find(ipport) != this->ipport_to_channel.end(); });
}

void Server::receiver_function(const char *message, sockaddr_in sender_addr)
{
    if (ip_to_channel.find(sender_addr.sin_addr.s_addr) == ip_to_channel.end())
        throw std::runtime_error("Receive on non existing channel");

    channel *channel = channels[ip_to_channel[sender_addr.sin_addr.s_addr]];
    {
        unique_lock<mutex> lock(channel->receive_que_lock);
        channel->receive_que.push_back(std::string(message));
    }
    delete message;
}

std::string Server::receive_from_channel(uint32_t channel_number)
{
    unique_lock<mutex> lock(universal_lock);
    if (channels.find(channel_number) == channels.end())
        throw std::runtime_error("Channel not found");

    return channels[channel_number]->receive();
}

std::string Server::receive_from_channel_by_ipport(std::string ipport)
{
    unique_lock<mutex> lock(universal_lock);
    if (ipport_to_channel.find(ipport) == ipport_to_channel.end())
    {
        throw std::runtime_error("channel_not_exist");
    }
    int channel_number = ipport_to_channel[ipport];

    return channels[channel_number]->receive();
}

void Server::handle_receive_packet()
{
    fd_set read_set;
    int nfds = getdtablesize();
    char *buf = new char[MAX_TRANSMITTED_LEN + 1];
    struct sockaddr_in sender;
    socklen_t socklen;
    int received_bytes;

    channel *sender_channel;

    for (;;)
    {
        {
            unique_lock<mutex> lock(universal_lock);
            memcpy(&read_set, &(this->clients_sockets_set), sizeof(this->clients_sockets_set));

            if (select(nfds, &read_set, NULL, NULL, &(this->select_timeval)))
            {

                for (unordered_set<int32_t>::iterator it = client_sockets.begin(); it != client_sockets.end(); ++it)
                {
                    if (FD_ISSET(*it, &read_set))
                    {
                        socklen = sizeof(sockaddr_in);
                        received_bytes = recvfrom(*it, buf, MAX_TRANSMITTED_LEN, 0,
                                                  (struct sockaddr *)&sender, &socklen);
                        if (received_bytes == -1)
                        {
                            continue;
                        }
                        buf[received_bytes] = '\0';

                        char *message = new char[strlen(buf) + 1];
                        strcpy(message, buf);
                        // TO_DO ask how to deal with message

                        std::function<void()> task = [this, message, received_bytes, sender, it]
                        { this->receiver_function(message, sender); };
                        receivers->enqueue(task);
                    }
                }
            }
        }
    }
}

Server::Server() : channel_count(initial_channel_number)
{
    this->senders = new thread_pool(THREAD_POOL_SIZE);
    this->receivers = new thread_pool(THREAD_POOL_SIZE);
    FD_ZERO(&(this->clients_sockets_set));
};

void Server::send_to_all(const char *buf, size_t length)
{
    {
        unique_lock<mutex> lock(universal_lock);
        for (auto it = channels.begin(); it != channels.end(); ++it)
        {

            // std::function<void()> task = [this, it, buf, length]
            // { sendto(it->second->socket, buf, length, 0, (const sockaddr *)&((it->second)->cli_addr), sizeof((it->second)->cli_addr)); };
            // senders->enqueue(task);
            // senders->enqueue([this, it, buf, length]
            //  { sendto(it->second->socket, buf, length, 0, (const sockaddr *)&((it->second)->cli_addr), sizeof((it->second)->cli_addr)); });
        }
    }
}

void Server::send_to_channel(uint32_t channel_number, const char *buf, size_t length)
{
    {
        unique_lock<mutex> lock(universal_lock);
        auto it = channels.find(channel_number);
        if (it == channels.end())
        {
            throw std::runtime_error("channel_not_exist");
        }
        if (!it->second->get_type())
        {
            std::function<void()> task = [this, it, buf, length]
            { sendto(it->second->socket, buf, length, 0, (const sockaddr *)&((it->second)->dest_addr), sizeof((it->second)->dest_addr)); };
            senders->enqueue(task);
            return;
        }
        reliable_channel *channel = static_cast<reliable_channel *>(it->second);
        {
            unique_lock<mutex> lock(channel->send_que_lock);
            channel->send_que.push_back(buf);
        }
    }
}

void Server::send_to_channel_by_ipport(std::string ipport, std::string message)
{
    {
        unique_lock<mutex> lock(universal_lock);
        if (ipport_to_channel.find(ipport) == ipport_to_channel.end())
        {
            throw std::runtime_error("channel_not_exist");
        }
        int channel_number = ipport_to_channel[ipport];

        auto it = channels.find(channel_number);

        // std::cout << channel_number << "\n";
        if (it == channels.end())
        {
            throw std::runtime_error("channel_not_exist");
        }
        if (!it->second->get_type())
        {
            std::function<void()> task = [this, it, message]
            { sendto(it->second->socket, message.c_str(), message.size(), 0, (const sockaddr *)&((it->second)->dest_addr), sizeof((it->second)->dest_addr)); };
            senders->enqueue(task);
            return;
        }
        reliable_channel *channel = static_cast<reliable_channel *>(it->second);
        {
            unique_lock<mutex> lock(channel->send_que_lock);
            channel->send_que.push_back(message);
        }
    }
}

// TO DO: ADD LOCKS
void Server::create_new_unreliable_channel(int32_t client_socket, sockaddr_in cli_addr)
{
    unique_lock<mutex> lock(universal_lock);
    channel *new_channel = new unreliable_channel(
        client_socket,
        ++channel_count,
        cli_addr);
    ip_to_channel[cli_addr.sin_addr.s_addr] = new_channel->channel_number;
    FD_SET(client_socket, &(this->clients_sockets_set));
    client_sockets.insert(client_socket);
    channels[new_channel->channel_number] = new_channel;
}

void Server::listen_server()
{
    int n;
    struct sockaddr_in cli_addr;
    char *buffer = new char[40];
    memset(buffer, 0, 10);
    unordered_set<std::string> pending_connections;
    socklen_t len;
    for (;;)
    {
        // cout << "listening\n";
        bzero(&cli_addr, sizeof(cli_addr));
        len = sizeof(struct sockaddr_in);
        n = recvfrom(server_sockfd, buffer, 10, 0, (struct sockaddr *)&cli_addr,
                     &len);
        if (n > 0)
        {
            buffer[n] = '\0';
            if (buffer[0] == '0')
            {
                // printf("new non reliable connection\n");
                this->handle_new_connection(cli_addr);
            }
            else
            {
                // cout << "got new reliable\n";
                this->handle_new_reliable_connection(cli_addr);
            }
        }
    }
}

void Server::init_server(uint16_t port)
{
    FD_ZERO(&(this->clients_sockets_set));
    this->select_timeval = {0,
                            TIMEVAL_MICROSECONDS};

    this->server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    bind(this->server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    this->receive_thread = thread(&Server::handle_receive_packet, this);
    this->receive_thread.detach();
}

void Server::listen()
{
    this->listen_thread = thread(&Server::listen_server, this);
    this->listen_thread.detach();
}

void Server::handle_new_connection(struct sockaddr_in cli_addr)
{

    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in *new_client = new sockaddr_in;
    bzero(new_client, sizeof(sockaddr_in));

    new_client->sin_family = AF_INET;
    new_client->sin_addr.s_addr = INADDR_ANY;
    new_client->sin_port = 0;

    if (bind(sockfd, (struct sockaddr *)new_client, sizeof(*new_client)) < 0)
    {
        printf("New connection bind error: %s\n", strerror(errno));
    }

    socklen_t len = sizeof(*new_client);

    if (getsockname(sockfd, (struct sockaddr *)new_client, &len) == -1)
    {
        printf("getsockname error:%s\n", strerror(errno));
    }

    char *buf = new char[6];
    sprintf(buf, "%d", ntohs(cli_addr.sin_port));
    printf("buf = %s\n", buf);
    printf("strlen buf = %d\n", strlen(buf));
    int n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
    cout << "n = " << n << "\n";
    create_new_unreliable_channel(sockfd, cli_addr);
    delete[] buf;
}
