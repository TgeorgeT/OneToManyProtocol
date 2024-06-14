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

int main(int argc, char *argv[])
{
    cout << "server started\n";
    Server server;
    server.init(4001);
    server.listen();

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

    std::vector<std::string> ipport_list{"172.10.0.2:4000"};//, "172.10.0.3:4000", "172.10.0.4:4000"}; // "172.10.0.5:4000", "172.10.0.6:4000"};
    std::vector<std::string>
        numbers_list = read_from_file("numbers.txt");
    server.wait_for_ipport_list(ipport_list);

    // for (int i = 0; i < ipport_list.size(); i++)
    //     server.send_to_channel_by_ipport(ipport_list[i], numbers_list[i]);
    int result = 0;
    auto start = std::chrono::system_clock::now();
    std::cout.flush();
    int total_bytes = 0;
    for (int i = 0; i < ipport_list.size(); i++)
    {
        for (int j = 0; j < num_iterations; j++)
        {
            cout << j << "\n";
            total_bytes += (server.receive_from_channel_by_ipport(ipport_list[i])).size();
        }
    }
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - start;
    cout << diff.count() << "\n";
    cout << total_bytes << "\n";
    cout.flush();

    for (;;)
        ;
    return 0;
}