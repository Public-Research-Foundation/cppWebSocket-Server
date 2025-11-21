#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "../utils/Metrics.hpp"
#include <unordered_map>
#include <vector>
#include <deque>
#include <atomic>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

struct StatisticalSummary {
    uint64_t count;
    double sum;
    double mean;
    double min;
    double max;
    double stddev;
    double p50;
    double p95;
    double p99;
    double rate; // per second
};

class StatsCollector {
public:
    StatsCollector(const std::string& name, size_t windowSize = 1000);
    ~StatsCollector() = default;

    WEBSOCKET_DISABLE_COPY(StatsCollector)

        // Data collection
        void recordValue(double value);
    void recordValue(double value, const std::chrono::steady_clock::time_point& timestamp);
    void recordValue(const std::string& key, double value);
    void incrementCounter(const std::string& key, double amount = 1.0);

    // Statistical access
    StatisticalSummary getSummary() const;
    StatisticalSummary getSummary(const std::string& key) const;
    double getRate() const;
    double getRate(const std::string& key) const;

    // Time-window statistics
    StatisticalSummary getWindowSummary(uint32_t windowSeconds = 60) const;
    std::vector<double> getRecentValues(uint32_t count = 100) const;
    std::vector<std::pair<std::chrono::steady_clock::time_point, double>> getTimeSeries(uint32_t durationSeconds = 300) const;

    // Key-based statistics
    std::vector<std::string> getKeys() const;
    std::unordered_map<std::string, StatisticalSummary> getAllSummaries() const;

    // Reset and clear
    void reset();
    void reset(const std::string& key);
    void clearWindow();

    // Configuration
    void setWindowSize(size_t windowSize);
    void setRetentionPeriod(uint32_t seconds);
    void enableRollingStats(bool enable);

    // Alerting
    void setAlertThreshold(double threshold, std::function<void(double)> alertHandler = nullptr);
    void setRateAlertThreshold(double threshold, std::function<void(double)> alertHandler = nullptr);

    // Export and serialization
    std::unordered_map<std::string, std::any> exportStats() const;
    std::string toJson() const;

private:
    struct DataPoint {
        double value;
        std::chrono::steady_clock::time_point timestamp;
    };

    struct KeyStats {
        std::deque<DataPoint> window;
        std::atomic<double> sum{ 0.0 };
        std::atomic<double> sumSquares{ 0.0 };
        std::atomic<double> min{ 0.0 };
        std::atomic<double> max{ 0.0 };
        std::atomic<uint64_t> count{ 0 };
        std::chrono::steady_clock::time_point firstSeen;
        std::chrono::steady_clock::time_point lastSeen;
    };

    mutable std::shared_mutex mutex_;

    std::string name_;
    size_t windowSize_;
    uint32_t retentionPeriod_{ 3600 }; // 1 hour default

    KeyStats globalStats_;
    std::unordered_map<std::string, KeyStats> keyedStats_;
    std::deque<DataPoint> timeSeries_;

    std::atomic<bool> rollingStatsEnabled_{ true };

    double alertThreshold_{ 0.0 };
    double rateAlertThreshold_{ 0.0 };
    std::function<void(double)> alertHandler_;
    std::function<void(double)> rateAlertHandler_;

    void updateGlobalStats(double value, const std::chrono::steady_clock::time_point& timestamp);
    void updateKeyedStats(const std::string& key, double value, const std::chrono::steady_clock::time_point& timestamp);
    void cleanupOldData();

    StatisticalSummary calculateSummary(const KeyStats& stats) const;
    StatisticalSummary calculateWindowSummary(const std::deque<DataPoint>& window) const;
    double calculateRate(const KeyStats& stats) const;

    void checkAlerts(double value, double rate);
    void pruneTimeSeries();
};

class StatsAggregator {
public:
    static StatsAggregator& getInstance();

    // Collector management
    SharedPtr<StatsCollector> createCollector(const std::string& name, size_t windowSize = 1000);
    SharedPtr<StatsCollector> getCollector(const std::string& name) const;
    bool hasCollector(const std::string& name) const;
    void removeCollector(const std::string& name);

    // Bulk operations
    void recordValueToAll(double value);
    void resetAll();
    std::unordered_map<std::string, StatisticalSummary> getAllSummaries() const;

    // Aggregated statistics
    StatisticalSummary getAggregatedSummary() const;
    std::unordered_map<std::string, double> getAggregatedRates() const;

    // Reporting
    std::string generateAggregatedReport() const;
    std::unordered_map<std::string, std::any> exportAllStats() const;

    // Automatic collection
    void startAutoCollection(uint32_t intervalMs = 5000);
    void stopAutoCollection();
    bool isAutoCollecting() const;

private:
    StatsAggregator();
    ~StatsAggregator();

    WEBSOCKET_DISABLE_COPY(StatsAggregator)

        mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, SharedPtr<StatsCollector>> collectors_;
    std::atomic<bool> autoCollecting_{ false };
    std::thread collectionThread_;

    void autoCollectionThreadFunction(uint32_t intervalMs);
};

WEBSOCKET_NAMESPACE_END