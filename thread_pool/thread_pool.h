#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <future>

using std::bind;
using std::condition_variable;
using std::forward;
using std::function;
using std::future;
using std::make_shared;
using std::mutex;
using std::packaged_task;
using std::queue;
using std::thread;
using std::unique_lock;
using std::vector;

#ifndef thread_pool_h
#define thread_pool_h
class thread_pool
{
private:
    vector<thread> workers;
    mutex queue_mutex;
    condition_variable condition;
    bool stop;
    queue<function<void()>> tasks;

public:
    thread_pool(size_t no_threads);
    ~thread_pool();
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
};

#endif