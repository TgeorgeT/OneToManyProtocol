#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

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
int main(int argc, char *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    std::string s = gen_random(1000);
    char *c = new char[s.size()];
    strcpy(c, s.c_str());

    int num_iterations = std::stoi(argv[1]);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "193.168.0.2", &serv_addr.sin_addr) <= 0)
    {
        return 1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        return 1;
    }
    int sum = 0;

    for (int i = 0; i < num_iterations; i++)
    {
        send(sock, c, s.size(), 0);
        sum += s.size();
    }

    close(sock);
    return 0;
}