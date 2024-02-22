#include "server.h"

using std::cout;
using std::thread;
using std::unique_lock;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void Server::receiver_function(const char *message, sockaddr_in sender_addr)
{
    channel *sender_channel;
    {
        unique_lock<mutex> lock(universal_lock);
        sender_channel = channels[ip_to_channel[sender_addr.sin_addr.s_addr]];
        {
            sender_channel->receive_que.push_back(message);
        }
    }
}

std::string Server::receive_from_channel(uint32_t channel_number)
{
    if (channels.find(channel_number) == channels.end())
    {
        throw std::runtime_error("channel_not_exist");
    }
    channel *receive_channel = channels[channel_number];
    std::string receive_buf;
    while (1)
    {
        if (receive_channel->receive_que.empty())
        {
            continue;
        }
        receive_buf = std::move(receive_channel->receive_que.front());
        receive_channel->receive_que.pop_front();

        cout << receive_buf << "\n";
        if (receive_buf == "unreliable_end")
        {
            try
            {
                unique_lock<mutex> lock(universal_lock);
                close(receive_channel->socket);
                printf("am dat erase\n ");
                channels.erase(channel_number);
                cout << "channel_number = " << channels.size() << "\n";
                client_sockets.erase(receive_channel->socket);
                return "connection_closed\n";
            }
            catch (...)
            {
                return "unreliable_close_error\n";
            }
        }

        return receive_buf;
    }
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
                        buf[received_bytes] = '\0';

                        char *message = new char[strlen(buf)];
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

void Server::send_to_channel(const char *buf, size_t length, uint32_t channel_number)
{
    {
        unique_lock<mutex> lock(universal_lock);

        auto it = channels.find(channel_number);

        if (it == channels.end())
        {
            throw std::runtime_error("channel_not_exist");
        }
        // std::function<void()> task = [this, it, buf, length]
        // { sendto(it->second->socket, buf, length, 0, (const sockaddr *)&((it->second)->cli_addr), sizeof((it->second)->cli_addr)); };
        // senders->enqueue(task);
        // senders->enqueue([this, it, buf, length]
        //  { sendto(it->second->socket, buf, length, 0, (const sockaddr *)&((it->second)->cli_addr), sizeof((it->second)->cli_addr)); });
    }
}

void Server::create_new_unreliable_channel(int32_t client_socket, sockaddr_in cli_addr)
{

    channel *new_channel = new unreliable_channel(
        client_socket,
        ++channel_count,
        cli_addr);
    ip_to_channel[cli_addr.sin_addr.s_addr] = new_channel->channel_number;
    {
        FD_SET(client_socket, &(this->clients_sockets_set));
        client_sockets.insert(client_socket);
    }

    channels[new_channel->channel_number] = new_channel;
}
void Server::listen_server()
{
    int n;
    struct sockaddr_in cli_addr;
    char *buffer = new char[10];
    socklen_t len;
    printf("listening\n");
    for (;;)
    {
        // cout << "listening\n";
        bzero(&cli_addr, sizeof(cli_addr));
        len = sizeof(struct sockaddr_in);
        n = recvfrom(server_sockfd, buffer, 10, 0, (struct sockaddr *)&cli_addr,
                     &len);

        cout << "new connection = \n"
             << n << "\n";
        buffer[n] = '\0';
        if (n > 0)
        {
            if (buffer[0] == '0')
            {
                printf("new connection\n");
                this->handle_new_connection(cli_addr);
            }
            else
            {
                std::thread(&Server::handle_new_reliable_connection, this, cli_addr).detach();
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

#