#pragma once

#include "../common/Types.hpp"
#include "interfaces/IInitializable.hpp"
#include "interfaces/IShutdownHandler.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class LifecycleManager {
public:
    static LifecycleManager& getInstance();

    // Initialization management
    Result initializeAll();
    Result preInitializeAll();
    Result postInitializeAll();

    // Shutdown management
    Result shutdownAll();
    Result gracefulShutdownAll();
    Result emergencyShutdownAll();

    // Component registration
    void registerInitializable(const SharedPtr<IInitializable>& initializable);
    void registerShutdownHandler(const SharedPtr<IShutdownHandler>& handler);

    void unregisterInitializable(const SharedPtr<IInitializable>& initializable);
    void unregisterShutdownHandler(const SharedPtr<IShutdownHandler>& handler);

    // State queries
    bool isSystemInitialized() const;
    bool isShutdownInProgress() const;
    bool isEmergencyShutdownInProgress() const;

    // Dependency management
    Result resolveInitializationDependencies();
    Result resolveShutdownDependencies();
    bool areInitializationDependenciesSatisfied() const;
    bool areShutdownDependenciesSatisfied() const;

    // Progress monitoring
    double getInitializationProgress() const;
    double getShutdownProgress() const;
    std::string getInitializationStatus() const;
    std::string getShutdownStatus() const;

    // Timeout management
    void setInitializationTimeout(uint32_t timeoutMs);
    void setShutdownTimeout(uint32_t timeoutMs);
    void setEmergencyShutdownTimeout(uint32_t timeoutMs);

    // Error handling
    Error getLastInitializationError() const;
    Error getLastShutdownError() const;
    std::vector<Error> getAllInitializationErrors() const;
    std::vector<Error> getAllShutdownErrors() const;
    void clearErrors();

    // Statistics
    size_t getInitializableCount() const;
    size_t getShutdownHandlerCount() const;
    size_t getInitializedCount() const;
    size_t getShutdownCount() const;

private:
    LifecycleManager();
    ~LifecycleManager() = default;

    mutable std::shared_mutex initMutex_;
    mutable std::shared_mutex shutdownMutex_;

    std::vector<SharedPtr<IInitializable>> initializables_;
    std::vector<SharedPtr<IShutdownHandler>> shutdownHandlers_;

    std::atomic<bool> systemInitialized_{ false };
    std::atomic<bool> shutdownInProgress_{ false };
    std::atomic<bool> emergencyShutdown_{ false };

    std::atomic<uint32_t> initializationTimeout_{ 30000 }; // 30 seconds
    std::atomic<uint32_t> shutdownTimeout_{ 15000 }; // 15 seconds
    std::atomic<uint32_t> emergencyShutdownTimeout_{ 5000 }; // 5 seconds

    Error lastInitializationError_;
    Error lastShutdownError_;
    std::vector<Error> initializationErrors_;
    std::vector<Error> shutdownErrors_;

    void sortInitializablesByPriority();
    void sortShutdownHandlersByPriority();

    Result initializeComponent(const SharedPtr<IInitializable>& initializable);
    Result shutdownComponent(const SharedPtr<IShutdownHandler>& handler);

    bool checkInitializationDependencies(const SharedPtr<IInitializable>& initializable) const;
    bool checkShutdownDependencies(const SharedPtr<IShutdownHandler>& handler) const;

    void updateInitializationProgress();
    void updateShutdownProgress();
};

WEBSOCKET_NAMESPACE_END