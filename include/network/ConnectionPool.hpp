#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "interfaces/IConnection.hpp"
#include <queue>
#include <unordered_set>
#include <memory>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class ConnectionPool {
public:
    explicit ConnectionPool(size_t initialSize = 10, size_t maxSize = 1000);
    ~ConnectionPool();

    WEBSOCKET_DISABLE_COPY(ConnectionPool)

        SharedPtr<IConnection> acquire();
    void release(SharedPtr<IConnection> connection);

    bool isValid(SharedPtr<IConnection> connection) const;
    void invalidate(SharedPtr<IConnection> connection);

    void clear();
    void resize(size_t newMaxSize);

    // Statistics
    size_t getAvailableCount() const;
    size_t getActiveCount() const;
    size_t getMaxSize() const;
    size_t getTotalCreated() const;
    size_t getTotalDestroyed() const;

    struct Stats {
        size_t available;
        size_t active;
        size_t maxSize;
        size_t totalCreated;
        size_t totalDestroyed;
    };

    Stats getStats() const;

    // Configuration
    void setConnectionTimeout(uint32_t timeoutMs);
    void setMaxIdleTime(uint32_t idleTimeMs);
    void setHealthCheckInterval(uint32_t intervalMs);

private:
    mutable std::shared_mutex mutex_;

    std::queue<SharedPtr<IConnection>> availableConnections_;
    std::unordered_set<SharedPtr<IConnection>> activeConnections_;

    size_t maxSize_;
    std::atomic<size_t> totalCreated_{ 0 };
    std::atomic<size_t> totalDestroyed_{ 0 };

    std::atomic<uint32_t> connectionTimeout_{ 30000 }; // 30 seconds
    std::atomic<uint32_t> maxIdleTime_{ 60000 }; // 60 seconds
    std::atomic<uint32_t> healthCheckInterval_{ 30000 }; // 30 seconds

    SharedPtr<IConnection> createConnection();
    void destroyConnection(SharedPtr<IConnection> connection);
    void performHealthChecks();
    bool isConnectionHealthy(SharedPtr<IConnection> connection) const;
};

WEBSOCKET_NAMESPACE_END