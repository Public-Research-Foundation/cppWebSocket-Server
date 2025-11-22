#include "monitoring/HealthCheck.hpp"
#include "utils/Logger.hpp"
#include "utils/Metrics.hpp"
#include <thread>
#include <chrono>

WEBSOCKET_NAMESPACE_BEGIN

// Singleton instance access
HealthCheckManager& HealthCheckManager::getInstance() {
    static HealthCheckManager instance;
    return instance;
}

HealthCheckManager::HealthCheckManager() {
    LOG_DEBUG("HealthCheckManager created");
}

HealthCheckManager::~HealthCheckManager() {
    stopMonitoring();
    LOG_DEBUG("HealthCheckManager destroyed");
}

// Register a new health check with specified interval
void HealthCheckManager::registerCheck(const std::string& name,
    std::function<HealthCheckResult()> checkFunction,
    uint32_t intervalMs,
    bool enabled) {
    std::unique_lock lock(mutex_);

    HealthCheck check;
    check.name = name;
    check.checkFunction = checkFunction;
    check.intervalMs = intervalMs;
    check.timeoutMs = 5000; // Default 5 second timeout
    check.enabled = enabled;
    check.critical = false; // Default to non-critical
    check.warningThreshold = 0.0;
    check.criticalThreshold = 0.0;
    check.lastStatus.isHealthy = false;
    check.lastStatus.status = "Not executed";
    check.lastStatus.message = "Check not yet executed";
    check.lastStatus.lastCheck = std::chrono::steady_clock::now();
    check.lastStatus.responseTime = std::chrono::milliseconds(0);
    check.executionCount = 0;
    check.failureCount = 0;

    checks_[name] = check;
    LOG_DEBUG("Registered health check: {}", name);
}

// Remove a health check
void HealthCheckManager::unregisterCheck(const std::string& name) {
    std::unique_lock lock(mutex_);
    checks_.erase(name);
    LOG_DEBUG("Unregistered health check: {}", name);
}

// Enable a specific health check
void HealthCheckManager::enableCheck(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        it->second.enabled = true;
        LOG_DEBUG("Enabled health check: {}", name);
    }
}

// Disable a specific health check
void HealthCheckManager::disableCheck(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        it->second.enabled = false;
        LOG_DEBUG("Disabled health check: {}", name);
    }
}

// Execute a specific health check by name
HealthStatus HealthCheckManager::performCheck(const std::string& name) {
    std::shared_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it == checks_.end()) {
        HealthStatus status;
        status.isHealthy = false;
        status.status = "ERROR";
        status.message = "Health check not found: " + name;
        status.lastCheck = std::chrono::steady_clock::now();
        status.responseTime = std::chrono::milliseconds(0);
        return status;
    }

    HealthCheckResult result = executeCheck(it->second);
    updateHealthStatus(name, result);
    return it->second.lastStatus;
}

// Execute all registered health checks
HealthStatus HealthCheckManager::performAllChecks() {
    std::shared_lock lock(mutex_);

    std::vector<HealthStatus> allStatuses;
    for (auto& [name, check] : checks_) {
        if (check.enabled && shouldExecuteCheck(check)) {
            HealthCheckResult result = executeCheck(check);
            updateHealthStatus(name, result);
            allStatuses.push_back(check.lastStatus);
        }
    }

    return aggregateHealthStatus(allStatuses);
}

// Execute only critical health checks
HealthStatus HealthCheckManager::performCriticalChecks() {
    std::shared_lock lock(mutex_);

    std::vector<HealthStatus> criticalStatuses;
    for (auto& [name, check] : checks_) {
        if (check.enabled && check.critical && shouldExecuteCheck(check)) {
            HealthCheckResult result = executeCheck(check);
            updateHealthStatus(name, result);
            criticalStatuses.push_back(check.lastStatus);
        }
    }

    return aggregateHealthStatus(criticalStatuses);
}

// Start automatic health monitoring
void HealthCheckManager::startMonitoring() {
    if (monitoring_) {
        LOG_WARN("Health monitoring already running");
        return;
    }

    monitoring_ = true;
    monitoringThread_ = std::thread(&HealthCheckManager::monitoringThreadFunction, this);
    LOG_INFO("Health monitoring started");
}

// Stop automatic health monitoring
void HealthCheckManager::stopMonitoring() {
    if (!monitoring_) {
        return;
    }

    monitoring_ = false;
    if (monitoringThread_.joinable()) {
        monitoringThread_.join();
    }
    LOG_INFO("Health monitoring stopped");
}

bool HealthCheckManager::isMonitoring() const {
    return monitoring_;
}

// Get overall system health status
HealthStatus HealthCheckManager::getOverallHealth() const {
    std::shared_lock lock(mutex_);

    std::vector<HealthStatus> allStatuses;
    for (const auto& [name, check] : checks_) {
        if (check.enabled) {
            allStatuses.push_back(check.lastStatus);
        }
    }

    return aggregateHealthStatus(allStatuses);
}

// Get health status for a specific check
HealthStatus HealthCheckManager::getCheckHealth(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        return it->second.lastStatus;
    }

    HealthStatus status;
    status.isHealthy = false;
    status.status = "NOT_FOUND";
    status.message = "Health check not found";
    return status;
}

// Get health status for all checks
std::unordered_map<std::string, HealthStatus> HealthCheckManager::getAllHealthStatuses() const {
    std::shared_lock lock(mutex_);
    std::unordered_map<std::string, HealthStatus> statuses;

    for (const auto& [name, check] : checks_) {
        statuses[name] = check.lastStatus;
    }

    return statuses;
}

// Configure check interval
void HealthCheckManager::setCheckInterval(const std::string& name, uint32_t intervalMs) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        it->second.intervalMs = intervalMs;
    }
}

// Configure check timeout
void HealthCheckManager::setCheckTimeout(const std::string& name, uint32_t timeoutMs) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        it->second.timeoutMs = timeoutMs;
    }
}

// Mark a check as critical
void HealthCheckManager::setCriticalCheck(const std::string& name, bool critical) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        it->second.critical = critical;
    }
}

// Set warning threshold for a check
void HealthCheckManager::setWarningThreshold(const std::string& name, double threshold) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        it->second.warningThreshold = threshold;
    }
}

// Set critical threshold for a check
void HealthCheckManager::setCriticalThreshold(const std::string& name, double threshold) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        it->second.criticalThreshold = threshold;
    }
}

// Add dependency between health checks
void HealthCheckManager::addCheckDependency(const std::string& checkName, const std::string& dependencyName) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(checkName);
    if (it != checks_.end()) {
        it->second.dependencies.push_back(dependencyName);
    }
}

// Remove dependency between health checks
void HealthCheckManager::removeCheckDependency(const std::string& checkName, const std::string& dependencyName) {
    std::unique_lock lock(mutex_);
    auto it = checks_.find(checkName);
    if (it != checks_.end()) {
        auto& deps = it->second.dependencies;
        deps.erase(std::remove(deps.begin(), deps.end(), dependencyName), deps.end());
    }
}

// Get dependencies for a check
std::vector<std::string> HealthCheckManager::getCheckDependencies(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = checks_.find(name);
    if (it != checks_.end()) {
        return it->second.dependencies;
    }
    return {};
}

// Get health monitoring statistics
HealthCheckManager::HealthStats HealthCheckManager::getStats() const {
    std::shared_lock lock(mutex_);

    HealthStats stats;
    stats.totalChecks = checks_.size();
    stats.enabledChecks = 0;
    stats.criticalChecks = 0;
    stats.healthyChecks = 0;
    stats.warningChecks = 0;
    stats.criticalFailedChecks = 0;

    for (const auto& [name, check] : checks_) {
        if (check.enabled) {
            stats.enabledChecks++;
        }
        if (check.critical) {
            stats.criticalChecks++;
        }
        if (check.lastStatus.isHealthy) {
            stats.healthyChecks++;
        }
        if (check.lastStatus.status == "WARNING") {
            stats.warningChecks++;
        }
        if (check.critical && !check.lastStatus.isHealthy) {
            stats.criticalFailedChecks++;
        }
    }

    stats.totalExecutions = totalExecutions_;
    stats.failedExecutions = failedExecutions_;

    return stats;
}

// Set handler for health status changes
void HealthCheckManager::setOnHealthChangeHandler(std::function<void(const std::string&, const HealthStatus&)> handler) {
    onHealthChangeHandler_ = handler;
}

// Set handler for critical failures
void HealthCheckManager::setOnCriticalFailureHandler(std::function<void(const std::string&, const HealthCheckResult&)> handler) {
    onCriticalFailureHandler_ = handler;
}

// Generate health report
std::string HealthCheckManager::generateHealthReport() const {
    std::shared_lock lock(mutex_);

    std::ostringstream report;
    report << "Health Check Report\n";
    report << "===================\n";

    auto overall = getOverallHealth();
    report << "Overall Health: " << (overall.isHealthy ? "HEALTHY" : "UNHEALTHY") << "\n";
    report << "Overall Status: " << overall.status << "\n";
    report << "Message: " << overall.message << "\n\n";

    report << "Detailed Checks:\n";
    for (const auto& [name, check] : checks_) {
        if (check.enabled) {
            report << "  " << name << ":\n";
            report << "    Status: " << check.lastStatus.status << "\n";
            report << "    Healthy: " << (check.lastStatus.isHealthy ? "Yes" : "No") << "\n";
            report << "    Message: " << check.lastStatus.message << "\n";
            report << "    Last Check: " << std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - check.lastStatus.lastCheck).count() << "s ago\n";
            report << "    Response Time: " << check.lastStatus.responseTime.count() << "ms\n";
            report << "    Critical: " << (check.critical ? "Yes" : "No") << "\n";
            report << "    Executions: " << check.executionCount << "\n";
            report << "    Failures: " << check.failureCount << "\n";
            report << "\n";
        }
    }

    return report.str();
}

// Get health metrics for monitoring systems
std::unordered_map<std::string, std::any> HealthCheckManager::getHealthMetrics() const {
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, std::any> metrics;
    metrics["health.checks.total"] = checks_.size();
    metrics["health.checks.enabled"] = getStats().enabledChecks;
    metrics["health.checks.healthy"] = getStats().healthyChecks;
    metrics["health.checks.unhealthy"] = getStats().enabledChecks - getStats().healthyChecks;
    metrics["health.checks.critical_failed"] = getStats().criticalFailedChecks;
    metrics["health.executions.total"] = totalExecutions_.load();
    metrics["health.executions.failed"] = failedExecutions_.load();

    auto overall = getOverallHealth();
    metrics["health.overall.healthy"] = overall.isHealthy;
    metrics["health.overall.status"] = overall.status;

    return metrics;
}

// Private methods

// Monitoring thread function
void HealthCheckManager::monitoringThreadFunction() {
    LOG_DEBUG("Health monitoring thread started");

    while (monitoring_) {
        auto startTime = std::chrono::steady_clock::now();

        // Perform all enabled checks
        performAllChecks();

        // Calculate sleep time to maintain consistent interval
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto sleepTime = std::chrono::milliseconds(1000) - elapsed; // Check every second

        if (sleepTime.count() > 0) {
            std::this_thread::sleep_for(sleepTime);
        }
    }

    LOG_DEBUG("Health monitoring thread stopped");
}

// Execute a single health check
HealthCheckResult HealthCheckManager::executeCheck(const HealthCheck& check) {
    auto startTime = std::chrono::steady_clock::now();

    HealthCheckResult result;
    result.checkName = check.name;

    try {
        // Check dependencies first
        if (!checkDependencies(check)) {
            result.passed = false;
            result.message = "Dependencies not satisfied";
            return result;
        }

        // Execute the check function
        result = check.checkFunction();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);

    }
    catch (const std::exception& e) {
        result.passed = false;
        result.message = std::string("Exception: ") + e.what();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);
    }

    totalExecutions_++;
    if (!result.passed) {
        failedExecutions_++;
    }

    return result;
}

// Check if a health check should be executed based on its interval
bool HealthCheckManager::shouldExecuteCheck(const HealthCheck& check) const {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastCheck = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - check.lastExecution);
    return timeSinceLastCheck.count() >= check.intervalMs;
}

// Update health status after check execution
void HealthCheckManager::updateHealthStatus(const std::string& name, const HealthCheckResult& result) {
    auto it = checks_.find(name);
    if (it == checks_.end()) {
        return;
    }

    HealthCheck& check = it->second;
    HealthStatus oldStatus = check.lastStatus;

    // Update check statistics
    check.executionCount++;
    check.lastExecution = std::chrono::steady_clock::now();

    if (!result.passed) {
        check.failureCount++;
    }

    // Update health status
    check.lastStatus.isHealthy = result.passed;
    check.lastStatus.status = result.passed ? "HEALTHY" : "UNHEALTHY";
    check.lastStatus.message = result.message;
    check.lastStatus.lastCheck = std::chrono::steady_clock::now();
    check.lastStatus.responseTime = result.duration;
    check.lastStatus.details = result.metadata;

    // Notify health change
    if (oldStatus.isHealthy != check.lastStatus.isHealthy ||
        oldStatus.status != check.lastStatus.status) {
        notifyHealthChange(name, check.lastStatus);
    }

    // Notify critical failure
    if (check.critical && !check.lastStatus.isHealthy) {
        notifyCriticalFailure(name, result);
    }

    // Update metrics
    METRICS_INCREMENT("health_checks_executed");
    if (!result.passed) {
        METRICS_INCREMENT("health_checks_failed");
    }
}

// Notify health status change to registered handlers
void HealthCheckManager::notifyHealthChange(const std::string& name, const HealthStatus& status) {
    if (onHealthChangeHandler_) {
        try {
            onHealthChangeHandler_(name, status);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Health change handler failed: {}", e.what());
        }
    }

    LOG_INFO("Health status changed: {} -> {}", name, status.status);
}

// Notify critical failure to registered handlers
void HealthCheckManager::notifyCriticalFailure(const std::string& name, const HealthCheckResult& result) {
    if (onCriticalFailureHandler_) {
        try {
            onCriticalFailureHandler_(name, result);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Critical failure handler failed: {}", e.what());
        }
    }

    LOG_ERROR("Critical health check failed: {} - {}", name, result.message);
}

// Check if all dependencies are satisfied for a health check
bool HealthCheckManager::checkDependencies(const HealthCheck& check) const {
    for (const auto& depName : check.dependencies) {
        auto depIt = checks_.find(depName);
        if (depIt == checks_.end() || !depIt->second.lastStatus.isHealthy) {
            return false;
        }
    }
    return true;
}

// Aggregate multiple health statuses into an overall status
HealthStatus HealthCheckManager::aggregateHealthStatus(const std::vector<HealthStatus>& statuses) const {
    HealthStatus overall;
    overall.lastCheck = std::chrono::steady_clock::now();

    if (statuses.empty()) {
        overall.isHealthy = true;
        overall.status = "HEALTHY";
        overall.message = "No health checks configured";
        return overall;
    }

    // Count status types
    size_t healthy = 0;
    size_t warning = 0;
    size_t unhealthy = 0;

    for (const auto& status : statuses) {
        if (status.isHealthy) {
            healthy++;
        }
        else if (status.status == "WARNING") {
            warning++;
        }
        else {
            unhealthy++;
        }
    }

    // Determine overall status
    if (unhealthy > 0) {
        overall.isHealthy = false;
        overall.status = "UNHEALTHY";
        overall.message = std::to_string(unhealthy) + " checks failed";
    }
    else if (warning > 0) {
        overall.isHealthy = true; // Warnings don't make overall unhealthy
        overall.status = "WARNING";
        overall.message = std::to_string(warning) + " checks have warnings";
    }
    else {
        overall.isHealthy = true;
        overall.status = "HEALTHY";
        overall.message = "All checks passed";
    }

    // Add details
    overall.details["healthy_checks"] = healthy;
    overall.details["warning_checks"] = warning;
    overall.details["unhealthy_checks"] = unhealthy;
    overall.details["total_checks"] = statuses.size();

    return overall;
}

WEBSOCKET_NAMESPACE_END