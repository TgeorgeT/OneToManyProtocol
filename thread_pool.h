#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

using std::condition_variable;
using std::mutex;
using std::thread;
using std::vector;

class thread_pool
{
private:
    vector<thread> workers;
    mutex queue_mutex;
    condition_variable condition;
    bool stop;

public:
    thread_pool(concurrency_t thread_count);
    template <class F, class... Args>
}