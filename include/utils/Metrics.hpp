#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <atomic>
#include <unordered_map>
#include <string>
#include <chrono>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class MetricsCollector {
public:
    static MetricsCollector& getInstance();

    // Counter metrics
    void incrementCounter(const std::string& name, int64_t value = 1);
    void decrementCounter(const std::string& name, int64_t value = 1);
    void setCounter(const std::string& name, int64_t value);
    int64_t getCounter(const std::string& name) const;

    // Gauge metrics
    void setGauge(const std::string& name, double value);
    double getGauge(const std::string& name) const;

    // Timer metrics
    class Timer {
    public:
        Timer(const std::string& name);
        ~Timer();

        void stop();

    private:
        std::string name_;
        std::chrono::steady_clock::time_point start_;
        bool stopped_{ false };
    };

    void recordTimer(const std::string& name, std::chrono::nanoseconds duration);

    // Throughput metrics
    void recordThroughput(const std::string& name, size_t count);

    // System metrics
    void recordMemoryUsage();
    void recordCPUUsage();

    // Reporting
    std::unordered_map<std::string, std::string> getFormattedMetrics() const;
    void resetAll();
    void resetCounter(const std::string& name);
    void resetGauge(const std::string& name);

    // Export for monitoring systems
    std::string exportPrometheusFormat() const;
    std::string exportJsonFormat() const;

private:
    MetricsCollector();
    ~MetricsCollector() = default;

    mutable std::shared_mutex mutex_;

    // Counters
    std::unordered_map<std::string, std::atomic<int64_t>> counters_;

    // Gauges
    std::unordered_map<std::string, std::atomic<double>> gauges_;

    // Timers
    struct TimerStats {
        std::atomic<int64_t> count{ 0 };
        std::atomic<int64_t> total_ns{ 0 };
        std::atomic<int64_t> min_ns{ 0 };
        std::atomic<int64_t> max_ns{ 0 };
    };
    std::unordered_map<std::string, TimerStats> timers_;

    // Throughput
    std::unordered_map<std::string, std::atomic<int64_t>> throughput_;

    void initializeDefaultMetrics();
};

// Macros for easy metrics collection
#define METRICS_INCREMENT(name) CppWebSocket::MetricsCollector::getInstance().incrementCounter(name)
#define METRICS_TIMER(name) CppWebSocket::MetricsCollector::Timer timer_##__LINE__(name)

WEBSOCKET_NAMESPACE_END