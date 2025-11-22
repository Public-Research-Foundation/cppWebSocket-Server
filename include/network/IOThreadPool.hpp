#pragma once
#ifndef WEBSOCKET_IO_THREAD_POOL_HPP
#define WEBSOCKET_IO_THREAD_POOL_HPP

#include "../common/Types.hpp"
#include "../common/NonCopyable.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <functional>

// Forward declaration for ASIO
namespace asio {
    class io_context;
}

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class IOThreadPool
 * @brief Manages a pool of I/O threads for asynchronous operations
 *
 * Provides:
 * - Fixed-size thread pool for I/O operations
 * - Work stealing for load balancing
 * - Graceful shutdown support
 * - Thread affinity configuration
 */
    class IOThreadPool : public NonCopyable {
    public:
        using WorkHandler = std::function<void()>;

        /**
         * @brief Thread pool configuration
         */
        struct Config {
            size_t thread_count{ 0 };              ///< Number of threads (0 = hardware_concurrency)
            size_t queue_size_per_thread{ 1024 };  ///< Task queue size per thread
            bool enable_affinity{ false };         ///< Enable CPU affinity
            std::string name{ "IOThreadPool" };    ///< Pool name for logging
        };

        /**
         * @brief Construct a new IOThreadPool
         * @param config Thread pool configuration
         */
        explicit IOThreadPool(const Config& config = Config{});
        ~IOThreadPool();

        /**
         * @brief Start the thread pool
         * @return true if started successfully
         */
        bool start();

        /**
         * @brief Stop the thread pool (graceful shutdown)
         */
        void stop();

        /**
         * @brief Stop immediately (non-graceful)
         */
        void stopNow();

        /**
         * @brief Post work to the thread pool
         * @param handler Work handler to execute
         * @return true if work queued successfully
         */
        bool post(WorkHandler handler);

        /**
         * @brief Get thread pool statistics
         * @return Thread pool statistics
         */
        ThreadPoolStats getStats() const;

        /**
         * @brief Check if thread pool is running
         * @return true if pool is active
         */
        bool isRunning() const;

        /**
         * @brief Get number of active threads
         * @return Current thread count
         */
        size_t getThreadCount() const;

        /**
         * @brief Get number of pending tasks
         * @return Total pending tasks across all threads
         */
        size_t getPendingTaskCount() const;

        /**
         * @brief Get ASIO io_context for direct use
         * @return Reference to ASIO io_context
         */
        asio::io_context& getIoContext();

    private:
        /**
         * @brief Worker thread function
         * @param thread_index Index of this worker thread
         */
        void workerThread(size_t thread_index);

        /**
         * @brief Setup thread affinity if enabled
         * @param thread_index Index of thread for affinity
         */
        void setupThreadAffinity(size_t thread_index);

        /**
         * @brief Initialize ASIO io_contexts
         */
        void initializeIoContexts();

        /**
         * @brief Cleanup resources
         */
        void cleanup();

        // Member variables
        Config config_;
        std::vector<std::unique_ptr<asio::io_context>> io_contexts_;
        std::vector<asio::executor_work_guard<asio::io_context::executor_type>> work_guards_;
        std::vector<std::thread> threads_;
        std::atomic<bool> running_{ false };
        std::atomic<size_t> next_thread_index_{ 0 };
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_IO_THREAD_POOL_HPP