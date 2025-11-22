#include "monitoring/PerformanceMonitor.hpp"
#include "utils/Logger.hpp"
#include "utils/Metrics.hpp"
#include <thread>
#include <chrono>
#include <algorithm>

WEBSOCKET_NAMESPACE_BEGIN

// Singleton instance access
PerformanceMonitor& PerformanceMonitor::getInstance() {
    static PerformanceMonitor instance;
    return instance;
}

PerformanceMonitor::PerformanceMonitor() {
    initializeDefaultThresholds();
    LOG_DEBUG("PerformanceMonitor created");
}

PerformanceMonitor::~PerformanceMonitor() {
    stopMonitoring();
    LOG_DEBUG("PerformanceMonitor destroyed");
}

// Start performance monitoring
void PerformanceMonitor::startMonitoring() {
    if (monitoring_) {
        LOG_WARN("Performance monitoring already running");
        return;
    }

    monitoring_ = true;
    startTime_ = std::chrono::steady_clock::now();
    monitoringThread_ = std::thread(&PerformanceMonitor::monitoringThreadFunction, this);
    LOG_INFO("Performance monitoring started");
}

// Stop performance monitoring
void PerformanceMonitor::stopMonitoring() {
    if (!monitoring_) {
        return;
    }

    monitoring_ = false;
    if (monitoringThread_.joinable()) {
        monitoringThread_.join();
    }
    LOG_INFO("Performance monitoring stopped");
}

bool PerformanceMonitor::isMonitoring() const {
    return monitoring_;
}

// Collect system-level metrics
void PerformanceMonitor::collectSystemMetrics() {
    PerformanceMetrics metrics;
    metrics.timestamp = std::chrono::steady_clock::now();

    // Connection metrics
    metrics.activeConnections = 0; // Would get from connection manager
    metrics.totalConnections = 0;  // Would get from connection manager
    metrics.connectionErrors = 0;  // Would get from error counters
    metrics.connectionRate = 0.0;  // Would calculate from history

    // Message metrics
    metrics.messagesReceived = 0;  // Would get from message counters
    metrics.messagesSent = 0;      // Would get from message counters
    metrics.messagesDropped = 0;   // Would get from queue statistics
    metrics.messageRate = 0.0;     // Would calculate from history
    metrics.averageMessageSize = 0.0; // Would calculate from message sizes

    // Memory metrics (simplified - in production use system APIs)
    metrics.memoryUsed = 0;
    metrics.memoryAllocated = 0;
    metrics.peakMemoryUsed = 0;
    metrics.memoryUsagePercent = 0.0;

    // CPU metrics (simplified)
    metrics.cpuUsagePercent = collectCpuUsage();
    metrics.systemCpuUsagePercent = 0.0;

    // Network metrics
    metrics.bytesReceived = 0;     // Would get from network counters
    metrics.bytesSent = 0;         // Would get from network counters
    metrics.bandwidthIn = 0.0;     // Would calculate from history
    metrics.bandwidthOut = 0.0;    // Would calculate from history

    // Queue metrics
    metrics.queueSize = 0;         // Would get from message queues
    metrics.maxQueueSize = 0;      // Would get from queue configuration
    metrics.queueUtilizationPercent = 0.0;

    // Latency metrics
    metrics.averageProcessingTime = 0.0;
    metrics.p95ProcessingTime = 0.0;
    metrics.p99ProcessingTime = 0.0;
    metrics.maxProcessingTime = 0.0;

    // Error metrics
    metrics.totalErrors = 0;       // Would get from error counters
    metrics.errorRate = 0.0;       // Would calculate from history
    metrics.errorTypes = {};       // Would get from error categorizer

    {
        std::unique_lock lock(mutex_);
        currentMetrics_ = metrics;
        updateHistoricalMetrics(metrics);
    }

    checkThresholds(metrics);
}

// Collect application-level metrics
void PerformanceMonitor::collectApplicationMetrics() {
    // Collect metrics from various application components
    // This would integrate with the MetricsCollector system

    METRICS_INCREMENT("performance_monitor_collections");
}

// Collect network-specific metrics
void PerformanceMonitor::collectNetworkMetrics() {
    // Collect network-level statistics
    // This would integrate with network layer components
}

// Collect custom metrics from application components
void PerformanceMonitor::collectCustomMetrics(const std::string& category,
    const std::unordered_map<std::string, uint64_t>& metrics) {
    std::unique_lock lock(mutex_);

    for (const auto& [key, value] : metrics) {
        std::string fullKey = category + "." + key;
        metricHistory_[fullKey].push_back(value);

        // Keep history size manageable
        if (metricHistory_[fullKey].size() > maxHistorySize_) {
            metricHistory_[fullKey].erase(metricHistory_[fullKey].begin());
        }
    }
}

// Get current performance metrics
PerformanceMetrics PerformanceMonitor::getCurrentMetrics() const {
    std::shared_lock lock(mutex_);
    return currentMetrics_;
}

// Get a snapshot of current metrics
PerformanceMetrics PerformanceMonitor::getMetricsSnapshot() const {
    return getCurrentMetrics();
}

// Get historical metrics for specified duration
std::vector<PerformanceMetrics> PerformanceMonitor::getHistoricalMetrics(uint32_t durationSeconds) const {
    std::shared_lock lock(historyMutex_);

    std::vector<PerformanceMetrics> result;
    auto cutoffTime = std::chrono::steady_clock::now() - std::chrono::seconds(durationSeconds);

    for (const auto& metrics : historicalMetrics_) {
        if (metrics.timestamp >= cutoffTime) {
            result.push_back(metrics);
        }
    }

    return result;
}

// Set warning threshold for a metric
void PerformanceMonitor::setWarningThreshold(const std::string& metric, double threshold) {
    std::unique_lock lock(mutex_);
    thresholds_[metric].warning = threshold;
}

// Set critical threshold for a metric
void PerformanceMonitor::setCriticalThreshold(const std::string& metric, double threshold) {
    std::unique_lock lock(mutex_);
    thresholds_[metric].critical = threshold;
}

// Set both warning and critical thresholds
void PerformanceMonitor::setThreshold(const std::string& metric, double warning, double critical) {
    std::unique_lock lock(mutex_);
    thresholds_[metric].warning = warning;
    thresholds_[metric].critical = critical;
}

// Enable alerting for a metric
void PerformanceMonitor::enableAlert(const std::string& metric, bool enabled) {
    std::unique_lock lock(mutex_);
    thresholds_[metric].alertEnabled = enabled;
}

// Disable alerting for a metric
void PerformanceMonitor::disableAlert(const std::string& metric) {
    std::unique_lock lock(mutex_);
    thresholds_[metric].alertEnabled = false;
}

// Set alert cooldown period
void PerformanceMonitor::setAlertCooldown(const std::string& metric, uint32_t cooldownMs) {
    std::unique_lock lock(mutex_);
    thresholds_[metric].cooldownMs = cooldownMs;
}

// Check if a metric exceeds its threshold
bool PerformanceMonitor::isMetricExceedingThreshold(const std::string& metric) const {
    return isMetricInWarningState(metric) || isMetricInCriticalState(metric);
}

// Check if a metric is in warning state
bool PerformanceMonitor::isMetricInWarningState(const std::string& metric) const {
    std::shared_lock lock(mutex_);
    auto thresholdIt = thresholds_.find(metric);
    if (thresholdIt == thresholds_.end() || !thresholdIt->second.alertEnabled) {
        return false;
    }

    // Get current value (simplified - would map metric names to actual values)
    double value = 0.0;
    if (metric == "cpu_usage") value = currentMetrics_.cpuUsagePercent;
    else if (metric == "memory_usage") value = currentMetrics_.memoryUsagePercent;
    // Add more metric mappings as needed

    return value >= thresholdIt->second.warning && value < thresholdIt->second.critical;
}

// Check if a metric is in critical state
bool PerformanceMonitor::isMetricInCriticalState(const std::string& metric) const {
    std::shared_lock lock(mutex_);
    auto thresholdIt = thresholds_.find(metric);
    if (thresholdIt == thresholds_.end() || !thresholdIt->second.alertEnabled) {
        return false;
    }

    // Get current value (simplified)
    double value = 0.0;
    if (metric == "cpu_usage") value = currentMetrics_.cpuUsagePercent;
    else if (metric == "memory_usage") value = currentMetrics_.memoryUsagePercent;

    return value >= thresholdIt->second.critical;
}

// Get all metrics currently exceeding thresholds
std::vector<std::string> PerformanceMonitor::getExceedingMetrics() const {
    std::shared_lock lock(mutex_);
    std::vector<std::string> exceeding;

    for (const auto& [metric, threshold] : thresholds_) {
        if (threshold.alertEnabled && isMetricExceedingThreshold(metric)) {
            exceeding.push_back(metric);
        }
    }

    return exceeding;
}

// Calculate trend for a metric over specified duration
double PerformanceMonitor::getMetricTrend(const std::string& metric, uint32_t durationSeconds) const {
    std::shared_lock lock(mutex_);
    auto it = metricHistory_.find(metric);
    if (it == metricHistory_.end() || it->second.size() < 2) {
        return 0.0;
    }

    return calculateTrend(it->second);
}

// Check if a metric is improving (trending downward)
bool PerformanceMonitor::isMetricImproving(const std::string& metric, uint32_t durationSeconds) const {
    return getMetricTrend(metric, durationSeconds) < 0.0;
}

// Check if a metric is degrading (trending upward)
bool PerformanceMonitor::isMetricDegrading(const std::string& metric, uint32_t durationSeconds) const {
    return getMetricTrend(metric, durationSeconds) > 0.0;
}

// Get current CPU utilization
double PerformanceMonitor::getCpuUtilization() const {
    std::shared_lock lock(mutex_);
    return currentMetrics_.cpuUsagePercent;
}

// Get current memory utilization
double PerformanceMonitor::getMemoryUtilization() const {
    std::shared_lock lock(mutex_);
    return currentMetrics_.memoryUsagePercent;
}

// Get current network utilization
double PerformanceMonitor::getNetworkUtilization() const {
    std::shared_lock lock(mutex_);
    return 0.0; // Would calculate from bandwidth metrics
}

// Get current queue utilization
double PerformanceMonitor::getQueueUtilization() const {
    std::shared_lock lock(mutex_);
    return currentMetrics_.queueUtilizationPercent;
}

// Get performance recommendations based on current metrics
std::vector<std::string> PerformanceMonitor::getPerformanceRecommendations() const {
    std::vector<std::string> recommendations;

    if (isMetricInCriticalState("cpu_usage")) {
        recommendations.push_back("CPU usage critically high - consider scaling horizontally");
    }

    if (isMetricInCriticalState("memory_usage")) {
        recommendations.push_back("Memory usage critically high - check for memory leaks");
    }

    if (currentMetrics_.queueUtilizationPercent > 90.0) {
        recommendations.push_back("Message queue nearly full - consider increasing queue size or processing capacity");
    }

    if (currentMetrics_.errorRate > 10.0) {
        recommendations.push_back("High error rate detected - investigate error sources");
    }

    return recommendations;
}

// Get optimization suggestions
std::vector<std::string> PerformanceMonitor::getOptimizationSuggestions() const {
    std::vector<std::string> suggestions;

    if (currentMetrics_.averageProcessingTime > 100.0) { // More than 100ms
        suggestions.push_back("High message processing time - optimize message handlers");
    }

    if (currentMetrics_.bandwidthIn > 100000000) { // More than 100 MB/s
        suggestions.push_back("High inbound bandwidth - consider compression");
    }

    return suggestions;
}

// Get performance monitoring statistics
PerformanceMonitor::MonitorStats PerformanceMonitor::getStats() const {
    MonitorStats stats;
    stats.collectionCycles = collectionCycles_.load();
    stats.alertsTriggered = alertsTriggered_.load();
    stats.thresholdViolations = thresholdViolations_.load();
    stats.startTime = startTime_;
    stats.totalUptime = std::chrono::steady_clock::now() - startTime_;

    return stats;
}

// Generate performance report
std::string PerformanceMonitor::generatePerformanceReport() const {
    std::shared_lock lock(mutex_);

    std::ostringstream report;
    report << "Performance Report\n";
    report << "==================\n\n";

    report << "Current Metrics:\n";
    report << "  CPU Usage: " << currentMetrics_.cpuUsagePercent << "%\n";
    report << "  Memory Usage: " << currentMetrics_.memoryUsagePercent << "%\n";
    report << "  Active Connections: " << currentMetrics_.activeConnections << "\n";
    report << "  Message Rate: " << currentMetrics_.messageRate << " msg/s\n";
    report << "  Error Rate: " << currentMetrics_.errorRate << " errors/s\n";
    report << "  Queue Utilization: " << currentMetrics_.queueUtilizationPercent << "%\n\n";

    report << "Threshold Violations:\n";
    auto exceeding = getExceedingMetrics();
    if (exceeding.empty()) {
        report << "  None\n";
    }
    else {
        for (const auto& metric : exceeding) {
            report << "  " << metric << "\n";
        }
    }
    report << "\n";

    report << "Recommendations:\n";
    auto recommendations = getPerformanceRecommendations();
    if (recommendations.empty()) {
        report << "  None\n";
    }
    else {
        for (const auto& rec : recommendations) {
            report << "  - " << rec << "\n";
        }
    }

    return report.str();
}

// Get performance summary for external systems
std::unordered_map<std::string, std::any> PerformanceMonitor::getPerformanceSummary() const {
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, std::any> summary;
    summary["cpu_usage"] = currentMetrics_.cpuUsagePercent;
    summary["memory_usage"] = currentMetrics_.memoryUsagePercent;
    summary["active_connections"] = currentMetrics_.activeConnections;
    summary["message_rate"] = currentMetrics_.messageRate;
    summary["error_rate"] = currentMetrics_.errorRate;
    summary["queue_utilization"] = currentMetrics_.queueUtilizationPercent;
    summary["collection_cycles"] = collectionCycles_.load();
    summary["alerts_triggered"] = alertsTriggered_.load();

    return summary;
}

// Private methods

// Monitoring thread function
void PerformanceMonitor::monitoringThreadFunction() {
    LOG_DEBUG("Performance monitoring thread started");

    while (monitoring_) {
        auto startTime = std::chrono::steady_clock::now();

        // Collect all metrics
        collectAllMetrics();

        // Calculate sleep time to maintain consistent interval
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto sleepTime = std::chrono::milliseconds(collectionIntervalMs_) - elapsed;

        if (sleepTime.count() > 0) {
            std::this_thread::sleep_for(sleepTime);
        }

        collectionCycles_++;
    }

    LOG_DEBUG("Performance monitoring thread stopped");
}

// Collect all performance metrics
void PerformanceMonitor::collectAllMetrics() {
    collectSystemMetrics();
    collectApplicationMetrics();
    collectNetworkMetrics();
}

// Update historical metrics storage
void PerformanceMonitor::updateHistoricalMetrics(const PerformanceMetrics& metrics) {
    std::unique_lock lock(historyMutex_);

    historicalMetrics_.push_back(metrics);

    // Keep history size manageable
    if (historicalMetrics_.size() > maxHistorySize_) {
        historicalMetrics_.erase(historicalMetrics_.begin());
    }
}

// Check all metrics against their thresholds
void PerformanceMonitor::checkThresholds(const PerformanceMetrics& metrics) {
    std::shared_lock lock(mutex_);

    for (const auto& [metric, threshold] : thresholds_) {
        if (!threshold.alertEnabled) {
            continue;
        }

        // Get current value (simplified mapping)
        double value = 0.0;
        if (metric == "cpu_usage") value = metrics.cpuUsagePercent;
        else if (metric == "memory_usage") value = metrics.memoryUsagePercent;
        else if (metric == "queue_utilization") value = metrics.queueUtilizationPercent;
        else if (metric == "error_rate") value = metrics.errorRate;

        // Check thresholds
        if (value >= threshold.critical) {
            triggerAlert(metric, value, "CRITICAL");
        }
        else if (value >= threshold.warning) {
            triggerAlert(metric, value, "WARNING");
        }
    }
}

// Trigger an alert for a metric exceeding threshold
void PerformanceMonitor::triggerAlert(const std::string& metric, double value, const std::string& level) {
    auto now = std::chrono::steady_clock::now();
    auto& threshold = thresholds_[metric];

    // Check cooldown period
    if (threshold.cooldownMs > 0) {
        auto timeSinceLastAlert = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - threshold.lastAlertTime);
        if (timeSinceLastAlert.count() < threshold.cooldownMs) {
            return; // Still in cooldown period
        }
    }

    threshold.lastAlertTime = now;
    alertsTriggered_++;
    thresholdViolations_++;

    LOG_WARN("Performance alert: {} {} - {} = {}", level, metric, value);

    // In production, this would send to AlertManager
    METRICS_INCREMENT("performance_alerts");
}

// Collect CPU usage (platform-specific)
double PerformanceMonitor::collectCpuUsage() {
    // Simplified implementation - in production use system APIs
    // This would read /proc/stat on Linux or use Performance Counters on Windows
    return 0.0;
}

// Collect memory usage (platform-specific)
double PerformanceMonitor::collectMemoryUsage() {
    // Simplified implementation
    return 0.0;
}

// Collect network statistics
void PerformanceMonitor::collectNetworkStats() {
    // Would collect from network interfaces
}

// Calculate rate from current and previous values
double PerformanceMonitor::calculateRate(uint64_t current, uint64_t previous, std::chrono::milliseconds interval) const {
    if (interval.count() == 0) return 0.0;
    return static_cast<double>(current - previous) / (interval.count() / 1000.0);
}

// Calculate percentile from values
double PerformanceMonitor::calculatePercentile(const std::vector<double>& values, double percentile) const {
    if (values.empty()) return 0.0;

    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());

    size_t index = static_cast<size_t>(percentile * (sorted.size() - 1));
    return sorted[index];
}

// Calculate trend from historical values
double PerformanceMonitor::calculateTrend(const std::vector<uint64_t>& values) const {
    if (values.size() < 2) return 0.0;

    // Simple linear regression for trend
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    size_t n = values.size();

    for (size_t i = 0; i < n; ++i) {
        sumX += i;
        sumY += values[i];
        sumXY += i * values[i];
        sumX2 += i * i;
    }

    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    return slope;
}

// Initialize default thresholds
void PerformanceMonitor::initializeDefaultThresholds() {
    setThreshold("cpu_usage", 80.0, 95.0);
    setThreshold("memory_usage", 85.0, 95.0);
    setThreshold("queue_utilization", 80.0, 95.0);
    setThreshold("error_rate", 5.0, 10.0);

    // Enable alerts for critical metrics
    enableAlert("cpu_usage");
    enableAlert("memory_usage");
    enableAlert("error_rate");
}

// Check if alert should be triggered considering cooldown
bool PerformanceMonitor::shouldTriggerAlert(const std::string& metric, const MetricThreshold& threshold) const {
    if (!threshold.alertEnabled) {
        return false;
    }

    if (threshold.cooldownMs > 0) {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastAlert = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - threshold.lastAlertTime);
        return timeSinceLastAlert.count() >= threshold.cooldownMs;
    }

    return true;
}

WEBSOCKET_NAMESPACE_END