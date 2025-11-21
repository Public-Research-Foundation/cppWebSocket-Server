#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <atomic>
#include <unordered_map>
#include <string>
#include <chrono>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Comprehensive metrics collection system for performance monitoring
 * Supports counters, gauges, timers, and throughput metrics
 * Thread-safe and optimized for high-frequency updates
 */
    class MetricsCollector {
    public:
        static MetricsCollector& getInstance();

        // ===== COUNTER METRICS =====
        // For counting events (monotonically increasing)

        /**
         * Increment a counter metric
         * @param name Counter name
         * @param value Amount to increment (default 1)
         */
        void incrementCounter(const std::string& name, int64_t value = 1);

        /**
         * Decrement a counter metric
         * @param name Counter name
         * @param value Amount to decrement (default 1)
         */
        void decrementCounter(const std::string& name, int64_t value = 1);

        /**
         * Set counter to specific value
         * @param name Counter name
         * @param value New counter value
         */
        void setCounter(const std::string& name, int64_t value);
        int64_t getCounter(const std::string& name) const;

        // ===== GAUGE METRICS =====
        // For values that can go up and down (like memory usage)

        void setGauge(const std::string& name, double value);
        double getGauge(const std::string& name) const;

        // ===== TIMER METRICS =====
        // For measuring operation durations

        /**
         * RAII timer that automatically records duration when destroyed
         * Usage: { METRICS_TIMER("operation_name"); ... code ... }
         */
        class Timer {
        public:
            Timer(const std::string& name);
            ~Timer();

            /**
             * Manually stop timer before destruction
             * Useful for conditional early stopping
             */
            void stop();

        private:
            std::string name_;
            std::chrono::steady_clock::time_point start_;
            bool stopped_{ false };
        };

        /**
         * Record timer duration manually
         * @param name Timer name
         * @param duration Measured duration
         */
        void recordTimer(const std::string& name, std::chrono::nanoseconds duration);

        // ===== THROUGHPUT METRICS =====
        // For measuring rates (operations per second)

        void recordThroughput(const std::string& name, size_t count);

        // ===== SYSTEM METRICS =====
        // Automatic system resource monitoring

        void recordMemoryUsage();
        void recordCPUUsage();

        // ===== REPORTING =====
        // Export metrics for monitoring systems

        std::unordered_map<std::string, std::string> getFormattedMetrics() const;
        void resetAll();
        void resetCounter(const std::string& name);
        void resetGauge(const std::string& name);

        /**
         * Export metrics in Prometheus format
         * @return Metrics in Prometheus text-based format
         */
        std::string exportPrometheusFormat() const;

        /**
         * Export metrics in JSON format
         * @return Metrics as JSON string
         */
        std::string exportJsonFormat() const;

    private:
        MetricsCollector();
        ~MetricsCollector() = default;

        mutable std::shared_mutex mutex_;

        // Counters: for event counting (monotonically increasing)
        std::unordered_map<std::string, std::atomic<int64_t>> counters_;

        // Gauges: for current values (can increase/decrease)
        std::unordered_map<std::string, std::atomic<double>> gauges_;

        // Timers: for duration measurements with statistics
        struct TimerStats {
            std::atomic<int64_t> count{ 0 };
            std::atomic<int64_t> total_ns{ 0 };
            std::atomic<int64_t> min_ns{ 0 };
            std::atomic<int64_t> max_ns{ 0 };
        };
        std::unordered_map<std::string, TimerStats> timers_;

        // Throughput: for rate calculations
        std::unordered_map<std::string, std::atomic<int64_t>> throughput_;

        void initializeDefaultMetrics();
};

/**
 * Convenience macros for easy metrics collection
 */
#define METRICS_INCREMENT(name) CppWebSocket::MetricsCollector::getInstance().incrementCounter(name)
#define METRICS_TIMER(name) CppWebSocket::MetricsCollector::Timer timer_##__LINE__(name)

WEBSOCKET_NAMESPACE_END