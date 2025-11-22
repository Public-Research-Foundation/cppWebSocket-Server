#include "config/RuntimeConfig.hpp"
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

RuntimeConfig& RuntimeConfig::getInstance() {
    static RuntimeConfig instance;
    return instance;
}

// Server configuration getters/setters
int RuntimeConfig::getPort() const {
    std::shared_lock lock(mutex_);
    return port_;
}

void RuntimeConfig::setPort(int port) {
    std::unique_lock lock(mutex_);
    port_ = port;
}

int RuntimeConfig::getThreadPoolSize() const {
    return threadPoolSize_;
}

void RuntimeConfig::setThreadPoolSize(int size) {
    threadPoolSize_ = size;
}

int RuntimeConfig::getMaxConnections() const {
    return maxConnections_;
}

void RuntimeConfig::setMaxConnections(int max) {
    maxConnections_ = max;
}

int RuntimeConfig::getConnectionTimeout() const {
    return connectionTimeout_;
}

void RuntimeConfig::setConnectionTimeout(int timeoutMs) {
    connectionTimeout_ = timeoutMs;
}

// Performance configuration
size_t RuntimeConfig::getBufferSize() const {
    return bufferSize_;
}

void RuntimeConfig::setBufferSize(size_t size) {
    bufferSize_ = size;
}

size_t RuntimeConfig::getMaxMessageSize() const {
    return maxMessageSize_;
}

void RuntimeConfig::setMaxMessageSize(size_t size) {
    maxMessageSize_ = size;
}

bool RuntimeConfig::getCompressionEnabled() const {
    return compressionEnabled_;
}

void RuntimeConfig::setCompressionEnabled(bool enabled) {
    compressionEnabled_ = enabled;
}

// Security configuration
bool RuntimeConfig::getSSLEnabled() const {
    return sslEnabled_;
}

void RuntimeConfig::setSSLEnabled(bool enabled) {
    sslEnabled_ = enabled;
}

std::string RuntimeConfig::getSSLCertPath() const {
    std::shared_lock lock(mutex_);
    return sslCertPath_;
}

void RuntimeConfig::setSSLCertPath(const std::string& path) {
    std::unique_lock lock(mutex_);
    sslCertPath_ = path;
}

std::string RuntimeConfig::getSSLKeyPath() const {
    std::shared_lock lock(mutex_);
    return sslKeyPath_;
}

void RuntimeConfig::setSSLKeyPath(const std::string& path) {
    std::unique_lock lock(mutex_);
    sslKeyPath_ = path;
}

// Logging configuration
std::string RuntimeConfig::getLogLevel() const {
    std::shared_lock lock(mutex_);
    return logLevel_;
}

void RuntimeConfig::setLogLevel(const std::string& level) {
    std::unique_lock lock(mutex_);
    logLevel_ = level;
}

std::string RuntimeConfig::getLogFile() const {
    std::shared_lock lock(mutex_);
    return logFile_;
}

void RuntimeConfig::setLogFile(const std::string& file) {
    std::unique_lock lock(mutex_);
    logFile_ = file;
}

void RuntimeConfig::applyHotConfig(const std::unordered_map<std::string, std::any>& newConfig) {
    std::unique_lock lock(mutex_);

    for (const auto& [key, value] : newConfig) {
        if (key == "server.port" && value.type() == typeid(int)) {
            port_ = std::any_cast<int>(value);
        }
        else if (key == "server.threads" && value.type() == typeid(int)) {
            threadPoolSize_ = std::any_cast<int>(value);
        }
        else if (key == "server.max_connections" && value.type() == typeid(int)) {
            maxConnections_ = std::any_cast<int>(value);
        }
        else if (key == "performance.buffer_size" && value.type() == typeid(size_t)) {
            bufferSize_ = std::any_cast<size_t>(value);
        }
        else if (key == "logging.level" && value.type() == typeid(std::string)) {
            logLevel_ = std::any_cast<std::string>(value);
        }
        // Add more configuration keys as needed
    }
}

WEBSOCKET_NAMESPACE_END