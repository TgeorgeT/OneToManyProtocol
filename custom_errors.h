class ConnectionError : public std::exception
{
private:
    char *message;
public:
    ConnectionError(char *msg) : message(msg){};
    char *what()
    {
        return message;
    }
};

void err_sys(const char *x)
{
    perror(x);
    exit(1);
}