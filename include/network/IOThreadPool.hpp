#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "../../utils/ThreadPool.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

WEBSOCKET_NAMESPACE_BEGIN

class IOThreadPool {
public:
    explicit IOThreadPool(size_t numThreads = 0); // 0 = auto-detect
    ~IOThreadPool();

    WEBSOCKET_DISABLE_COPY(IOThreadPool)

        // Thread pool management
        Result start();
    Result stop();
    Result restart();

    bool isRunning() const;
    size_t getThreadCount() const;
    void resize(size_t numThreads);

    // I/O task submission
    template<typename F, typename... Args>
    auto post(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    template<typename F, typename... Args>
    auto dispatch(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    // Async I/O operations
    Result asyncRead(SharedPtr<IConnection> connection, ByteBuffer& buffer, Callback completionCallback);
    Result asyncWrite(SharedPtr<IConnection> connection, const ByteBuffer& data, Callback completionCallback);
    Result asyncAccept(SharedPtr<IEndpoint> endpoint, EventCallback<SharedPtr<IConnection>> callback);

    // Timer operations
    Result setTimer(uint32_t milliseconds, Callcallback callback, bool recurring = false);
    Result cancelTimer(int timerId);

    // Statistics
    struct IOStats {
        size_t pendingReads;
        size_t pendingWrites;
        size_t pendingAccepts;
        size_t activeTimers;
        size_t completedOperations;
        size_t failedOperations;
    };

    IOStats getStats() const;

    // Configuration
    void setMaxPendingOperations(size_t maxOperations);
    void setOperationTimeout(uint32_t timeoutMs);
    void setLoadBalancingStrategy(const std::string& strategy);

    // Worker thread management
    void pinThreadToCore(size_t threadIndex, int coreId);
    void setThreadPriority(size_t threadIndex, int priority);

private:
    std::vector<std::thread> ioThreads_;
    SharedPtr<ThreadPool> threadPool_;

    std::atomic<bool> isRunning_{ false };
    std::atomic<size_t> threadCount_{ 0 };

    std::atomic<size_t> pendingReads_{ 0 };
    std::atomic<size_t> pendingWrites_{ 0 };
    std::atomic<size_t> pendingAccepts_{ 0 };
    std::atomic<size_t> activeTimers_{ 0 };
    std::atomic<size_t> completedOperations_{ 0 };
    std::atomic<size_t> failedOperations_{ 0 };

    std::atomic<size_t> maxPendingOperations_{ 10000 };
    std::atomic<uint32_t> operationTimeout_{ 30000 }; // 30 seconds

    void workerFunction(size_t threadIndex);
    void initializeThreadPool(size_t numThreads);
    void shutdownThreadPool();

    // Timer management
    struct TimerInfo {
        int id;
        std::chrono::steady_clock::time_point expiration;
        uint32_t interval;
        Callback callback;
        bool recurring;
        bool active;
    };

    std::unordered_map<int, TimerInfo> timers_;
    std::atomic<int> nextTimerId_{ 1 };
    mutable std::mutex timersMutex_;

    void processTimers();
    int createTimerId();
};

// Template implementations
template<typename F, typename... Args>
auto IOThreadPool::post(F&& f, Args&&... args)
-> std::future<typename std::invoke_result<F, Args...>::type> {
    if (!threadPool_) {
        throw std::runtime_error("IOThreadPool not initialized");
    }
    return threadPool_->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename F, typename... Args>
auto IOThreadPool::dispatch(F&& f, Args&&... args)
-> std::future<typename std::invoke_result<F, Args...>::type> {
    // For immediate execution if possible, otherwise post to queue
    if (std::this_thread::get_id() == ioThreads_[0].get_id()) {
        // Already in I/O thread, execute immediately
        using return_type = typename std::invoke_result<F, Args...>::type;
        std::promise<return_type> promise;
        auto future = promise.get_future();
        try {
            promise.set_value(std::forward<F>(f)(std::forward<Args>(args)...));
        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }
        return future;
    }
    return post(std::forward<F>(f), std::forward<Args>(args)...);
}

WEBSOCKET_NAMESPACE_END