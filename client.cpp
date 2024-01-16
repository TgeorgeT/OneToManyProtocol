#include <iostream>
#include "client.h"
#include <iostream>

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

    srand((unsigned)time(NULL) * getpid());
    init_client(4005);
    connect_to_server("172.7.0.1", 4001);
    char *message = new char[11];
    for (int i = 0; i < 2; i++)
    {
        char *buf = new char[100];
        size_t len;
        send_to_server((gen_random(10) + "\n").c_str(), 11);
        int n = receive_from_server(buf, &len);
        buf[n] = '\0';
        cout << buf << "\n";

        sleep(2);
    }

    unreliable_announce_end();
}
