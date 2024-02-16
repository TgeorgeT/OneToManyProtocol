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

thread_pool::thread_pool(size_t no_threads) : stop(false)
{
    for (size_t i = 0; i < no_threads; i++)
    {
        workers.emplace_back(
            [this]
            {
                for (;;)
                {
                    function<void()> task;
                    {
                        unique_lock<mutex> lock(this->queue_mutex);

                        condition.wait(lock, [this]
                                       { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
    }
};

template <class F, class... Args>
auto thread_pool::enqueue(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = make_shared<packaged_task<return_type()>>(
        std::bind(forward<F>(f), forward<Args>(args)...));
    future<return_type> res = task->get_future();
    {
        unique_lock<mutex> lock(queue_mutex);
        if (stop)
            throw std::runtime_error("enqueue on stopped thread pool");
        tasks.emplace([task]
                      { (*task)(); });
    }

    condition.notify_one();
    return res;
}

thread_pool::~thread_pool()
{
    {
        unique_lock<mutex> lock(queue_mutex);
        stop = true;
    }

    condition.notify_all();
    for (thread &worker : workers)
        worker.join();
}