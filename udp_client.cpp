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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <number_of_iterations>\n";
        return 1;
    }

    int num_iterations = std::stoi(argv[1]);
    if (num_iterations <= 0)
    {
        cout << "The number of iterations must be a positive integer.\n";
        return 1;
    }

    Client client = Client();
    client.init(4000);
    std::string s = gen_random(1000);
    client.reliable_connect_to_one("193.168.0.2", 4001);

    // for(int i=0;i<num_iterations;i++){
    //     client.send(s);
    // }

    // cl.close();

    return 0;
}
