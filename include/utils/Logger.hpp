#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <fstream>
#include <memory>
#include <shared_mutex>
#include <sstream>

WEBSOCKET_NAMESPACE_BEGIN

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    OFF
};

class Logger {
public:
    static Logger& getInstance();

    void initialize(const std::string& logFile = "", LogLevel level = LogLevel::INFO);
    void setLogLevel(LogLevel level);
    void setLogFile(const std::string& logFile);

    void trace(const std::string& message, const std::string& component = "");
    void debug(const std::string& message, const std::string& component = "");
    void info(const std::string& message, const std::string& component = "");
    void warn(const std::string& message, const std::string& component = "");
    void error(const std::string& message, const std::string& component = "");
    void fatal(const std::string& message, const std::string& component = "");

    void flush();
    void rotateLog();

    bool isEnabled(LogLevel level) const;
    std::string getLogLevelString() const;

private:
    Logger();
    ~Logger();

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

// Macros for convenient logging
#define LOG_TRACE(msg) CppWebSocket::Logger::getInstance().trace(msg, __FUNCTION__)
#define LOG_DEBUG(msg) CppWebSocket::Logger::getInstance().debug(msg, __FUNCTION__)
#define LOG_INFO(msg) CppWebSocket::Logger::getInstance().info(msg, __FUNCTION__)
#define LOG_WARN(msg) CppWebSocket::Logger::getInstance().warn(msg, __FUNCTION__)
#define LOG_ERROR(msg) CppWebSocket::Logger::getInstance().error(msg, __FUNCTION__)
#define LOG_FATAL(msg) CppWebSocket::Logger::getInstance().fatal(msg, __FUNCTION__)

WEBSOCKET_NAMESPACE_END