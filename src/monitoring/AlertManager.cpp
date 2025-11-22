#include "monitoring/AlertManager.hpp"
#include "utils/Logger.hpp"
#include <atomic>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

WEBSOCKET_NAMESPACE_BEGIN

AlertManager& AlertManager::getInstance() {
    static AlertManager instance;
    return instance;
}

AlertManager::AlertManager() {
    initializeDefaultConfigs();
    initializeDefaultChannels();
}

AlertManager::~AlertManager() {
    // Cleanup resources if needed
}

void AlertManager::triggerAlert(const std::string& title,
    const std::string& description,
    AlertSeverity severity,
    const std::string& source,
    const std::string& metric,
    double value,
    double threshold,
    const std::unordered_map<std::string, std::any>& metadata) {

    Alert alert;
    alert.id = generateAlertId();
    alert.title = title;
    alert.description = description;
    alert.severity = severity;
    alert.state = AlertState::ACTIVE;
    alert.source = source;
    alert.metric = metric;
    alert.value = value;
    alert.threshold = threshold;
    alert.triggeredAt = std::chrono::steady_clock::now();
    alert.metadata = metadata;

    triggerAlert(alert);
}

void AlertManager::triggerAlert(const Alert& alert) {
    // Check if alert should be suppressed due to cooldown
    if (!alert.metric.empty()) {
        std::shared_lock lock(mutex_);
        auto configIt = alertConfigs_.find(alert.metric);
        if (configIt != alertConfigs_.end() && !configIt->second.enabled) {
            return; // Alerting disabled for this metric
        }

        if (configIt != alertConfigs_.end()) {
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastTrigger = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - configIt->second.lastTriggered);
            if (timeSinceLastTrigger.count() < static_cast<int64_t>(configIt->second.cooldownMs)) {
                return; // Still in cooldown period
            }
        }
    }

    {
        std::unique_lock lock(alertsMutex_);
        alerts_[alert.id] = alert;

        // Update cooldown timer if this is a metric-based alert
        if (!alert.metric.empty()) {
            std::unique_lock configLock(mutex_);
            alertConfigs_[alert.metric].lastTriggered = std::chrono::steady_clock::now();
        }
    }

    logAlert(alert, "triggered");

    // Notify subscribers
    if (onAlertTriggeredHandler_) {
        onAlertTriggeredHandler_(alert);
    }

    // Process routing and notifications
    processAlertRouting(alert);
    broadcastNotification(alert);

    // Update statistics
    totalAlerts_++;
}

void AlertManager::acknowledgeAlert(const std::string& alertId, const std::string& acknowledgedBy) {
    std::unique_lock lock(alertsMutex_);
    auto it = alerts_.find(alertId);
    if (it != alerts_.end() && it->second.state == AlertState::ACTIVE) {
        auto oldState = it->second.state;
        it->second.state = AlertState::ACKNOWLEDGED;
        it->second.acknowledgedAt = std::chrono::steady_clock::now();
        it->second.acknowledgedBy = acknowledgedBy;

        logAlert(it->second, "acknowledged");
        updateStats(it->second, oldState, AlertState::ACKNOWLEDGED);
    }
}

void AlertManager::resolveAlert(const std::string& alertId, const std::string& resolutionNotes) {
    std::unique_lock lock(alertsMutex_);
    auto it = alerts_.find(alertId);
    if (it != alerts_.end() && it->second.state != AlertState::RESOLVED) {
        auto oldState = it->second.state;
        it->second.state = AlertState::RESOLVED;
        it->second.resolvedAt = std::chrono::steady_clock::now();
        it->second.resolutionNotes = resolutionNotes;

        logAlert(it->second, "resolved");
        updateStats(it->second, oldState, AlertState::RESOLVED);

        if (onAlertResolvedHandler_) {
            onAlertResolvedHandler_(it->second);
        }
    }
}

void AlertManager::suppressAlert(const std::string& alertId, uint32_t durationMs) {
    std::unique_lock lock(alertsMutex_);
    auto it = alerts_.find(alertId);
    if (it != alerts_.end()) {
        auto oldState = it->second.state;
        it->second.state = AlertState::SUPPRESSED;
        updateStats(it->second, oldState, AlertState::SUPPRESSED);
        logAlert(it->second, "suppressed");

        // TODO: Implement duration-based suppression timer
        (void)durationMs; // Suppress unused parameter warning for now
    }
}

void AlertManager::unsuppressAlert(const std::string& alertId) {
    std::unique_lock lock(alertsMutex_);
    auto it = alerts_.find(alertId);
    if (it != alerts_.end() && it->second.state == AlertState::SUPPRESSED) {
        auto oldState = it->second.state;
        it->second.state = AlertState::ACTIVE;
        updateStats(it->second, oldState, AlertState::ACTIVE);
        logAlert(it->second, "unsuppressed");
    }
}

std::vector<Alert> AlertManager::getActiveAlerts() const {
    std::shared_lock lock(alertsMutex_);
    std::vector<Alert> activeAlerts;
    for (const auto& [id, alert] : alerts_) {
        if (alert.state == AlertState::ACTIVE) {
            activeAlerts.push_back(alert);
        }
    }
    return activeAlerts;
}

std::vector<Alert> AlertManager::getAlertsBySeverity(AlertSeverity severity) const {
    std::shared_lock lock(alertsMutex_);
    std::vector<Alert> filteredAlerts;
    for (const auto& [id, alert] : alerts_) {
        if (alert.severity == severity) {
            filteredAlerts.push_back(alert);
        }
    }
    return filteredAlerts;
}

std::vector<Alert> AlertManager::getAlertsBySource(const std::string& source) const {
    std::shared_lock lock(alertsMutex_);
    std::vector<Alert> filteredAlerts;
    for (const auto& [id, alert] : alerts_) {
        if (alert.source == source) {
            filteredAlerts.push_back(alert);
        }
    }
    return filteredAlerts;
}

std::vector<Alert> AlertManager::getAlertsByMetric(const std::string& metric) const {
    std::shared_lock lock(alertsMutex_);
    std::vector<Alert> filteredAlerts;
    for (const auto& [id, alert] : alerts_) {
        if (alert.metric == metric) {
            filteredAlerts.push_back(alert);
        }
    }
    return filteredAlerts;
}

Alert AlertManager::getAlert(const std::string& alertId) const {
    std::shared_lock lock(alertsMutex_);
    auto it = alerts_.find(alertId);
    if (it != alerts_.end()) {
        return it->second;
    }

    // Return empty alert if not found
    Alert emptyAlert;
    emptyAlert.state = AlertState::RESOLVED; // Mark as resolved to indicate not found
    return emptyAlert;
}

bool AlertManager::hasActiveAlerts() const {
    std::shared_lock lock(alertsMutex_);
    for (const auto& [id, alert] : alerts_) {
        if (alert.state == AlertState::ACTIVE) {
            return true;
        }
    }
    return false;
}

void AlertManager::setSeverityThreshold(const std::string& metric, AlertSeverity severity, double threshold) {
    std::unique_lock lock(mutex_);
    alertConfigs_[metric].severity = severity;
    alertConfigs_[metric].threshold = threshold;
    alertConfigs_[metric].enabled = true;
}

void AlertManager::setCooldownPeriod(const std::string& metric, uint32_t cooldownMs) {
    std::unique_lock lock(mutex_);
    alertConfigs_[metric].cooldownMs = cooldownMs;
}

void AlertManager::enableAlerting(const std::string& metric, bool enabled) {
    std::unique_lock lock(mutex_);
    alertConfigs_[metric].enabled = enabled;
}

void AlertManager::disableAlerting(const std::string& metric) {
    enableAlerting(metric, false);
}

void AlertManager::addNotificationChannel(const std::string& channelName,
    std::function<void(const Alert&)> notifier) {
    std::unique_lock lock(mutex_);
    notificationChannels_[channelName] = NotificationChannel{ channelName, notifier, true };
}

void AlertManager::removeNotificationChannel(const std::string& channelName) {
    std::unique_lock lock(mutex_);
    notificationChannels_.erase(channelName);
}

void AlertManager::sendNotification(const Alert& alert, const std::string& channelName) {
    std::shared_lock lock(mutex_);
    auto it = notificationChannels_.find(channelName);
    if (it != notificationChannels_.end() && it->second.enabled) {
        try {
            it->second.notifier(alert);
            notificationsSent_++;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to send notification via channel '" + channelName + "': " + e.what());
        }
    }
}

void AlertManager::broadcastNotification(const Alert& alert) {
    std::shared_lock lock(mutex_);
    for (const auto& [name, channel] : notificationChannels_) {
        if (channel.enabled) {
            try {
                channel.notifier(alert);
                notificationsSent_++;
            }
            catch (const std::exception& e) {
                LOG_ERROR("Failed to broadcast notification via channel '" + name + "': " + e.what());
            }
        }
    }
}

void AlertManager::setSeverityRouting(AlertSeverity severity, const std::vector<std::string>& channels) {
    std::unique_lock lock(mutex_);
    severityRouting_[severity] = channels;
}

void AlertManager::setSourceRouting(const std::string& source, const std::vector<std::string>& channels) {
    std::unique_lock lock(mutex_);
    sourceRouting_[source] = channels;
}

void AlertManager::setEscalationPolicy(const std::string& policyName,
    uint32_t escalationDelayMs,
    const std::vector<std::string>& escalationChannels) {
    std::unique_lock lock(mutex_);
    escalationPolicies_[policyName] = EscalationPolicy{ policyName, escalationDelayMs, escalationChannels, std::chrono::steady_clock::time_point{} };
}

void AlertManager::triggerEscalation(const std::string& alertId) {
    // Implementation for escalation logic
    escalationsTriggered_++;
    LOG_WARN("Escalation triggered for alert: " + alertId);
}

void AlertManager::enableAlertAggregation(bool enable) {
    aggregationEnabled_ = enable;
}

void AlertManager::setAggregationWindow(uint32_t windowMs) {
    aggregationWindowMs_ = windowMs;
}

void AlertManager::aggregateSimilarAlerts() {
    // Implementation for alert aggregation logic
    LOG_DEBUG("Aggregating similar alerts");
}

AlertManager::AlertStats AlertManager::getStats() const {
    AlertStats stats{};
    std::shared_lock lock(alertsMutex_);

    stats.totalAlerts = alerts_.size();

    for (const auto& [id, alert] : alerts_) {
        stats.alertsBySeverity[alert.severity]++;

        switch (alert.state) {
        case AlertState::ACTIVE: stats.activeAlerts++; break;
        case AlertState::ACKNOWLEDGED: stats.acknowledgedAlerts++; break;
        case AlertState::RESOLVED: stats.resolvedAlerts++; break;
        case AlertState::SUPPRESSED: stats.suppressedAlerts++; break;
        }
    }

    stats.notificationsSent = notificationsSent_.load();
    stats.escalationsTriggered = escalationsTriggered_.load();

    return stats;
}

void AlertManager::resetStats() {
    totalAlerts_ = 0;
    notificationsSent_ = 0;
    escalationsTriggered_ = 0;

    // Note: We don't clear the actual alerts map, just the counters
}

void AlertManager::cleanupResolvedAlerts(uint32_t retentionDays) {
    std::unique_lock lock(alertsMutex_);
    auto cutoffTime = std::chrono::steady_clock::now() - std::chrono::hours(24 * retentionDays);

    auto it = alerts_.begin();
    while (it != alerts_.end()) {
        if (it->second.state == AlertState::RESOLVED &&
            it->second.resolvedAt < cutoffTime) {
            it = alerts_.erase(it);
        }
        else {
            ++it;
        }
    }
}

void AlertManager::cleanupOldAlerts(uint32_t maxAgeDays) {
    std::unique_lock lock(alertsMutex_);
    auto cutoffTime = std::chrono::steady_clock::now() - std::chrono::hours(24 * maxAgeDays);

    auto it = alerts_.begin();
    while (it != alerts_.end()) {
        if (it->second.triggeredAt < cutoffTime) {
            it = alerts_.erase(it);
        }
        else {
            ++it;
        }
    }
}

void AlertManager::setOnAlertTriggeredHandler(std::function<void(const Alert&)> handler) {
    std::unique_lock lock(mutex_);
    onAlertTriggeredHandler_ = handler;
}

void AlertManager::setOnAlertResolvedHandler(std::function<void(const Alert&)> handler) {
    std::unique_lock lock(mutex_);
    onAlertResolvedHandler_ = handler;
}

// Private methods implementation
std::string AlertManager::generateAlertId() const {
    static std::atomic<uint64_t> counter{ 0 };
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    auto id = ++counter;

    std::stringstream ss;
    ss << "ALERT-" << timestamp << "-" << id;
    return ss.str();
}

bool AlertManager::shouldTriggerAlert(const std::string& metric, double value) const {
    std::shared_lock lock(mutex_);
    auto it = alertConfigs_.find(metric);
    if (it == alertConfigs_.end() || !it->second.enabled) {
        return false;
    }

    return value >= it->second.threshold;
}

void AlertManager::updateAlertState(const std::string& alertId, AlertState newState) {
    std::unique_lock lock(alertsMutex_);
    auto it = alerts_.find(alertId);
    if (it != alerts_.end()) {
        auto oldState = it->second.state;
        it->second.state = newState;
        updateStats(it->second, oldState, newState);
    }
}

void AlertManager::processAlertRouting(const Alert& alert) {
    std::shared_lock lock(mutex_);

    // Check severity-based routing
    auto severityIt = severityRouting_.find(alert.severity);
    if (severityIt != severityRouting_.end()) {
        for (const auto& channel : severityIt->second) {
            sendNotification(alert, channel);
        }
    }

    // Check source-based routing
    auto sourceIt = sourceRouting_.find(alert.source);
    if (sourceIt != sourceRouting_.end()) {
        for (const auto& channel : sourceIt->second) {
            sendNotification(alert, channel);
        }
    }
}

void AlertManager::processEscalations() {
    auto now = std::chrono::steady_clock::now();
    std::shared_lock alertLock(alertsMutex_);
    std::shared_lock policyLock(mutex_);

    for (const auto& [policyName, policy] : escalationPolicies_) {
        for (const auto& [alertId, alert] : alerts_) {
            if (alert.state == AlertState::ACTIVE) {
                auto timeActive = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - alert.triggeredAt);

                if (timeActive.count() >= static_cast<int64_t>(policy.delayMs)) {
                    // Trigger escalation for this alert
                    for (const auto& channel : policy.channels) {
                        sendNotification(alert, channel);
                    }
                    escalationsTriggered_++;
                }
            }
        }
    }
}

void AlertManager::initializeDefaultConfigs() {
    std::unique_lock lock(mutex_);

    // Set default cooldown periods and thresholds
    alertConfigs_["connection_count"] = AlertConfig{ AlertSeverity::WARNING, 1000.0, 60000, true, std::chrono::steady_clock::time_point{} };
    alertConfigs_["memory_usage"] = AlertConfig{ AlertSeverity::ERROR, 90.0, 30000, true, std::chrono::steady_clock::time_point{} };
    alertConfigs_["cpu_usage"] = AlertConfig{ AlertSeverity::ERROR, 85.0, 30000, true, std::chrono::steady_clock::time_point{} };
    alertConfigs_["error_rate"] = AlertConfig{ AlertSeverity::WARNING, 5.0, 60000, true, std::chrono::steady_clock::time_point{} };
}

void AlertManager::initializeDefaultChannels() {
    std::unique_lock lock(mutex_);

    // Default logging channel
    addNotificationChannel("logger", [](const Alert& alert) {
        std::stringstream ss;
        ss << "ALERT [" << alert.id << "] " << alert.title
            << " | Severity: " << static_cast<int>(alert.severity)
            << " | Source: " << alert.source
            << " | Metric: " << alert.metric
            << " | Value: " << alert.value;

        switch (alert.severity) {
        case AlertSeverity::INFO:
            LOG_INFO(ss.str());
            break;
        case AlertSeverity::WARNING:
            LOG_WARN(ss.str());
            break;
        case AlertSeverity::ERROR:
            LOG_ERROR(ss.str());
            break;
        case AlertSeverity::CRITICAL:
            LOG_FATAL(ss.str());
            break;
        }
        });
}

void AlertManager::logAlert(const Alert& alert, const std::string& action) {
    std::stringstream ss;
    ss << "Alert " << action << " [ID: " << alert.id
        << ", Title: " << alert.title
        << ", Severity: " << static_cast<int>(alert.severity)
        << ", Source: " << alert.source << "]";

    LOG_INFO(ss.str());
}

void AlertManager::updateStats(const Alert& alert, AlertState oldState, AlertState newState) {
    // Stats are updated atomically in the main methods
    // This method can be extended for more complex stat tracking
    (void)alert;
    (void)oldState;
    (void)newState;
}

WEBSOCKET_NAMESPACE_END