#pragma once
#ifndef WEBSOCKET_CONNECTION_POOL_HPP
#define WEBSOCKET_CONNECTION_POOL_HPP

#include "../common/Types.hpp"
#include "WebSocketConnection.hpp"
#include <memory>
#include <vector>
#include <mutex>
#include <queue>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class ConnectionPool
 * @brief Manages pool of WebSocket connections for efficient resource reuse
 *
 * Features:
 * - Object pooling to reduce allocation overhead
 * - Connection reuse for better performance
 * - Configurable pool sizes and growth strategies
 * - Thread-safe operations
 */
    class ConnectionPool {
    public:
        /**
         * @brief Pool configuration parameters
         */
        struct Config {
            size_t initial_size{ 100 };           ///< Initial pool size
            size_t max_size{ 1000 };              ///< Maximum pool size
            size_t grow_size{ 50 };               ///< Growth increment when pool is empty
            bool enable_growth{ true };           ///< Allow pool to grow dynamically
        };

        /**
         * @brief Construct a new Connection Pool
         * @param config Pool configuration
         */
        explicit ConnectionPool(const Config& config = Config{});
        ~ConnectionPool();

        // Delete copy constructor and assignment operator
        ConnectionPool(const ConnectionPool&) = delete;
        ConnectionPool& operator=(const ConnectionPool&) = delete;

        /**
         * @brief Acquire a connection from the pool
         * @return Shared pointer to WebSocketConnection
         *
         * @note If pool is empty, may create new connection or return nullptr based on configuration
         */
        std::shared_ptr<WebSocketConnection> acquire();

        /**
         * @brief Release a connection back to the pool
         * @param connection Connection to release
         *
         * @note Connection is reset before being returned to pool
         */
        void release(std::shared_ptr<WebSocketConnection> connection);

        /**
         * @brief Get current pool statistics
         * @return Pool statistics
         */
        PoolStats getStats() const;

        /**
         * @brief Resize the connection pool
         * @param new_size New pool size
         * @return true if resize successful
         */
        bool resize(size_t new_size);

        /**
         * @brief Clear all connections from pool
         */
        void clear();

        /**
         * @brief Get pool configuration
         * @return Current configuration
         */
        Config getConfig() const;

        /**
         * @brief Update pool configuration
         * @param config New configuration
         */
        void setConfig(const Config& config);

    private:
        /**
         * @brief Initialize pool with initial connections
         */
        void initializePool();

        /**
         * @brief Create a new connection instance
         * @return New connection instance
         */
        std::shared_ptr<WebSocketConnection> createConnection();

        /**
         * @brief Grow the pool by configured growth size
         */
        void growPool();

        // Member variables
        mutable std::mutex mutex_;
        std::queue<std::shared_ptr<WebSocketConnection>> available_connections_;
        std::vector<std::shared_ptr<WebSocketConnection>> all_connections_;
        Config config_;
        PoolStats stats_;
        std::atomic<size_t> active_count_{ 0 };
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_CONNECTION_POOL_HPP