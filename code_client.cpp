
#include <iostream>
#include "client.h"
#include <iostream>

using std::cin;
using std::cout;

int main()
{
    init_client(4005);
    connect_to_server("172.7.0.1", 4001);
    for (;;)
    {
        char *buf = new char[100];
        size_t len;
        cout << "123342342\n";
        int n = receive_from_server(buf, &len);
        cout << "am primit\n";
        buf[n] = '\0';
        cout << buf << "\n";
    }
}
