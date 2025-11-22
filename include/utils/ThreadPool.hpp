#pragma once
#ifndef WEBSOCKET_THREAD_POOL_HPP
#define WEBSOCKET_THREAD_POOL_HPP

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
#include <memory>
#include <stdexcept>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class ThreadPool
 * @brief High-performance thread pool for concurrent task execution
 *
 * Features:
 * - Dynamic thread management
 * - Task queue with priority support
 * - Wait-for-completion support
 * - Comprehensive statistics and monitoring
 * - Exception-safe task execution
 * - Graceful shutdown
 *
 * Usage:
 * ThreadPool pool(4); // 4 threads
 * auto future = pool.enqueue([]() { return 42; });
 * int result = future.get();
 */
    class ThreadPool {
    public:
        /**
         * @brief Thread pool configuration structure
         */
        struct Config {
            size_t numThreads{ 0 };                    ///< Number of worker threads (0 = auto-detect)
            size_t maxQueueSize{ 1000 };               ///< Maximum queue size before rejection
            bool enableStatistics{ true };             ///< Enable performance statistics
            std::string name{ "ThreadPool" };          ///< Pool name for logging
        };

        /**
         * @brief Thread pool statistics structure
         */
        struct Stats {
            size_t totalTasks{ 0 };                    ///< Total tasks submitted
            size_t completedTasks{ 0 };                ///< Successfully completed tasks
            size_t failedTasks{ 0 };                   ///< Tasks that failed with exception
            size_t queueSize{ 0 };                     ///< Current queue size
            size_t activeThreads{ 0 };                 ///< Currently active threads
            size_t totalThreads{ 0 };                  ///< Total worker threads
            size_t peakQueueSize{ 0 };                 ///< Peak queue size
            size_t rejectedTasks{ 0 };                 ///< Tasks rejected due to queue full
        };

        /**
         * @brief Create thread pool with default configuration
         */
        ThreadPool();

        /**
         * @brief Create thread pool with specified number of threads
         * @param numThreads Number of worker threads (0 = auto-detect)
         */
        explicit ThreadPool(size_t numThreads);

        /**
         * @brief Create thread pool with configuration
         * @param config Thread pool configuration
         */
        explicit ThreadPool(const Config& config);

        /**
         * @brief Destructor - stops pool and waits for completion
         */
        ~ThreadPool();

        // Delete copy and move operations
        WEBSOCKET_DISABLE_COPY(ThreadPool)
            WEBSOCKET_DISABLE_MOVE(ThreadPool)

            /**
             * @brief Enqueue task for execution
             * @tparam F Callable type (function, lambda, etc.)
             * @tparam Args Argument types
             * @param f Callable to execute
             * @param args Arguments to pass to callable
             * @return std::future for retrieving result or exception
             * @throws std::runtime_error if pool is stopped or queue is full
             */
            template<typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args)
            -> std::future<typename std::invoke_result_t<F, Args...>>;

        /**
         * @brief Enqueue task with priority (lower number = higher priority)
         * @tparam F Callable type
         * @tparam Args Argument types
         * @param priority Task priority (0 = highest)
         * @param f Callable to execute
         * @param args Arguments to pass to callable
         * @return std::future for retrieving result
         */
        template<typename F, typename... Args>
        auto enqueueWithPriority(int priority, F&& f, Args&&... args)
            -> std::future<typename std::invoke_result_t<F, Args...>>;

        /**
         * @brief Wait for all currently queued tasks to complete
         * Does not stop the thread pool
         * @param timeoutMs Maximum time to wait in milliseconds (0 = wait forever)
         * @return true if all tasks completed, false if timeout reached
         */
        bool waitAll(uint64_t timeoutMs = 0);

        /**
         * @brief Stop thread pool and reject new tasks
         * Running tasks will complete, queued tasks will be discarded
         */
        void stop();

        /**
         * @brief Stop immediately, interrupting running tasks
         * @warning This may leave resources in inconsistent state
         */
        void stopNow();

        /**
         * @brief Restart stopped thread pool
         * @return true if restart successful
         */
        bool restart();

        /**
         * @brief Check if thread pool is running
         * @return true if pool is active and accepting tasks
         */
        bool isRunning() const;

        /**
         * @brief Check if thread pool is stopped
         * @return true if pool is stopped
         */
        bool isStopped() const;

        // ===== MONITORING AND MANAGEMENT =====

        /**
         * @brief Get current queue size
         * @return Number of pending tasks
         */
        size_t getQueueSize() const;

        /**
         * @brief Get number of active threads
         * @return Currently executing threads count
         */
        size_t getActiveThreadCount() const;

        /**
         * @brief Get total number of threads
         * @return Total worker threads count
         */
        size_t getTotalThreadCount() const;

        /**
         * @brief Get thread pool statistics
         * @return Current statistics snapshot
         */
        Stats getStats() const;

        /**
         * @brief Resize thread pool (dynamic scaling)
         * @param numThreads New number of threads
         * @return true if resize successful
         */
        bool resize(size_t numThreads);

        /**
         * @brief Get current configuration
         * @return Current configuration
         */
        Config getConfig() const;

        /**
         * @brief Update configuration
         * @param config New configuration
         * @return true if configuration applied successfully
         */
        bool setConfig(const Config& config);

        /**
         * @brief Clear task queue (discard pending tasks)
         * @return Number of tasks cleared
         */
        size_t clearQueue();

    private:
        /**
         * @brief Task structure with priority support
         */
        struct Task {
            std::function<void()> function;
            int priority{ 0 };

            // For priority queue ordering (lower priority number = higher priority)
            bool operator<(const Task& other) const {
                return priority > other.priority; // Min-heap behavior
            }
        };

        /**
         * @brief Initialize thread pool with current configuration
         */
        void initialize();

        /**
         * @brief Worker thread function
         */
        void workerFunction();

        /**
         * @brief Update statistics
         */
        void updateStats();

        // Configuration
        Config config_;

        // Worker threads
        std::vector<std::thread> workers_;

        // Task queue with priority
        std::priority_queue<Task> tasks_;

        // Synchronization
        mutable std::mutex queueMutex_;
        std::condition_variable condition_;
        std::condition_variable completionCondition_;

        // State management
        std::atomic<bool> stop_{ false };
        std::atomic<bool> stopNow_{ false };
        std::atomic<size_t> activeThreads_{ 0 };

        // Statistics
        mutable std::mutex statsMutex_;
        Stats stats_;
        std::atomic<size_t> pendingTasks_{ 0 };
};

// Template implementation must be in header
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::invoke_result_t<F, Args...>> {

    using return_type = typename std::invoke_result_t<F, Args...>;

    // Create packaged task for future result
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
        std::unique_lock lock(queueMutex_);

        // Check if pool is stopped
        if (stop_ || stopNow_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        // Check queue size limit
        if (tasks_.size() >= config_.maxQueueSize) {
            stats_.rejectedTasks++;
            throw std::runtime_error("ThreadPool queue is full");
        }

        // Enqueue task
        tasks_.push(Task{
            [task]() { (*task)(); },
            0 // Default priority
            });

        // Update statistics
        stats_.totalTasks++;
        stats_.queueSize = tasks_.size();
        if (stats_.queueSize > stats_.peakQueueSize) {
            stats_.peakQueueSize = stats_.queueSize;
        }
        pendingTasks_++;
    }

    // Notify one waiting thread
    condition_.notify_one();
    return result;
}

template<typename F, typename... Args>
auto ThreadPool::enqueueWithPriority(int priority, F&& f, Args&&... args)
-> std::future<typename std::invoke_result_t<F, Args...>> {

    using return_type = typename std::invoke_result_t<F, Args...>;

    // Create packaged task for future result
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
        std::unique_lock lock(queueMutex_);

        // Check if pool is stopped
        if (stop_ || stopNow_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        // Check queue size limit
        if (tasks_.size() >= config_.maxQueueSize) {
            stats_.rejectedTasks++;
            throw std::runtime_error("ThreadPool queue is full");
        }

        // Enqueue task with priority
        tasks_.push(Task{
            [task]() { (*task)(); },
            priority
            });

        // Update statistics
        stats_.totalTasks++;
        stats_.queueSize = tasks_.size();
        if (stats_.queueSize > stats_.peakQueueSize) {
            stats_.peakQueueSize = stats_.queueSize;
        }
        pendingTasks_++;
    }

    // Notify one waiting thread
    condition_.notify_one();
    return result;
}

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_THREAD_POOL_HPP