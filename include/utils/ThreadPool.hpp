#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    WEBSOCKET_DISABLE_COPY(ThreadPool)

        template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    void waitAll();
    void stop();
    void restart();

    size_t getQueueSize() const;
    size_t getActiveThreadCount() const;
    size_t getTotalThreadCount() const;

    bool isRunning() const;
    void resize(size_t numThreads);

    // Statistics
    struct Stats {
        size_t totalTasks;
        size_t completedTasks;
        size_t failedTasks;
        size_t queueSize;
        size_t activeThreads;
    };

    Stats getStats() const;

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::condition_variable completionCondition_;

    std::atomic<bool> stop_{ false };
    std::atomic<size_t> activeThreads_{ 0 };
    std::atomic<size_t> totalTasks_{ 0 };
    std::atomic<size_t> completedTasks_{ 0 };
    std::atomic<size_t> failedTasks_{ 0 };

    void workerFunction();
};

// Template implementation
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::invoke_result<F, Args...>::type> {

    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
        std::unique_lock lock(queueMutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
        totalTasks_++;
    }

    condition_.notify_one();
    return result;
}

WEBSOCKET_NAMESPACE_END