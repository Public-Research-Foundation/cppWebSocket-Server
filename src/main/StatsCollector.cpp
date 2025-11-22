#include "monitoring/StatsCollector.hpp"
#include "utils/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

WEBSOCKET_NAMESPACE_BEGIN

// StatsCollector implementation
StatsCollector::StatsCollector(const std::string& name, size_t windowSize)
    : name_(name), windowSize_(windowSize) {

    // Initialize global stats
    globalStats_.min = std::numeric_limits<double>::max();
    globalStats_.max = std::numeric_limits<double>::lowest();
    globalStats_.firstSeen = std::chrono::steady_clock::now();
    globalStats_.lastSeen = globalStats_.firstSeen;

    LOG_DEBUG("StatsCollector created: {}", name_);
}

// Record a value with current timestamp
void StatsCollector::recordValue(double value) {
    recordValue(value, std::chrono::steady_clock::now());
}

// Record a value with specific timestamp
void StatsCollector::recordValue(double value, const std::chrono::steady_clock::time_point& timestamp) {
    DataPoint point{ value, timestamp };

    {
        std::unique_lock lock(mutex_);
        updateGlobalStats(value, timestamp);
        timeSeries_.push_back(point);

        // Maintain window size
        if (rollingStatsEnabled_ && timeSeries_.size() > windowSize_) {
            timeSeries_.pop_front();
        }

        // Cleanup old data if retention period is set
        cleanupOldData();
    }

    // Check for alerts
    checkAlerts(value, calculateRate(globalStats_));
}

// Record a value for a specific key
void StatsCollector::recordValue(const std::string& key, double value) {
    auto timestamp = std::chrono::steady_clock::now();
    DataPoint point{ value, timestamp };

    {
        std::unique_lock lock(mutex_);

        // Update or create key stats
        KeyStats& stats = keyedStats_[key];
        updateKeyedStats(stats, value, timestamp);

        // Maintain window size for this key
        if (rollingStatsEnabled_ && stats.window.size() > windowSize_) {
            stats.window.pop_front();
        }
    }
}

// Increment a counter for a specific key
void StatsCollector::incrementCounter(const std::string& key, double amount) {
    recordValue(key, amount);
}

// Get statistical summary for all data
StatisticalSummary StatsCollector::getSummary() const {
    std::shared_lock lock(mutex_);
    return calculateSummary(globalStats_);
}

// Get statistical summary for a specific key
StatisticalSummary StatsCollector::getSummary(const std::string& key) const {
    std::shared_lock lock(mutex_);

    auto it = keyedStats_.find(key);
    if (it != keyedStats_.end()) {
        return calculateSummary(it->second);
    }

    // Return empty summary if key not found
    StatisticalSummary summary;
    summary.count = 0;
    return summary;
}

// Get current rate (values per second)
double StatsCollector::getRate() const {
    std::shared_lock lock(mutex_);
    return calculateRate(globalStats_);
}

// Get rate for a specific key
double StatsCollector::getRate(const std::string& key) const {
    std::shared_lock lock(mutex_);

    auto it = keyedStats_.find(key);
    if (it != keyedStats_.end()) {
        return calculateRate(it->second);
    }

    return 0.0;
}

// Get summary for a specific time window
StatisticalSummary StatsCollector::getWindowSummary(uint32_t windowSeconds) const {
    std::shared_lock lock(mutex_);

    auto cutoffTime = std::chrono::steady_clock::now() - std::chrono::seconds(windowSeconds);
    std::deque<DataPoint> windowData;

    // Collect data within the time window
    for (const auto& point : timeSeries_) {
        if (point.timestamp >= cutoffTime) {
            windowData.push_back(point);
        }
    }

    return calculateWindowSummary(windowData);
}

// Get recent values (most recent first)
std::vector<double> StatsCollector::getRecentValues(uint32_t count) const {
    std::shared_lock lock(mutex_);

    std::vector<double> values;
    size_t actualCount = std::min(count, static_cast<uint32_t>(timeSeries_.size()));

    // Get most recent values
    auto it = timeSeries_.rbegin();
    for (size_t i = 0; i < actualCount && it != timeSeries_.rend(); ++i, ++it) {
        values.push_back(it->value);
    }

    return values;
}

// Get time series data for specified duration
std::vector<std::pair<std::chrono::steady_clock::time_point, double>>
StatsCollector::getTimeSeries(uint32_t durationSeconds) const {
    std::shared_lock lock(mutex_);

    auto cutoffTime = std::chrono::steady_clock::now() - std::chrono::seconds(durationSeconds);
    std::vector<std::pair<std::chrono::steady_clock::time_point, double>> series;

    for (const auto& point : timeSeries_) {
        if (point.timestamp >= cutoffTime) {
            series.emplace_back(point.timestamp, point.value);
        }
    }

    return series;
}

// Get all registered keys
std::vector<std::string> StatsCollector::getKeys() const {
    std::shared_lock lock(mutex_);

    std::vector<std::string> keys;
    for (const auto& [key, stats] : keyedStats_) {
        keys.push_back(key);
    }

    return keys;
}

// Get summaries for all keys
std::unordered_map<std::string, StatisticalSummary> StatsCollector::getAllSummaries() const {
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, StatisticalSummary> summaries;
    for (const auto& [key, stats] : keyedStats_) {
        summaries[key] = calculateSummary(stats);
    }

    return summaries;
}

// Reset all statistics
void StatsCollector::reset() {
    std::unique_lock lock(mutex_);

    // Reset global stats
    globalStats_ = KeyStats();
    globalStats_.min = std::numeric_limits<double>::max();
    globalStats_.max = std::numeric_limits<double>::lowest();
    globalStats_.firstSeen = std::chrono::steady_clock::now();
    globalStats_.lastSeen = globalStats_.firstSeen;

    // Clear time series
    timeSeries_.clear();

    // Clear keyed stats
    keyedStats_.clear();

    LOG_DEBUG("StatsCollector reset: {}", name_);
}

// Reset statistics for a specific key
void StatsCollector::reset(const std::string& key) {
    std::unique_lock lock(mutex_);
    keyedStats_.erase(key);
}

// Clear time window data
void StatsCollector::clearWindow() {
    std::unique_lock lock(mutex_);
    timeSeries_.clear();
}

// Configure window size
void StatsCollector::setWindowSize(size_t windowSize) {
    std::unique_lock lock(mutex_);
    windowSize_ = windowSize;

    // Trim if necessary
    if (timeSeries_.size() > windowSize_) {
        timeSeries_.resize(windowSize_);
    }
}

// Configure data retention period
void StatsCollector::setRetentionPeriod(uint32_t seconds) {
    std::unique_lock lock(mutex_);
    retentionPeriod_ = seconds;
}

// Enable or disable rolling statistics
void StatsCollector::enableRollingStats(bool enable) {
    rollingStatsEnabled_ = enable;
}

// Set alert threshold for values
void StatsCollector::setAlertThreshold(double threshold, std::function<void(double)> alertHandler) {
    alertThreshold_ = threshold;
    alertHandler_ = alertHandler;
}

// Set alert threshold for rates
void StatsCollector::setRateAlertThreshold(double threshold, std::function<void(double)> alertHandler) {
    rateAlertThreshold_ = threshold;
    rateAlertHandler_ = alertHandler;
}

// Export statistics as key-value pairs
std::unordered_map<std::string, std::any> StatsCollector::exportStats() const {
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, std::any> stats;
    StatisticalSummary summary = getSummary();

    stats["count"] = summary.count;
    stats["sum"] = summary.sum;
    stats["mean"] = summary.mean;
    stats["min"] = summary.min;
    stats["max"] = summary.max;
    stats["stddev"] = summary.stddev;
    stats["p50"] = summary.p50;
    stats["p95"] = summary.p95;
    stats["p99"] = summary.p99;
    stats["rate"] = summary.rate;

    return stats;
}

// Convert statistics to JSON format
std::string StatsCollector::toJson() const {
    auto stats = exportStats();
    std::ostringstream json;

    json << "{";
    bool first = true;

    for (const auto& [key, value] : stats) {
        if (!first) json << ",";
        first = false;

        json << "\"" << key << "\":";

        if (value.type() == typeid(int)) {
            json << std::any_cast<int>(value);
        }
        else if (value.type() == typeid(double)) {
            json << std::any_cast<double>(value);
        }
        else if (value.type() == typeid(uint64_t)) {
            json << std::any_cast<uint64_t>(value);
        }
    }

    json << "}";
    return json.str();
}

// Private methods

// Update global statistics with new value
void StatsCollector::updateGlobalStats(double value, const std::chrono::steady_clock::time_point& timestamp) {
    globalStats_.sum += value;
    globalStats_.sumSquares += value * value;
    globalStats_.min = std::min(globalStats_.min, value);
    globalStats_.max = std::max(globalStats_.max, value);
    globalStats_.count++;
    globalStats_.lastSeen = timestamp;
}

// Update keyed statistics with new value
void StatsCollector::updateKeyedStats(KeyStats& stats, double value, const std::chrono::steady_clock::time_point& timestamp) {
    stats.sum += value;
    stats.sumSquares += value * value;
    stats.min = std::min(stats.min, value);
    stats.max = std::max(stats.max, value);
    stats.count++;
    stats.lastSeen = timestamp;

    if (stats.count == 1) {
        stats.firstSeen = timestamp;
    }

    // Add to rolling window
    stats.window.push_back({ value, timestamp });
}

// Clean up old data based on retention period
void StatsCollector::cleanupOldData() {
    if (retentionPeriod_ == 0) {
        return; // No retention limit
    }

    auto cutoffTime = std::chrono::steady_clock::now() - std::chrono::seconds(retentionPeriod_);

    // Remove old data points from time series
    while (!timeSeries_.empty() && timeSeries_.front().timestamp < cutoffTime) {
        timeSeries_.pop_front();
    }

    // Remove old data from keyed stats
    for (auto& [key, stats] : keyedStats_) {
        while (!stats.window.empty() && stats.window.front().timestamp < cutoffTime) {
            stats.window.pop_front();
        }
    }
}

// Calculate statistical summary from key stats
StatisticalSummary StatsCollector::calculateSummary(const KeyStats& stats) const {
    StatisticalSummary summary;

    summary.count = stats.count;
    summary.sum = stats.sum;

    if (stats.count > 0) {
        summary.mean = stats.sum / stats.count;
        summary.min = stats.min;
        summary.max = stats.max;

        // Calculate standard deviation
        if (stats.count > 1) {
            double variance = (stats.sumSquares - (stats.sum * stats.sum) / stats.count) / (stats.count - 1);
            summary.stddev = std::sqrt(std::max(0.0, variance));
        }
        else {
            summary.stddev = 0.0;
        }

        // Calculate percentiles from time series
        std::vector<double> values;
        for (const auto& point : timeSeries_) {
            values.push_back(point.value);
        }

        if (!values.empty()) {
            std::sort(values.begin(), values.end());
            summary.p50 = calculatePercentile(values, 0.50);
            summary.p95 = calculatePercentile(values, 0.95);
            summary.p99 = calculatePercentile(values, 0.99);
        }

        // Calculate rate
        summary.rate = calculateRate(stats);
    }
    else {
        summary.mean = 0.0;
        summary.min = 0.0;
        summary.max = 0.0;
        summary.stddev = 0.0;
        summary.p50 = 0.0;
        summary.p95 = 0.0;
        summary.p99 = 0.0;
        summary.rate = 0.0;
    }

    return summary;
}

// Calculate summary for a specific window of data
StatisticalSummary StatsCollector::calculateWindowSummary(const std::deque<DataPoint>& window) const {
    StatisticalSummary summary;
    summary.count = window.size();

    if (window.empty()) {
        return summary;
    }

    // Calculate basic statistics
    double sum = 0.0;
    double sumSquares = 0.0;
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();

    std::vector<double> values;
    for (const auto& point : window) {
        sum += point.value;
        sumSquares += point.value * point.value;
        min = std::min(min, point.value);
        max = std::max(max, point.value);
        values.push_back(point.value);
    }

    summary.sum = sum;
    summary.mean = sum / window.size();
    summary.min = min;
    summary.max = max;

    // Calculate standard deviation
    if (window.size() > 1) {
        double variance = (sumSquares - (sum * sum) / window.size()) / (window.size() - 1);
        summary.stddev = std::sqrt(std::max(0.0, variance));
    }

    // Calculate percentiles
    if (!values.empty()) {
        std::sort(values.begin(), values.end());
        summary.p50 = calculatePercentile(values, 0.50);
        summary.p95 = calculatePercentile(values, 0.95);
        summary.p99 = calculatePercentile(values, 0.99);
    }

    // Calculate rate (values per second)
    if (window.size() >= 2) {
        auto timeSpan = std::chrono::duration_cast<std::chrono::seconds>(
            window.back().timestamp - window.front().timestamp);
        if (timeSpan.count() > 0) {
            summary.rate = static_cast<double>(window.size()) / timeSpan.count();
        }
    }

    return summary;
}

// Calculate rate (values per second)
double StatsCollector::calculateRate(const KeyStats& stats) const {
    if (stats.count < 2) {
        return 0.0;
    }

    auto timeSpan = std::chrono::duration_cast<std::chrono::seconds>(
        stats.lastSeen - stats.firstSeen);

    if (timeSpan.count() > 0) {
        return static_cast<double>(stats.count) / timeSpan.count();
    }

    return 0.0;
}

// Calculate percentile from sorted values
double StatsCollector::calculatePercentile(const std::vector<double>& values, double percentile) const {
    if (values.empty()) {
        return 0.0;
    }

    size_t index = static_cast<size_t>(percentile * (values.size() - 1));
    return values[index];
}

// Check for alert conditions
void StatsCollector::checkAlerts(double value, double rate) {
    if (alertHandler_ && alertThreshold_ > 0 && value >= alertThreshold_) {
        try {
            alertHandler_(value);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Alert handler failed: {}", e.what());
        }
    }

    if (rateAlertHandler_ && rateAlertThreshold_ > 0 && rate >= rateAlertThreshold_) {
        try {
            rateAlertHandler_(rate);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Rate alert handler failed: {}", e.what());
        }
    }
}

// Prune time series data if it grows too large
void StatsCollector::pruneTimeSeries() {
    if (timeSeries_.size() > windowSize_ * 2) {
        // Keep only the most recent windowSize_ elements
        std::deque<DataPoint> recent(timeSeries_.end() - windowSize_, timeSeries_.end());
        timeSeries_.swap(recent);
    }
}

// StatsAggregator implementation
StatsAggregator& StatsAggregator::getInstance() {
    static StatsAggregator instance;
    return instance;
}

StatsAggregator::StatsAggregator() {
    LOG_DEBUG("StatsAggregator created");
}

StatsAggregator::~StatsAggregator() {
    stopAutoCollection();
    LOG_DEBUG("StatsAggregator destroyed");
}

// Create a new stats collector
SharedPtr<StatsCollector> StatsAggregator::createCollector(const std::string& name, size_t windowSize) {
    std::unique_lock lock(mutex_);

    auto collector = std::make_shared<StatsCollector>(name, windowSize);
    collectors_[name] = collector;

    LOG_DEBUG("Created stats collector: {}", name);
    return collector;
}

// Get an existing stats collector
SharedPtr<StatsCollector> StatsAggregator::getCollector(const std::string& name) const {
    std::shared_lock lock(mutex_);

    auto it = collectors_.find(name);
    return it != collectors_.end() ? it->second : nullptr;
}

// Check if a collector exists
bool StatsAggregator::hasCollector(const std::string& name) const {
    std::shared_lock lock(mutex_);
    return collectors_.find(name) != collectors_.end();
}

// Remove a stats collector
void StatsAggregator::removeCollector(const std::string& name) {
    std::unique_lock lock(mutex_);
    collectors_.erase(name);
    LOG_DEBUG("Removed stats collector: {}", name);
}

// Record a value to all collectors
void StatsAggregator::recordValueToAll(double value) {
    std::shared_lock lock(mutex_);

    for (const auto& [name, collector] : collectors_) {
        collector->recordValue(value);
    }
}

// Reset all collectors
void StatsAggregator::resetAll() {
    std::shared_lock lock(mutex_);

    for (const auto& [name, collector] : collectors_) {
        collector->reset();
    }

    LOG_DEBUG("Reset all stats collectors");
}

// Get summaries from all collectors
std::unordered_map<std::string, StatisticalSummary> StatsAggregator::getAllSummaries() const {
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, StatisticalSummary> allSummaries;
    for (const auto& [name, collector] : collectors_) {
        allSummaries[name] = collector->getSummary();
    }

    return allSummaries;
}

// Get aggregated summary across all collectors
StatisticalSummary StatsAggregator::getAggregatedSummary() const {
    auto allSummaries = getAllSummaries();

    StatisticalSummary aggregated;
    aggregated.count = 0;
    aggregated.sum = 0.0;
    aggregated.min = std::numeric_limits<double>::max();
    aggregated.max = std::numeric_limits<double>::lowest();

    // Aggregate basic statistics
    for (const auto& [name, summary] : allSummaries) {
        if (summary.count > 0) {
            aggregated.count += summary.count;
            aggregated.sum += summary.sum;
            aggregated.min = std::min(aggregated.min, summary.min);
            aggregated.max = std::max(aggregated.max, summary.max);
        }
    }

    // Calculate mean
    if (aggregated.count > 0) {
        aggregated.mean = aggregated.sum / aggregated.count;
    }

    return aggregated;
}

// Get aggregated rates across all collectors
std::unordered_map<std::string, double> StatsAggregator::getAggregatedRates() const {
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, double> rates;
    for (const auto& [name, collector] : collectors_) {
        rates[name] = collector->getRate();
    }

    return rates;
}

// Generate aggregated report
std::string StatsAggregator::generateAggregatedReport() const {
    auto allSummaries = getAllSummaries();

    std::ostringstream report;
    report << "Stats Aggregated Report\n";
    report << "======================\n\n";

    report << "Collectors: " << allSummaries.size() << "\n\n";

    for (const auto& [name, summary] : allSummaries) {
        report << name << ":\n";
        report << "  Count: " << summary.count << "\n";
        report << "  Mean: " << summary.mean << "\n";
        report << "  Min: " << summary.min << "\n";
        report << "  Max: " << summary.max << "\n";
        report << "  Rate: " << summary.rate << " values/s\n";
        report << "  P95: " << summary.p95 << "\n";
        report << "\n";
    }

    return report.str();
}

// Export all statistics
std::unordered_map<std::string, std::any> StatsAggregator::exportAllStats() const {
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, std::any> allStats;
    for (const auto& [name, collector] : collectors_) {
        allStats[name] = collector->exportStats();
    }

    return allStats;
}

// Start automatic collection
void StatsAggregator::startAutoCollection(uint32_t intervalMs) {
    if (autoCollecting_) {
        LOG_WARN("Auto collection already running");
        return;
    }

    autoCollecting_ = true;
    collectionThread_ = std::thread(&StatsAggregator::autoCollectionThreadFunction, this, intervalMs);
    LOG_INFO("Auto stats collection started");
}

// Stop automatic collection
void StatsAggregator::stopAutoCollection() {
    if (!autoCollecting_) {
        return;
    }

    autoCollecting_ = false;
    if (collectionThread_.joinable()) {
        collectionThread_.join();
    }
    LOG_INFO("Auto stats collection stopped");
}

bool StatsAggregator::isAutoCollecting() const {
    return autoCollecting_;
}

// Automatic collection thread function
void StatsAggregator::autoCollectionThreadFunction(uint32_t intervalMs) {
    LOG_DEBUG("Auto stats collection thread started");

    while (autoCollecting_) {
        // Collect system metrics and record to appropriate collectors
        // This would integrate with system monitoring

        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }

    LOG_DEBUG("Auto stats collection thread stopped");
}

WEBSOCKET_NAMESPACE_END