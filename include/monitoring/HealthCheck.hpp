#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

struct HealthStatus {
    bool isHealthy;
    std::string status;
    std::string message;
    std::chrono::steady_clock::time_point lastCheck;
    std::chrono::milliseconds responseTime;
    std::unordered_map<std::string, std::any> details;
};

struct HealthCheckResult {
    std::string checkName;
    bool passed;
    std::string message;
    std::chrono::milliseconds duration;
    std::unordered_map<std::string, std::any> metadata;
};

class HealthCheckManager {
public:
    static HealthCheckManager& getInstance();

    // Health check registration
    void registerCheck(const std::string& name,
        std::function<HealthCheckResult()> checkFunction,
        uint32_t intervalMs = 30000, // 30 seconds default
        bool enabled = true);

    void unregisterCheck(const std::string& name);
    void enableCheck(const std::string& name);
    void disableCheck(const std::string& name);

    // Health check execution
    HealthStatus performCheck(const std::string& name);
    HealthStatus performAllChecks();
    HealthStatus performCriticalChecks();

    // Automatic health monitoring
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

    // Health status access
    HealthStatus getOverallHealth() const;
    HealthStatus getCheckHealth(const std::string& name) const;
    std::unordered_map<std::string, HealthStatus> getAllHealthStatuses() const;

    // Check configuration
    void setCheckInterval(const std::string& name, uint32_t intervalMs);
    void setCheckTimeout(const std::string& name, uint32_t timeoutMs);
    void setCriticalCheck(const std::string& name, bool critical = true);

    // Threshold configuration
    void setWarningThreshold(const std::string& name, double threshold);
    void setCriticalThreshold(const std::string& name, double threshold);

    // Dependency management
    void addCheckDependency(const std::string& checkName, const std::string& dependencyName);
    void removeCheckDependency(const std::string& checkName, const std::string& dependencyName);
    std::vector<std::string> getCheckDependencies(const std::string& name) const;

    // Statistics
    struct HealthStats {
        size_t totalChecks;
        size_t enabledChecks;
        size_t criticalChecks;
        size_t healthyChecks;
        size_t warningChecks;
        size_t criticalFailedChecks;
        uint64_t totalExecutions;
        uint64_t failedExecutions;
    };

    HealthStats getStats() const;

    // Event handlers
    void setOnHealthChangeHandler(std::function<void(const std::string&, const HealthStatus&)> handler);
    void setOnCriticalFailureHandler(std::function<void(const std::string&, const HealthCheckResult&)> handler);

    // Reporting
    std::string generateHealthReport() const;
    std::unordered_map<std::string, std::any> getHealthMetrics() const;

private:
    HealthCheckManager();
    ~HealthCheckManager();

    WEBSOCKET_DISABLE_COPY(HealthCheckManager)

        struct HealthCheck {
        std::string name;
        std::function<HealthCheckResult()> checkFunction;
        uint32_t intervalMs;
        uint32_t timeoutMs;
        bool enabled;
        bool critical;
        double warningThreshold;
        double criticalThreshold;
        std::vector<std::string> dependencies;
        HealthStatus lastStatus;
        std::chrono::steady_clock::time_point lastExecution;
        uint64_t executionCount;
        uint64_t failureCount;
    };

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, HealthCheck> checks_;
    std::atomic<bool> monitoring_{ false };
    std::thread monitoringThread_;

    std::function<void(const std::string&, const HealthStatus&)> onHealthChangeHandler_;
    std::function<void(const std::string&, const HealthCheckResult&)> onCriticalFailureHandler_;

    std::atomic<uint64_t> totalExecutions_{ 0 };
    std::atomic<uint64_t> failedExecutions_{ 0 };

    void monitoringThreadFunction();
    HealthCheckResult executeCheck(const HealthCheck& check);
    bool shouldExecuteCheck(const HealthCheck& check) const;
    void updateHealthStatus(const std::string& name, const HealthCheckResult& result);
    void notifyHealthChange(const std::string& name, const HealthStatus& status);
    void notifyCriticalFailure(const std::string& name, const HealthCheckResult& result);

    bool checkDependencies(const HealthCheck& check) const;
    HealthStatus aggregateHealthStatus(const std::vector<HealthStatus>& statuses) const;
};

WEBSOCKET_NAMESPACE_END