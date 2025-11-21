#pragma once

#include "../common/Types.hpp"
#include <atomic>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class RuntimeConfig {
public:
    static RuntimeConfig& getInstance();

    // Server configuration
    int getPort() const;
    void setPort(int port);

    int getThreadPoolSize() const;
    void setThreadPoolSize(int size);

    int getMaxConnections() const;
    void setMaxConnections(int max);

    int getConnectionTimeout() const;
    void setConnectionTimeout(int timeoutMs);

    // Performance configuration
    size_t getBufferSize() const;
    void setBufferSize(size_t size);

    size_t getMaxMessageSize() const;
    void setMaxMessageSize(size_t size);

    bool getCompressionEnabled() const;
    void setCompressionEnabled(bool enabled);

    // Security configuration
    bool getSSLEnabled() const;
    void setSSLEnabled(bool enabled);

    std::string getSSLCertPath() const;
    void setSSLCertPath(const std::string& path);

    std::string getSSLKeyPath() const;
    void setSSLKeyPath(const std::string& path);

    // Logging configuration
    std::string getLogLevel() const;
    void setLogLevel(const std::string& level);

    std::string getLogFile() const;
    void setLogFile(const std::string& file);

    // Hot-reloadable settings
    void applyHotConfig(const std::unordered_map<std::string, std::any>& newConfig);

private:
    RuntimeConfig() = default;

    mutable std::shared_mutex mutex_;

    std::atomic<int> port_{ 8080 };
    std::atomic<int> threadPoolSize_{ 4 };
    std::atomic<int> maxConnections_{ 1000 };
    std::atomic<int> connectionTimeout_{ 30000 };

    std::atomic<size_t> bufferSize_{ 8192 };
    std::atomic<size_t> maxMessageSize_{ 1048576 }; // 1MB
    std::atomic<bool> compressionEnabled_{ false };

    std::atomic<bool> sslEnabled_{ false };
    std::string sslCertPath_;
    std::string sslKeyPath_;

    std::string logLevel_{ "INFO" };
    std::string logFile_{ "websocket_server.log" };
};

WEBSOCKET_NAMESPACE_END