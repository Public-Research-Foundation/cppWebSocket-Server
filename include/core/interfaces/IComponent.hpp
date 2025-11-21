#pragma once

#include "../../common/Types.hpp"

WEBSOCKET_NAMESPACE_BEGIN

enum class ComponentState {
    CREATED,
    CONFIGURED,
    INITIALIZED,
    STARTED,
    STOPPED,
    DESTROYED,
    ERROR
};

class IComponent {
    WEBSOCKET_INTERFACE(IComponent)

public:
    // Component lifecycle
    virtual Result initialize() = 0;
    virtual Result configure() = 0;
    virtual Result start() = 0;
    virtual Result stop() = 0;
    virtual Result destroy() = 0;

    // Component state
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual ComponentState getState() const = 0;
    virtual bool isOperational() const = 0;

    // Component identification
    virtual std::string getComponentName() const = 0;
    virtual std::string getComponentVersion() const = 0;
    virtual std::string getComponentType() const = 0;
    virtual std::string getComponentId() const = 0;

    // Dependencies and relationships
    virtual std::vector<std::string> getDependencies() const = 0;
    virtual std::vector<std::string> getDependents() const = 0;
    virtual bool hasDependency(const std::string& componentId) const = 0;

    // Configuration
    virtual void setConfigurationParameter(const std::string& key, const std::any& value) = 0;
    virtual std::any getConfigurationParameter(const std::string& key) const = 0;
    virtual bool hasConfigurationParameter(const std::string& key) const = 0;

    // Health and metrics
    virtual bool isHealthy() const = 0;
    virtual std::string getHealthStatus() const = 0;
    virtual std::unordered_map<std::string, std::any> getMetrics() const = 0;

    // Error handling
    virtual Error getLastError() const = 0;
    virtual std::vector<Error> getErrorHistory() const = 0;
    virtual void clearErrors() = 0;

    // Resource management
    virtual size_t getMemoryUsage() const = 0;
    virtual size_t getResourceCount() const = 0;
    virtual void setResourceLimit(const std::string& resourceType, size_t limit) = 0;
};

WEBSOCKET_NAMESPACE_END