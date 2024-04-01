#include <iostream>
#include "client/client.h"
#include <unistd.h>

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
    srand((unsigned)time(NULL) * getpid());
    cl.init_client(4000);

    cout << "dupa init\n";
    // connect_to_server("172.7.0.1", 4001);
    cl.reliable_connect_to_server("193.168.0.2", 4001);

    // cout << "dupa connect\n";
    // cl.channel->send("123");
    // cl.channel->send("123");
    std::string s;
    // sleep(1);
    for (int i = 0; i < 16000; i++)
    {
        // sleep(1);
        s = gen_random(1000);
        cout << "-----------------------------------\n"
             << s << "\n-----------------------------------\n";
        cl.channel->send(s);
    }
    for (;;)
        ;
    // char *message = new char[11];
    // for (int i = 0; i < 2; i++)
    // {
    //     char *buf = new char[501];
    //     size_t len;
    //     send_to_server((gen_random(500) + "\n").c_str(), 500);
    //     // int n = receive_from_server(buf, &len);
    //     // buf[n] = '\0';
    //     // cout << buf << "\n";

    //     sleep(0.5);
    // }

    // unreliable_announce_end();
}
