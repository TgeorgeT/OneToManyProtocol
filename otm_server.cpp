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

class OneEndpoint : public Server
{
};

int main(int argc, char *argv[])
{
    OneEndpoint one_endpoint;
    one_endpoint.init(4001);
    one_endpoint.listen();
    std::vector<std::string> ipport_list{"172.10.0.2:4000", "172.10.0.3:4000", "172.10.0.4:4000"};
    std::vector<std::string> numbers_lists = read_from_file("numbers.txt");

    one_endpoint.wait_for_endpoints(ipport_list);
    one_endpoint.send(ipport_list, numbers_lists);

    int result = 0;
    for (std::string message : one_endpoint.receive(ipport_list))
        result += std::stoi(message);

    std::cout << "result = " << result << "\n";

    one_endpoint.stop();
    return 0;
}


