#include <iostream>
#include "client/client.h"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstdlib>

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

int compute(const std::string &str)
{
    int result = 0;
    size_t start = 0;
    char delim = ',';

    while (start < str.size())
    {
        size_t found = str.find(delim, start);
        if (found == std::string::npos)
        {
            result += std::stoi(str.substr(start));
            break;
        }
        else
        {
            result += std::stoi(str.substr(start, found - start));
            start = found + 1;
        }
    }

    return result;
}

class ManyEnpoint : public Client
{
};

int main(int argc, char *argv[])
{
    ManyEnpoint many_endpoint = ManyEnpoint();
    many_endpoint.init(4000);
    many_endpoint.reliable_connect("193.168.0.2", 4001);

    std::string s = many_endpoint.receive();
    cout << s << "\n";
    cout << "sending " << std::to_string(compute(s)) << "\n";
    many_endpoint.send(std::to_string(compute(s)));
    many_endpoint.close();

    return 0;
}



