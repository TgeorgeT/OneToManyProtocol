#include "channel.h"

std::string unreliable_channel::receive()
{
    std::unique_lock<std::mutex> lock(receive_que_lock);
    receive_que_cv.wait(lock, [this]
                        { return !receive_que.empty(); });
    std::string message = receive_que.front();
    receive_que.pop_front();
    return message;
}