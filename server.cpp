#include "server.h"

int main()
{
    Server server;
    server.init_server(4001);
    server.listen();
    for (;;)
    {
        cout << "size = " << server.client_sockets.size() << "\n";
        try
        {
            sleep(1);
            cout << server.receive_from_any() << "\n";
            server.send_to_all("cefaci\n", 6);
        }
        catch (std::runtime_error e)
        {
            cout << "channel not exist\n";
        }
    }
}