#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

enum class ServiceState {
    UNINITIALIZED,
    INITIALIZING,
    INITIALIZED,
    STARTING,
    RUNNING,
    STOPPING,
    STOPPED,
    SHUTDOWN,
    ERROR
};

class IService {
    WEBSOCKET_INTERFACE(IService)

public:
    // Service lifecycle management
    virtual Result initialize() = 0;
    virtual Result start() = 0;
    virtual Result stop() = 0;
    virtual Result shutdown() = 0;

    // Service state management
    virtual bool isRunning() const = 0;
    virtual bool isInitialized() const = 0;
    virtual ServiceState getState() const = 0;
    virtual std::string getStatusMessage() const = 0;

    // Service identification
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;

    // Service dependencies
    virtual std::vector<std::string> getDependencies() const = 0;
    virtual bool hasDependency(const std::string& serviceName) const = 0;

    // Error handling
    virtual Error getLastError() const = 0;
    virtual void clearError() = 0;
    virtual bool hasError() const = 0;

    // Configuration
    virtual void setConfiguration(const std::unordered_map<std::string, std::any>& config) = 0;
    virtual std::unordered_map<std::string, std::any> getConfiguration() const = 0;

    // Statistics
    virtual std::chrono::steady_clock::time_point getStartTime() const = 0;
    virtual std::chrono::duration<double> getUptime() const = 0;
    virtual uint64_t getRequestCount() const = 0;
    virtual uint64_t getErrorCount() const = 0;
};

WEBSOCKET_NAMESPACE_END