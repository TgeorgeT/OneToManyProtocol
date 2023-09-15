#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <utility>
#include "custom_errors.h"
#include <sys/socket.h>
#include <string.h>
#include <vector>

using std::pair;
using std::string;
using std::vector;


class protocol
{
private:
    void make_socket(const pair<string, u_int16_t> & ip_port)
    {

        int sockfd;
        struct sockaddr_in servaddr;
        if (sockfd = socket(AF_INET, SOCK_STREAM, 0) < 0)
        {
            err_sys("socket_error");
        }
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(ip_port.second);
        

    }

public:
    int Connect(const vector<pair<string, u_int16_t>> &endpoints)
    {
        for (pair<string, u_int16_t> ip_port : endpoints)
        {
            make_socket(ip_port);
        }
        return 0;
    }
};