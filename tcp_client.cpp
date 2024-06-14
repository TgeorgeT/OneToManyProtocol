#include <iostream>
#include "client/client.h"
#include <unistd.h>
#include <fstream>
#include <sstream>
using std::cin;
using std::cout;

std::string gen_random(const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
    {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

int main()
{
    client cl = client();
    cl.init_client(4000);

    cl.reliable_connect_to_server("193.168.0.2", 4001);

    cout << cl.channel->receive() << "\n";

    cl.channel->send("got them");

    cl.channel->close_connection();
}