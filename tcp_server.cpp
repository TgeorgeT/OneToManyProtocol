#include <iostream>
#include "server/server.h"
#include <fstream>
#include <chrono>

using std::cout;

int main()
{
    cout << "server started\n";
    Server server;
    server.init_server(4001);
    server.listen();

    server.wait_for_ipport("172.10.0.24000");

    std::cout << "client connected\n";

    server.send_to_channel_by_ipport("172.10.0.24000", "123456", 6);
    cout << server.receive_from_channel(1) << "\n";
    server.send_to_channel_by_ipport("172.10.0.24000", "123456", 6);

    cout << "TOTU OK\n";
    for (;;)
}