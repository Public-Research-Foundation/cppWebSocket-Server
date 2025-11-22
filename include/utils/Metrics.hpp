#pragma once
#ifndef WEBSOCKET_METRICS_HPP
#define WEBSOCKET_METRICS_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <atomic>
#include <unordered_map>
#include <string>
#include <chrono>
#include <shared_mutex>
#include <sstream>
#include <iomanip>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class Metrics
 * @brief Comprehensive metrics collection system for performance monitoring
 *
 * Supports counters, gauges, timers, and throughput metrics.
 * Thread-safe and optimized for high-frequency updates.
 *
 * Features:
 * - Counter metrics for event counting
 * - Gauge metrics for current values
 * - Timer metrics for duration measurements
 * - Throughput metrics for rate calculations
 * - System resource monitoring
 * - Multiple export formats (Prometheus, JSON)
 */
    class Metrics {
    public:
        /**
         * @brief Timer statistics structure
         */
        struct TimerStats {
            std::atomic<int64_t> count{ 0 };           ///< Number of measurements
            std::atomic<int64_t> total_ns{ 0 };        ///< Total duration in nanoseconds
            std::atomic<int64_t> min_ns{ 0 };          ///< Minimum duration in nanoseconds
            std::atomic<int64_t> max_ns{ 0 };          ///< Maximum duration in nanoseconds
            std::atomic<int64_t> last_ns{ 0 };         ///< Last measured duration

            /**
             * @brief Get average duration in nanoseconds
             * @return Average duration, or 0 if no measurements
             */
            double average() const {
                auto cnt = count.load();
                return cnt > 0 ? static_cast<double>(total_ns.load()) / cnt : 0.0;
            }
        };

        /**
         * @brief Throughput statistics structure
         */
        struct ThroughputStats {
            std::atomic<int64_t> count{ 0 };           ///< Total events counted
            std::atomic<int64_t> last_count{ 0 };      ///< Count at last reset
            std::chrono::steady_clock::time_point last_reset; ///< Last reset time

            ThroughputStats() : last_reset(std::chrono::steady_clock::now()) {}
        };

        /**
         * @brief RAII timer for automatic duration measurement
         *
         * Usage:
         * {
         *     Metrics::Timer timer("operation_name");
         *     // ... code to measure ...
         * } // Timer automatically records duration when destroyed
         */
        class Timer {
        public:
            /**
             * @brief Start timer for specific metric
             * @param name Metric name for this timer
             */
            explicit Timer(const std::string& name);

            /**
             * @brief Stop timer and record measurement (destructor)
             */
            ~Timer();

            /**
             * @brief Manually stop timer before destruction
             * Useful for conditional early stopping
             */
            void stop();

            // Disable copy and move
            WEBSOCKET_DISABLE_COPY(Timer)
                WEBSOCKET_DISABLE_MOVE(Timer)

        private:
            std::string name_;
            std::chrono::steady_clock::time_point start_;
            bool stopped_{ false };
        };

        /**
         * @brief Get singleton metrics instance
         * @return Reference to metrics collector
         */
        static Metrics& getInstance();

        // Delete copy and move operations
        WEBSOCKET_DISABLE_COPY(Metrics)
            WEBSOCKET_DISABLE_MOVE(Metrics)

            // ===== COUNTER METRICS =====

            /**
             * @brief Increment a counter metric
             * @param name Counter name
             * @param value Amount to increment (default 1)
             */
            void incrementCounter(const std::string& name, int64_t value = 1);

        /**
         * @brief Decrement a counter metric
         * @param name Counter name
         * @param value Amount to decrement (default 1)
         */
        void decrementCounter(const std::string& name, int64_t value = 1);

        /**
         * @brief Set counter to specific value
         * @param name Counter name
         * @param value New counter value
         */
        void setCounter(const std::string& name, int64_t value);

        /**
         * @brief Get current counter value
         * @param name Counter name
         * @return Current counter value, or 0 if not found
         */
        int64_t getCounter(const std::string& name) const;

        // ===== GAUGE METRICS =====

        /**
         * @brief Set gauge value
         * @param name Gauge name
         * @param value New gauge value
         */
        void setGauge(const std::string& name, double value);

        /**
         * @brief Get current gauge value
         * @param name Gauge name
         * @return Current gauge value, or 0.0 if not found
         */
        double getGauge(const std::string& name) const;

        // ===== TIMER METRICS =====

        /**
         * @brief Record timer duration manually
         * @param name Timer name
         * @param duration Measured duration
         */
        void recordTimer(const std::string& name, std::chrono::nanoseconds duration);

        /**
         * @brief Get timer statistics
         * @param name Timer name
         * @return Timer statistics, or empty stats if not found
         */
        TimerStats getTimerStats(const std::string& name) const;

        // ===== THROUGHPUT METRICS =====

        /**
         * @brief Record throughput event
         * @param name Throughput metric name
         * @param count Number of events (default 1)
         */
        void recordThroughput(const std::string& name, size_t count = 1);

        /**
         * @brief Get throughput rate (events per second)
         * @param name Throughput metric name
         * @return Events per second, or 0.0 if not enough data
         */
        double getThroughputRate(const std::string& name) const;

        // ===== SYSTEM METRICS =====

        /**
         * @brief Record current memory usage
         */
        void recordMemoryUsage();

        /**
         * @brief Record current CPU usage
         * @note Platform-specific implementation
         */
        void recordCPUUsage();

        /**
         * @brief Record connection metrics
         * @param activeConnections Number of active connections
         * @param totalConnections Total connections since start
         */
        void recordConnectionMetrics(size_t activeConnections, size_t totalConnections);

        // ===== METRICS MANAGEMENT =====

        /**
         * @brief Reset all metrics
         */
        void resetAll();

        /**
         * @brief Reset specific counter
         * @param name Counter name
         */
        void resetCounter(const std::string& name);

        /**
         * @brief Reset specific gauge
         * @param name Gauge name
         */
        void resetGauge(const std::string& name);

        /**
         * @brief Reset specific timer
         * @param name Timer name
         */
        void resetTimer(const std::string& name);

        /**
         * @brief Reset specific throughput metric
         * @param name Throughput metric name
         */
        void resetThroughput(const std::string& name);

        // ===== EXPORT AND REPORTING =====

        /**
         * @brief Export metrics in Prometheus format
         * @return Metrics in Prometheus text-based format
         */
        std::string exportPrometheusFormat() const;

        /**
         * @brief Export metrics in JSON format
         * @return Metrics as JSON string
         */
        std::string exportJsonFormat() const;

        /**
         * @brief Get all metrics as formatted string for logging
         * @return Human-readable metrics summary
         */
        std::string getFormattedSummary() const;

        /**
         * @brief Get comprehensive metrics snapshot
         * @return Map of all metric names and values
         */
        std::unordered_map<std::string, std::string> getAllMetrics() const;

    private:
        /**
         * @brief Private constructor for singleton
         */
        Metrics();

        /**
         * @brief Destructor
         */
        ~Metrics() = default;

        /**
         * @brief Initialize default metrics
         */
        void initializeDefaultMetrics();

        /**
         * @brief Get current process memory usage in bytes
         * @return Memory usage in bytes, or 0 if unavailable
         */
        size_t getProcessMemoryUsage() const;

        /**
         * @brief Get current process CPU usage percentage
         * @return CPU usage percentage, or 0.0 if unavailable
         */
        double getProcessCPUUsage() const;

        mutable std::shared_mutex mutex_;

        // Counters: for event counting (monotonically increasing)
        std::unordered_map<std::string, std::atomic<int64_t>> counters_;

        // Gauges: for current values (can increase/decrease)
        std::unordered_map<std::string, std::atomic<double>> gauges_;

        // Timers: for duration measurements with statistics
        std::unordered_map<std::string, TimerStats> timers_;

        // Throughput: for rate calculations
        std::unordered_map<std::string, ThroughputStats> throughput_;

        // Singleton instance
        static std::unique_ptr<Metrics> instance_;
        static std::once_flag initFlag_;
};

/**
 * @brief Convenience macros for easy metrics collection
 */

 // Counter macros
#define METRICS_INCREMENT(name) CppWebSocket::Metrics::getInstance().incrementCounter(name)
#define METRICS_INCREMENT_BY(name, value) CppWebSocket::Metrics::getInstance().incrementCounter(name, value)
#define METRICS_DECREMENT(name) CppWebSocket::Metrics::getInstance().decrementCounter(name)
#define METRICS_SET_COUNTER(name, value) CppWebSocket::Metrics::getInstance().setCounter(name, value)

// Gauge macros
#define METRICS_SET_GAUGE(name, value) CppWebSocket::Metrics::getInstance().setGauge(name, value)

// Timer macros
#define METRICS_TIMER(name) CppWebSocket::Metrics::Timer timer_##__LINE__(name)
#define METRICS_RECORD_TIMER(name, duration) CppWebSocket::Metrics::getInstance().recordTimer(name, duration)

// Throughput macros
#define METRICS_RECORD_THROUGHPUT(name) CppWebSocket::Metrics::getInstance().recordThroughput(name)
#define METRICS_RECORD_THROUGHPUT_BY(name, count) CppWebSocket::Metrics::getInstance().recordThroughput(name, count)

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_METRICS_HPP