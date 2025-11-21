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

/**
 * High-performance thread pool for concurrent task execution
 * Features:
 * - Dynamic thread management
 * - Task queue with priority
 * - Wait-for-completion support
 * - Statistics and monitoring
 * - Exception-safe task execution
 */
    class ThreadPool {
    public:
        /**
         * Create thread pool with specified number of threads
         * @param numThreads Number of worker threads (0 = auto-detect)
         */
        explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
        ~ThreadPool();

        WEBSOCKET_DISABLE_COPY(ThreadPool)

            /**
             * Enqueue task for execution
             * @tparam F Callable type (function, lambda, etc.)
             * @tparam Args Argument types
             * @param f Callable to execute
             * @param args Arguments to pass to callable
             * @return std::future for retrieving result or exception
             */
            template<typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args)
            -> std::future<typename std::invoke_result<F, Args...>::type>;

        /**
         * Wait for all currently queued tasks to complete
         * Does not stop the thread pool
         */
        void waitAll();

        /**
         * Stop thread pool and reject new tasks
         * Running tasks will complete
         */
        void stop();

        /**
         * Restart stopped thread pool
         */
        void restart();

        // ===== MONITORING =====

        size_t getQueueSize() const;
        size_t getActiveThreadCount() const;
        size_t getTotalThreadCount() const;
        bool isRunning() const;

        /**
         * Resize thread pool (dynamic scaling)
         * @param numThreads New number of threads
         */
        void resize(size_t numThreads);

        // Statistics for performance monitoring
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

        /**
         * Worker thread function
         * Continuously takes tasks from queue and executes them
         */
        void workerFunction();
};

// Template implementation must be in header
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::invoke_result<F, Args...>::type> {

    using return_type = typename std::invoke_result<F, Args...>::type;

    // Create packaged task for future result
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