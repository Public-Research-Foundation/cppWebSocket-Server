#pragma once

#include "../../common/Types.hpp"

WEBSOCKET_NAMESPACE_BEGIN

enum class ShutdownPhase {
    PRE_SHUTDOWN,    // Prepare for shutdown
    GRACEFUL,        // Graceful shutdown phase
    FORCEFUL,        // Forceful shutdown phase
    CLEANUP,         // Final cleanup phase
    COMPLETE         // Shutdown complete
};

class IShutdownHandler {
    WEBSOCKET_INTERFACE(IShutdownHandler)

public:
    // Shutdown management
    virtual void onShutdown() = 0;
    virtual void onEmergencyShutdown() = 0;
    virtual void onGracefulShutdown() = 0;

    // Shutdown state
    virtual bool canShutdown() const = 0;
    virtual bool isShuttingDown() const = 0;
    virtual ShutdownPhase getShutdownPhase() const = 0;

    // Shutdown priority and ordering
    virtual int getShutdownPriority() const = 0;
    virtual void setShutdownPriority(int priority) = 0;

    // Resource cleanup
    virtual Result cleanupResources() = 0;
    virtual bool hasPendingOperations() const = 0;
    virtual uint32_t getPendingOperationCount() const = 0;

    // Timeout management
    virtual uint32_t getShutdownTimeout() const = 0;
    virtual void setShutdownTimeout(uint32_t timeoutMs) = 0;
    virtual bool isShutdownTimeoutExceeded() const = 0;

    // Dependency management
    virtual std::vector<std::string> getShutdownDependencies() const = 0;
    virtual bool hasShutdownDependency(const std::string& handlerName) const = 0;
    virtual bool areShutdownDependenciesSatisfied() const = 0;

    // Error handling during shutdown
    virtual Error getShutdownError() const = 0;
    virtual std::vector<Error> getShutdownErrors() const = 0;
    virtual void clearShutdownErrors() = 0;

    // Shutdown progress monitoring
    virtual double getShutdownProgress() const = 0;
    virtual std::string getShutdownStatus() const = 0;
    virtual bool isShutdownComplete() const = 0;

    // Emergency shutdown
    virtual bool supportsEmergencyShutdown() const = 0;
    virtual uint32_t getEmergencyShutdownTimeout() const = 0;
};

WEBSOCKET_NAMESPACE_END