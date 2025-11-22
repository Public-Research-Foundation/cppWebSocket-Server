#pragma once
#ifndef WEBSOCKET_RUNTIME_CONFIG_HPP
#define WEBSOCKET_RUNTIME_CONFIG_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <atomic>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <any>
#include <functional>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class RuntimeConfig
 * @brief Thread-safe runtime configuration with hot-reload support
 *
 * Provides atomic access to frequently-used configuration values
 * with minimal locking overhead. Optimized for high-frequency access
 * patterns in the WebSocket server.
 */
    class RuntimeConfig {
    public:
        using ChangeCallback = std::function<void(const std::string& key, const std::any& oldValue, const std::any& newValue)>;

        /**
         * @brief Get singleton instance
         * @return Reference to runtime configuration instance
         */
        static RuntimeConfig& getInstance();

        // Delete copy and move operations
        WEBSOCKET_DISABLE_COPY(RuntimeConfig)
            WEBSOCKET_DISABLE_MOVE(RuntimeConfig)

            // ============================================================================
            // SERVER CONFIGURATION
            // ============================================================================

            /**
             * @brief Get server port
             * @return Server port number
             */
            uint16_t getPort() const;

        /**
         * @brief Set server port
         * @param port Port number (1-65535)
         */
        void setPort(uint16_t port);

        /**
         * @brief Get thread pool size
         * @return Number of threads in pool (0 = auto-detect)
         */
        size_t getThreadPoolSize() const;

        /**
         * @brief Set thread pool size
         * @param size Thread pool size (0 for auto-detect)
         */
        void setThreadPoolSize(size_t size);

        /**
         * @brief Get maximum connections
         * @return Maximum concurrent connections
         */
        size_t getMaxConnections() const;

        /**
         * @brief Set maximum connections
         * @param max Maximum concurrent connections
         */
        void setMaxConnections(size_t max);

        /**
         * @brief Get connection timeout
         * @return Connection timeout in milliseconds
         */
        uint32_t getConnectionTimeout() const;

        /**
         * @brief Set connection timeout
         * @param timeoutMs Timeout in milliseconds
         */
        void setConnectionTimeout(uint32_t timeoutMs);

        // ============================================================================
        // PERFORMANCE CONFIGURATION
        // ============================================================================

        /**
         * @brief Get buffer size
         * @return I/O buffer size in bytes
         */
        size_t getBufferSize() const;

        /**
         * @brief Set buffer size
         * @param size Buffer size in bytes
         */
        void setBufferSize(size_t size);

        /**
         * @brief Get maximum message size
         * @return Maximum WebSocket message size in bytes
         */
        size_t getMaxMessageSize() const;

        /**
         * @brief Set maximum message size
         * @param size Maximum message size in bytes
         */
        void setMaxMessageSize(size_t size);

        /**
         * @brief Check if compression is enabled
         * @return true if message compression is enabled
         */
        bool getCompressionEnabled() const;

        /**
         * @brief Set compression enabled
         * @param enabled true to enable message compression
         */
        void setCompressionEnabled(bool enabled);

        // ============================================================================
        // SECURITY CONFIGURATION
        // ============================================================================

        /**
         * @brief Check if SSL/TLS is enabled
         * @return true if SSL is enabled
         */
        bool getSSLEnabled() const;

        /**
         * @brief Set SSL enabled
         * @param enabled true to enable SSL/TLS
         */
        void setSSLEnabled(bool enabled);

        /**
         * @brief Get SSL certificate path
         * @return Path to SSL certificate file
         */
        std::string getSSLCertPath() const;

        /**
         * @brief Set SSL certificate path
         * @param path Path to certificate file
         */
        void setSSLCertPath(const std::string& path);

        /**
         * @brief Get SSL private key path
         * @return Path to SSL private key file
         */
        std::string getSSLKeyPath() const;

        /**
         * @brief Set SSL private key path
         * @param path Path to private key file
         */
        void setSSLKeyPath(const std::string& path);

        // ============================================================================
        // LOGGING CONFIGURATION
        // ============================================================================

        /**
         * @brief Get log level
         * @return Current log level ("TRACE", "DEBUG", "INFO", "WARN", "ERROR")
         */
        std::string getLogLevel() const;

        /**
         * @brief Set log level
         * @param level Log level string
         */
        void setLogLevel(const std::string& level);

        /**
         * @brief Get log file path
         * @return Path to log file
         */
        std::string getLogFile() const;

        /**
         * @brief Set log file path
         * @param file Log file path
         */
        void setLogFile(const std::string& file);

        // ============================================================================
        // HOT-RELOAD SUPPORT
        // ============================================================================

        /**
         * @brief Apply hot-reloaded configuration
         * @param newConfig New configuration values
         * @return Number of successfully applied changes
         */
        size_t applyHotConfig(const std::unordered_map<std::string, std::any>& newConfig);

        /**
         * @brief Add change listener for configuration key
         * @param key Configuration key to monitor
         * @param callback Function to call when value changes
         */
        void addChangeListener(const std::string& key, ChangeCallback callback);

        /**
         * @brief Remove change listener
         * @param key Configuration key
         * @param callback Callback to remove
         */
        void removeChangeListener(const std::string& key, ChangeCallback callback);

        /**
         * @brief Check if configuration has been modified
         * @return true if any configuration value has changed
         */
        bool isDirty() const;

        /**
         * @brief Reset dirty flag
         */
        void clearDirtyFlag();

        /**
         * @brief Get all current configuration values
         * @return Map of all configuration values
         */
        std::unordered_map<std::string, std::any> getAllValues() const;

    private:
        /**
         * @brief Private constructor for singleton
         */
        RuntimeConfig();

        /**
         * @brief Private destructor
         */
        ~RuntimeConfig() = default;

        /**
         * @brief Notify change listeners for a key
         * @param key Configuration key that changed
         * @param oldValue Previous value
         * @param newValue New value
         */
        void notifyChangeListeners(const std::string& key, const std::any& oldValue, const std::any& newValue);

        // Member variables
        mutable std::shared_mutex mutex_;

        // Server configuration (atomic for lock-free reads)
        std::atomic<uint16_t> port_{ 8080 };
        std::atomic<size_t> threadPoolSize_{ 0 }; // 0 = auto-detect
        std::atomic<size_t> maxConnections_{ 10000 };
        std::atomic<uint32_t> connectionTimeout_{ 30000 };

        // Performance configuration
        std::atomic<size_t> bufferSize_{ 8192 };
        std::atomic<size_t> maxMessageSize_{ 16 * 1024 * 1024 }; // 16MB
        std::atomic<bool> compressionEnabled_{ false };

        // Security configuration
        std::atomic<bool> sslEnabled_{ false };
        std::string sslCertPath_;
        std::string sslKeyPath_;

        // Logging configuration
        std::string logLevel_{ "INFO" };
        std::string logFile_{ "websocket_server.log" };

        // Change notification
        std::unordered_map<std::string, std::vector<ChangeCallback>> changeListeners_;
        std::atomic<bool> dirty_{ false };

        // Singleton instance
        static std::unique_ptr<RuntimeConfig> instance_;
        static std::once_flag initFlag_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_RUNTIME_CONFIG_HPP