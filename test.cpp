#include <iostream>
#include <fstream>
#include <vector>

int main()
{
    std::ifstream file("test/input.txt", std::ios::binary); // Open the file in binary mode

    if (!file)
    {
        std::cerr << "Cannot open the file." << std::endl;
        return 1;
    }

    const size_t bufferSize = 1024;
    std::vector<char> buffer(bufferSize);

    while (file.read(buffer.data(), bufferSize) || file.gcount() != 0)
    {
        std::cout.write(buffer.data(), file.gcount());
    }

    file.close(); // Close the file
    return 0;
}