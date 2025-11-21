#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "../utils/Logger.hpp"
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

enum class AlertSeverity {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

enum class AlertState {
    ACTIVE,
    ACKNOWLEDGED,
    RESOLVED,
    SUPPRESSED
};

struct Alert {
    std::string id;
    std::string title;
    std::string description;
    AlertSeverity severity;
    AlertState state;
    std::string source;
    std::string metric;
    double value;
    double threshold;
    std::chrono::steady_clock::time_point triggeredAt;
    std::chrono::steady_clock::time_point acknowledgedAt;
    std::chrono::steady_clock::time_point resolvedAt;
    std::string acknowledgedBy;
    std::string resolutionNotes;
    std::unordered_map<std::string, std::any> metadata;
};

class AlertManager {
public:
    static AlertManager& getInstance();

    // Alert triggering
    void triggerAlert(const std::string& title,
        const std::string& description,
        AlertSeverity severity,
        const std::string& source = "",
        const std::string& metric = "",
        double value = 0.0,
        double threshold = 0.0,
        const std::unordered_map<std::string, std::any>& metadata = {});

    void triggerAlert(const Alert& alert);

    // Alert management
    void acknowledgeAlert(const std::string& alertId, const std::string& acknowledgedBy = "");
    void resolveAlert(const std::string& alertId, const std::string& resolutionNotes = "");
    void suppressAlert(const std::string& alertId, uint32_t durationMs = 0); // 0 = indefinite
    void unsuppressAlert(const std::string& alertId);

    // Alert querying
    std::vector<Alert> getActiveAlerts() const;
    std::vector<Alert> getAlertsBySeverity(AlertSeverity severity) const;
    std::vector<Alert> getAlertsBySource(const std::string& source) const;
    std::vector<Alert> getAlertsByMetric(const std::string& metric) const;
    Alert getAlert(const std::string& alertId) const;
    bool hasActiveAlerts() const;

    // Alert configuration
    void setSeverityThreshold(const std::string& metric, AlertSeverity severity, double threshold);
    void setCooldownPeriod(const std::string& metric, uint32_t cooldownMs);
    void enableAlerting(const std::string& metric, bool enabled = true);
    void disableAlerting(const std::string& metric);

    // Notification channels
    void addNotificationChannel(const std::string& channelName,
        std::function<void(const Alert&)> notifier);
    void removeNotificationChannel(const std::string& channelName);
    void sendNotification(const Alert& alert, const std::string& channelName = "");
    void broadcastNotification(const Alert& alert);

    // Alert routing
    void setSeverityRouting(AlertSeverity severity, const std::vector<std::string>& channels);
    void setSourceRouting(const std::string& source, const std::vector<std::string>& channels);

    // Escalation policies
    void setEscalationPolicy(const std::string& policyName,
        uint32_t escalationDelayMs,
        const std::vector<std::string>& escalationChannels);
    void triggerEscalation(const std::string& alertId);

    // Alert aggregation
    void enableAlertAggregation(bool enable);
    void setAggregationWindow(uint32_t windowMs);
    void aggregateSimilarAlerts();

    // Statistics
    struct AlertStats {
        size_t totalAlerts;
        size_t activeAlerts;
        size_t acknowledgedAlerts;
        size_t resolvedAlerts;
        size_t suppressedAlerts;
        std::unordered_map<AlertSeverity, size_t> alertsBySeverity;
        uint64_t notificationsSent;
        uint64_t escalationsTriggered;
    };

    AlertStats getStats() const;
    void resetStats();

    // Maintenance
    void cleanupResolvedAlerts(uint32_t retentionDays = 30);
    void cleanupOldAlerts(uint32_t maxAgeDays = 90);

    // Event handlers
    void setOnAlertTriggeredHandler(std::function<void(const Alert&)> handler);
    void setOnAlertResolvedHandler(std::function<void(const Alert&)> handler);

private:
    AlertManager();
    ~AlertManager();

    WEBSOCKET_DISABLE_COPY(AlertManager)

        struct AlertConfig {
        AlertSeverity severity;
        double threshold;
        uint32_t cooldownMs;
        bool enabled;
        std::chrono::steady_clock::time_point lastTriggered;
    };

    struct NotificationChannel {
        std::string name;
        std::function<void(const Alert&)> notifier;
        bool enabled;
    };

    struct EscalationPolicy {
        std::string name;
        uint32_t delayMs;
        std::vector<std::string> channels;
        std::chrono::steady_clock::time_point lastEscalation;
    };

    mutable std::shared_mutex mutex_;
    mutable std::shared_mutex alertsMutex_;

    std::unordered_map<std::string, Alert> alerts_;
    std::unordered_map<std::string, AlertConfig> alertConfigs_;
    std::unordered_map<std::string, NotificationChannel> notificationChannels_;
    std::unordered_map<std::string, EscalationPolicy> escalationPolicies_;

    std::unordered_map<AlertSeverity, std::vector<std::string>> severityRouting_;
    std::unordered_map<std::string, std::vector<std::string>> sourceRouting_;

    std::atomic<bool> aggregationEnabled_{ false };
    std::atomic<uint32_t> aggregationWindowMs_{ 60000 }; // 1 minute default

    std::function<void(const Alert&)> onAlertTriggeredHandler_;
    std::function<void(const Alert&)> onAlertResolvedHandler_;

    std::atomic<uint64_t> totalAlerts_{ 0 };
    std::atomic<uint64_t> notificationsSent_{ 0 };
    std::atomic<uint64_t> escalationsTriggered_{ 0 };

    std::string generateAlertId() const;
    bool shouldTriggerAlert(const std::string& metric, double value) const;
    void updateAlertState(const std::string& alertId, AlertState newState);
    void processAlertRouting(const Alert& alert);
    void processEscalations();

    void initializeDefaultConfigs();
    void initializeDefaultChannels();

    void logAlert(const Alert& alert, const std::string& action);
    void updateStats(const Alert& alert, AlertState oldState, AlertState newState);
};

WEBSOCKET_NAMESPACE_END