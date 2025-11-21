#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <fstream>
#include <memory>
#include <shared_mutex>
#include <sstream>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Log levels for granular control over log output
 * TRACE: Very detailed debugging information
 * DEBUG: Debugging information
 * INFO: General operational information
 * WARN: Warning messages that don't stop execution
 * ERROR: Error messages that might affect functionality
 * FATAL: Critical errors that cause system failure
 * OFF: Disable all logging
 */
    enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    OFF
};

/**
 * Thread-safe logging system with file rotation and multiple output destinations
 * Features:
 * - Multiple log levels with runtime configuration
 * - File and console output
 * - Automatic log rotation
 * - Thread-safe operations
 * - Structured logging support
 */
class Logger {
public:
    static Logger& getInstance();

    /**
     * Initialize logger with configuration
     * @param logFile Path to log file (empty for console only)
     * @param level Minimum log level to output
     */
    void initialize(const std::string& logFile = "", LogLevel level = LogLevel::INFO);

    /**
     * Set the minimum log level for output
     * @param level Messages below this level will be filtered out
     */
    void setLogLevel(LogLevel level);

    /**
     * Change log file at runtime
     * @param logFile New log file path
     */
    void setLogFile(const std::string& logFile);

    // Logging methods with component context
    void trace(const std::string& message, const std::string& component = "");
    void debug(const std::string& message, const std::string& component = "");
    void info(const std::string& message, const std::string& component = "");
    void warn(const std::string& message, const std::string& component = "");
    void error(const std::string& message, const std::string& component = "");
    void fatal(const std::string& message, const std::string& component = "");

    /**
     * Flush any buffered log entries to output
     */
    void flush();

    /**
     * Rotate log file (for log rotation systems)
     * Creates new file and archives old one
     */
    void rotateLog();

    /**
     * Check if a specific log level is enabled
     * @param level Log level to check
     * @return true if level is enabled for output
     */
    bool isEnabled(LogLevel level) const;

    std::string getLogLevelString() const;

private:
    Logger();
    ~Logger();

    // Internal logging implementation
    void log(LogLevel level, const std::string& message, const std::string& component);
    std::string getTimestamp() const;
    std::string levelToString(LogLevel level) const;

    mutable std::shared_mutex mutex_;
    std::ofstream logFile_;
    std::string logFilePath_;
    std::atomic<LogLevel> currentLevel_{ LogLevel::INFO };
    std::atomic<bool> consoleOutput_{ true };
    std::atomic<bool> fileOutput_{ false };
    std::atomic<size_t> maxFileSize_{ 10485760 }; // 10MB
    std::atomic<size_t> currentFileSize_{ 0 };
};

/**
 * Convenience macros for logging with function context
 * Automatically includes function name as component
 */
#define LOG_TRACE(msg) CppWebSocket::Logger::getInstance().trace(msg, __FUNCTION__)
#define LOG_DEBUG(msg) CppWebSocket::Logger::getInstance().debug(msg, __FUNCTION__)
#define LOG_INFO(msg) CppWebSocket::Logger::getInstance().info(msg, __FUNCTION__)
#define LOG_WARN(msg) CppWebSocket::Logger::getInstance().warn(msg, __FUNCTION__)
#define LOG_ERROR(msg) CppWebSocket::Logger::getInstance().error(msg, __FUNCTION__)
#define LOG_FATAL(msg) CppWebSocket::Logger::getInstance().fatal(msg, __FUNCTION__)

WEBSOCKET_NAMESPACE_END