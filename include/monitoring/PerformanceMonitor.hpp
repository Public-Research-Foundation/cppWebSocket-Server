#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "../utils/Metrics.hpp"
#include <unordered_map>
#include <vector>
#include <atomic>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

struct PerformanceMetrics {
    // Connection metrics
    uint64_t activeConnections;
    uint64_t totalConnections;
    uint64_t connectionErrors;
    double connectionRate; // connections per second

    // Message metrics
    uint64_t messagesReceived;
    uint64_t messagesSent;
    uint64_t messagesDropped;
    double messageRate; // messages per second
    double averageMessageSize;

    // Memory metrics
    size_t memoryUsed;
    size_t memoryAllocated;
    size_t peakMemoryUsed;
    double memoryUsagePercent;

    // CPU metrics
    double cpuUsagePercent;
    double systemCpuUsagePercent;

    // Network metrics
    uint64_t bytesReceived;
    uint64_t bytesSent;
    double bandwidthIn; // bytes per second
    double bandwidthOut; // bytes per second

    // Queue metrics
    uint64_t queueSize;
    uint64_t maxQueueSize;
    double queueUtilizationPercent;

    // Latency metrics
    double averageProcessingTime;
    double p95ProcessingTime;
    double p99ProcessingTime;
    double maxProcessingTime;

    // Error metrics
    uint64_t totalErrors;
    uint64_t errorRate; // errors per second
    std::unordered_map<std::string, uint64_t> errorTypes;

    // Timestamp
    std::chrono::steady_clock::time_point timestamp;
};

class PerformanceMonitor {
public:
    static PerformanceMonitor& getInstance();

    // Monitoring control
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

    // Metrics collection
    void collectSystemMetrics();
    void collectApplicationMetrics();
    void collectNetworkMetrics();
    void collectCustomMetrics(const std::string& category,
        const std::unordered_map<std::string, uint64_t>& metrics);

    // Metrics access
    PerformanceMetrics getCurrentMetrics() const;
    PerformanceMetrics getMetricsSnapshot() const;
    std::vector<PerformanceMetrics> getHistoricalMetrics(uint32_t durationSeconds = 300) const; // 5 minutes default

    // Threshold configuration
    void setWarningThreshold(const std::string& metric, double threshold);
    void setCriticalThreshold(const std::string& metric, double threshold);
    void setThreshold(const std::string& metric, double warning, double critical);

    // Alert configuration
    void enableAlert(const std::string& metric, bool enabled = true);
    void disableAlert(const std::string& metric);
    void setAlertCooldown(const std::string& metric, uint32_t cooldownMs);

    // Performance analysis
    bool isMetricExceedingThreshold(const std::string& metric) const;
    bool isMetricInWarningState(const std::string& metric) const;
    bool isMetricInCriticalState(const std::string& metric) const;
    std::vector<std::string> getExceedingMetrics() const;

    // Trend analysis
    double getMetricTrend(const std::string& metric, uint32_t durationSeconds = 60) const;
    bool isMetricImproving(const std::string& metric, uint32_t durationSeconds = 60) const;
    bool isMetricDegrading(const std::string& metric, uint32_t durationSeconds = 60) const;

    // Resource utilization
    double getCpuUtilization() const;
    double getMemoryUtilization() const;
    double getNetworkUtilization() const;
    double getQueueUtilization() const;

    // Performance recommendations
    std::vector<std::string> getPerformanceRecommendations() const;
    std::vector<std::string> getOptimizationSuggestions() const;

    // Statistics
    struct MonitorStats {
        uint64_t collectionCycles;
        uint64_t alertsTriggered;
        uint64_t thresholdViolations;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::duration<double> totalUptime;
    };

    MonitorStats getStats() const;

    // Reporting
    std::string generatePerformanceReport() const;
    std::unordered_map<std::string, std::any> getPerformanceSummary() const;

private:
    PerformanceMonitor();
    ~PerformanceMonitor();

    WEBSOCKET_DISABLE_COPY(PerformanceMonitor)

        struct MetricThreshold {
        double warning;
        double critical;
        bool alertEnabled;
        uint32_t cooldownMs;
        std::chrono::steady_clock::time_point lastAlertTime;
    };

    mutable std::shared_mutex mutex_;
    mutable std::shared_mutex historyMutex_;

    std::atomic<bool> monitoring_{ false };
    std::thread monitoringThread_;
    uint32_t collectionIntervalMs_{ 5000 }; // 5 seconds default

    PerformanceMetrics currentMetrics_;
    std::vector<PerformanceMetrics> historicalMetrics_;
    size_t maxHistorySize_{ 3600 }; // 1 hour at 1-second intervals

    std::unordered_map<std::string, MetricThreshold> thresholds_;
    std::unordered_map<std::string, std::vector<uint64_t>> metricHistory_;

    std::atomic<uint64_t> collectionCycles_{ 0 };
    std::atomic<uint64_t> alertsTriggered_{ 0 };
    std::atomic<uint64_t> thresholdViolations_{ 0 };
    std::chrono::steady_clock::time_point startTime_;

    void monitoringThreadFunction();
    void collectAllMetrics();
    void updateHistoricalMetrics(const PerformanceMetrics& metrics);
    void checkThresholds(const PerformanceMetrics& metrics);
    void triggerAlert(const std::string& metric, double value, const std::string& level);

    // System metrics collection (platform-specific)
    double collectCpuUsage();
    double collectMemoryUsage();
    void collectNetworkStats();

    // Metric calculations
    double calculateRate(uint64_t current, uint64_t previous, std::chrono::milliseconds interval) const;
    double calculatePercentile(const std::vector<double>& values, double percentile) const;
    double calculateTrend(const std::vector<uint64_t>& values) const;

    void initializeDefaultThresholds();
    bool shouldTriggerAlert(const std::string& metric, const MetricThreshold& threshold) const;
};

WEBSOCKET_NAMESPACE_END