#pragma once
#ifndef WEBSOCKET_LOGGER_HPP
#define WEBSOCKET_LOGGER_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <fstream>
#include <memory>
#include <shared_mutex>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @enum LogLevel
 * @brief Log levels for granular control over log output
 *
 * TRACE: Very detailed debugging information
 * DEBUG: Debugging information
 * INFO: General operational information
 * WARN: Warning messages that don't stop execution
 * ERROR: Error messages that might affect functionality
 * FATAL: Critical errors that cause system failure
 * OFF: Disable all logging
 */
    enum class LogLevel {
    TRACE,      ///< Very detailed debugging information
    DEBUG,      ///< Debugging information
    INFO,       ///< General operational information
    WARN,       ///< Warning messages
    ERROR,      ///< Error messages
    FATAL,      ///< Critical errors
    OFF         ///< Disable all logging
};

/**
 * @class Logger
 * @brief Thread-safe logging system with file rotation and multiple output destinations
 *
 * Features:
 * - Multiple log levels with runtime configuration
 * - File and console output
 * - Automatic log rotation
 * - Thread-safe operations
 * - Structured logging support
 * - Performance monitoring
 */
class Logger {
public:
    /**
     * @brief Logger configuration structure
     */
    struct Config {
        std::string logFile;              ///< Path to log file (empty for console only)
        LogLevel level{ LogLevel::INFO };   ///< Minimum log level to output
        bool consoleOutput{ true };         ///< Enable console output
        bool fileOutput{ false };           ///< Enable file output
        size_t maxFileSize{ 10485760 };     ///< Maximum file size before rotation (10MB)
        size_t maxBackupFiles{ 5 };         ///< Maximum number of backup files to keep
        bool timestamp{ true };             ///< Include timestamps in log output
        bool coloredOutput{ true };         ///< Use colored output in console
    };

    /**
     * @brief Get singleton logger instance
     * @return Reference to logger instance
     */
    static Logger& getInstance();

    // Delete copy and move operations
    WEBSOCKET_DISABLE_COPY(Logger)
        WEBSOCKET_DISABLE_MOVE(Logger)

        /**
         * @brief Initialize logger with configuration
         * @param config Logger configuration
         * @return true if initialization successful
         */
        bool initialize(const Config& config);

    /**
     * @brief Initialize logger with basic parameters
     * @param logFile Path to log file (empty for console only)
     * @param level Minimum log level to output
     * @return true if initialization successful
     */
    bool initialize(const std::string& logFile = "", LogLevel level = LogLevel::INFO);

    /**
     * @brief Set the minimum log level for output
     * @param level Messages below this level will be filtered out
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Get current log level
     * @return Current log level
     */
    LogLevel getLogLevel() const;

    /**
     * @brief Change log file at runtime
     * @param logFile New log file path
     * @return true if file change successful
     */
    bool setLogFile(const std::string& logFile);

    // ===== LOGGING METHODS =====

    /**
     * @brief Log trace message
     * @param message Message to log
     * @param component Component name for context
     */
    void trace(const std::string& message, const std::string& component = "");

    /**
     * @brief Log debug message
     * @param message Message to log
     * @param component Component name for context
     */
    void debug(const std::string& message, const std::string& component = "");

    /**
     * @brief Log info message
     * @param message Message to log
     * @param component Component name for context
     */
    void info(const std::string& message, const std::string& component = "");

    /**
     * @brief Log warning message
     * @param message Message to log
     * @param component Component name for context
     */
    void warn(const std::string& message, const std::string& component = "");

    /**
     * @brief Log error message
     * @param message Message to log
     * @param component Component name for context
     */
    void error(const std::string& message, const std::string& component = "");

    /**
     * @brief Log fatal message
     * @param message Message to log
     * @param component Component name for context
     */
    void fatal(const std::string& message, const std::string& component = "");

    /**
     * @brief Flush any buffered log entries to output
     */
    void flush();

    /**
     * @brief Rotate log file (for log rotation systems)
     * Creates new file and archives old one
     * @return true if rotation successful
     */
    bool rotateLog();

    /**
     * @brief Check if a specific log level is enabled
     * @param level Log level to check
     * @return true if level is enabled for output
     */
    bool isEnabled(LogLevel level) const;

    /**
     * @brief Get log level as string
     * @param level Log level to convert
     * @return String representation of log level
     */
    static std::string levelToString(LogLevel level);

    /**
     * @brief Get current configuration
     * @return Current logger configuration
     */
    Config getConfig() const;

    /**
     * @brief Update logger configuration
     * @param config New configuration
     * @return true if configuration applied successfully
     */
    bool setConfig(const Config& config);

private:
    /**
     * @brief Private constructor for singleton
     */
    Logger();

    /**
     * @brief Destructor
     */
    ~Logger();

    /**
     * @brief Internal logging implementation
     * @param level Log level
     * @param message Message to log
     * @param component Component name
     */
    void log(LogLevel level, const std::string& message, const std::string& component);

    /**
     * @brief Get current timestamp as string
     * @return Timestamp string
     */
    std::string getTimestamp() const;

    /**
     * @brief Get color code for log level (console output)
     * @param level Log level
     * @return ANSI color code string
     */
    std::string getLevelColor(LogLevel level) const;

    /**
     * @brief Format log message with all components
     * @param level Log level
     * @param message Message text
     * @param component Component name
     * @return Formatted log message
     */
    std::string formatMessage(LogLevel level, const std::string& message, const std::string& component) const;

    /**
     * @brief Check if log file needs rotation
     * @return true if rotation needed
     */
    bool needsRotation() const;

    /**
     * @brief Perform actual log file rotation
     * @return true if rotation successful
     */
    bool performRotation();

    // Member variables
    mutable std::shared_mutex mutex_;
    std::ofstream logFile_;
    Config config_;
    std::atomic<size_t> currentFileSize_{ 0 };
    std::atomic<bool> initialized_{ false };

    // Singleton instance
    static std::unique_ptr<Logger> instance_;
    static std::once_flag initFlag_;
};

/**
 * @brief Convenience macros for logging with function context
 * Automatically includes function name as component
 */
#define LOG_TRACE(msg) CppWebSocket::Logger::getInstance().trace(msg, __FUNCTION__)
#define LOG_DEBUG(msg) CppWebSocket::Logger::getInstance().debug(msg, __FUNCTION__)
#define LOG_INFO(msg) CppWebSocket::Logger::getInstance().info(msg, __FUNCTION__)
#define LOG_WARN(msg) CppWebSocket::Logger::getInstance().warn(msg, __FUNCTION__)
#define LOG_ERROR(msg) CppWebSocket::Logger::getInstance().error(msg, __FUNCTION__)
#define LOG_FATAL(msg) CppWebSocket::Logger::getInstance().fatal(msg, __FUNCTION__)

 /**
  * @brief Macros with custom component context
  */
#define LOG_TRACE_C(msg, comp) CppWebSocket::Logger::getInstance().trace(msg, comp)
#define LOG_DEBUG_C(msg, comp) CppWebSocket::Logger::getInstance().debug(msg, comp)
#define LOG_INFO_C(msg, comp) CppWebSocket::Logger::getInstance().info(msg, comp)
#define LOG_WARN_C(msg, comp) CppWebSocket::Logger::getInstance().warn(msg, comp)
#define LOG_ERROR_C(msg, comp) CppWebSocket::Logger::getInstance().error(msg, comp)
#define LOG_FATAL_C(msg, comp) CppWebSocket::Logger::getInstance().fatal(msg, comp)

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_LOGGER_HPP