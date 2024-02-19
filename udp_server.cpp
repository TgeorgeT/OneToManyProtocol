#include <iostream>
#include "server/server.h"

using std::cout;

int main()
{
    Server server;
    server.init_server(4001);
    server.listen();
    for (;;)
    {
        // try
        // {
        //     sleep(2);
        //     cout << "channel 1 = " << server.receive_from_channel(1) << "\n";
        //     // cout << "channel 2 = " << server.receive_from_channel(2) << "\n";
        //     // cout << "channel 3 = " << server.receive_from_channel(3) << "\n";
        // }
        // catch (...)
        // {

        //     //cout << "channel not exist\n";
        // }
    }
}