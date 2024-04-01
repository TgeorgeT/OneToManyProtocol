#include <iostream>
#include "server/server.h"

using std::cout;

int main()
{
    cout << "server started\n";
    Server server;
    server.init_server(4001);
    server.listen();
    while (server.channel_count == 0)
    {
    };
    reliable_channel *channel;
    {
        std::unique_lock<std::mutex> lock(server.universal_lock);
        channel = static_cast<reliable_channel *>(server.channels[1]);
    }

    auto time1 = std::chrono::system_clock::now();
    for (int i = 0; i < 8000; i++)
    {
        cout << i << "\n";
        std::string message = channel->receive();
    }
    std::chrono::duration<double> diff = std::chrono::system_clock::now() - time1;
    cout << "Time: " << diff.count() << "\n";
}