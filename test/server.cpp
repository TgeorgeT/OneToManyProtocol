#include <iostream>
#include "server/server.h"
#include <fstream>
#include <chrono>

using std::cout;
std::vector<std::string> read_from_file(std::string file_name)
{
    std::ifstream file(file_name);
    std::vector<std::string> lines;

    if (!file.is_open())
    {
        return {};
    }
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    file.close();
    return lines;
}

int main()
{
    Server server;
    server.init_server(4001);
    server.listen();

    std::vector<std::string> ipport_list{"172.10.0.2:4000", "172.10.0.3:4000", "172.10.0.4:4000"};
    std::vector<std::string> numbers_list = read_from_file("numbers.txt");
    server.wait_for_ipport_list(ipport_list);

    for (int i = 0; i < ipport_list.size(); i++)
        server.send_to_channel_by_ipport(ipport_list[i], numbers_list[i]);

    int result = 0;

    for (int i = 0; i < ipport_list.size(); i++)
        result += std::stoi(server.receive_from_channel_by_ipport(ipport_list[i]));

    std::cout << "result = " << result << "\n";

    for (;;)
        ;
    return 0;
}