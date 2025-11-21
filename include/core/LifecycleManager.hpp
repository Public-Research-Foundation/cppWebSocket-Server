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

    Result initializeAll();
    Result shutdownAll();
    Result emergencyShutdown();

    void registerInitializable(const SharedPtr<IInitializable>& initializable);
    void registerShutdownHandler(const SharedPtr<IShutdownHandler>& handler);

    void unregisterInitializable(const SharedPtr<IInitializable>& initializable);
    void unregisterShutdownHandler(const SharedPtr<IShutdownHandler>& handler);

    bool isSystemInitialized() const;
    bool isShutdownInProgress() const;

private:
    LifecycleManager() = default;
    ~LifecycleManager() = default;

    mutable std::shared_mutex initMutex_;
    mutable std::shared_mutex shutdownMutex_;

    std::vector<SharedPtr<IInitializable>> initializables_;
    std::vector<SharedPtr<IShutdownHandler>> shutdownHandlers_;

    std::atomic<bool> systemInitialized_{ false };
    std::atomic<bool> shutdownInProgress_{ false };

    void sortShutdownHandlersByPriority();
};

WEBSOCKET_NAMESPACE_END