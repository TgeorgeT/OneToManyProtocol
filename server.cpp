#include <iostream>
#include "server.h"
#include <iostream>

using std::cin;
using std::cout;

int main()
{
    Server server = Server();
    server.init_server(4001);
    server.listen();
    while (1)
    {
        sleep(1);
        size_t len = 5;
        server.send_to_channel("1213", &len, 2);
    }
}