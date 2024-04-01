#include <exception>
#include <string>

class NetworkException : public std::exception
{
private:
    std::string message; // Stores the error message

public:
    // Constructor that takes a string message
    NetworkException(const std::string &msg) : message(msg) {}

    // Override the what() method to return the error message
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};