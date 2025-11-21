#pragma once

#include "../../common/Types.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IInitializable {
    WEBSOCKET_INTERFACE(IInitializable)

public:
    // Initialization phases
    virtual Result preInitialize() = 0;
    virtual Result initialize() = 0;
    virtual Result postInitialize() = 0;

    // Initialization state
    virtual bool isInitialized() const = 0;
    virtual bool isPreInitialized() const = 0;
    virtual bool isPostInitialized() const = 0;
    virtual std::string getInitializationPhase() const = 0;

    // Dependencies
    virtual std::vector<std::string> getDependencies() const = 0;
    virtual bool hasDependency(const std::string& name) const = 0;
    virtual bool areDependenciesSatisfied() const = 0;

    // Initialization order
    virtual int getInitializationPriority() const = 0;
    virtual void setInitializationPriority(int priority) = 0;

    // Error handling during initialization
    virtual Error getInitializationError() const = 0;
    virtual std::vector<std::string> getInitializationWarnings() const = 0;
    virtual void clearInitializationErrors() = 0;

    // Initialization events
    virtual void onDependenciesReady() = 0;
    virtual void onInitializationComplete() = 0;
    virtual void onInitializationFailed(const Error& error) = 0;

    // Reinitialization support
    virtual bool supportsReinitialization() const = 0;
    virtual Result reinitialize() = 0;
    virtual bool isReinitializationRequired() const = 0;

    // Resource cleanup
    virtual Result cleanup() = 0;
    virtual bool requiresCleanup() const = 0;
};

WEBSOCKET_NAMESPACE_END