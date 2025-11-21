#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <unordered_map>
#include <typeindex>
#include <any>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class ServiceLocator {
public:
    static ServiceLocator& getInstance();

    // Service registration and retrieval
    template<typename T>
    void registerService(const SharedPtr<T>& service);

    template<typename T>
    SharedPtr<T> getService() const;

    template<typename T>
    bool hasService() const;

    void unregisterService(const std::type_index& typeIndex);
    void clearAllServices();

    // Service lifecycle management
    Result initializeAllServices();
    Result startAllServices();
    Result stopAllServices();
    Result shutdownAllServices();

    // Service discovery
    std::vector<std::type_index> getRegisteredServiceTypes() const;
    std::vector<std::string> getRegisteredServiceNames() const;

    // Service dependencies
    Result resolveDependencies();
    bool areDependenciesSatisfied() const;
    std::vector<std::string> getUnsatisfiedDependencies() const;

    // Service statistics
    size_t getServiceCount() const;
    size_t getInitializedServiceCount() const;
    size_t getRunningServiceCount() const;

    struct ServiceInfo {
        std::type_index type;
        std::string name;
        std::string version;
        bool isInitialized;
        bool isRunning;
    };

    std::vector<ServiceInfo> getAllServiceInfo() const;

    // Error handling
    Error getLastError() const;
    void clearErrors();

private:
    ServiceLocator();
    ~ServiceLocator();

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::type_index, std::any> services_;
    std::unordered_map<std::type_index, std::string> serviceNames_;

    std::atomic<size_t> initializedServices_{ 0 };
    std::atomic<size_t> runningServices_{ 0 };
    Error lastError_;

    template<typename T>
    std::string getServiceName() const;

    void updateServiceCounts();
};

// Template implementations
template<typename T>
void ServiceLocator::registerService(const SharedPtr<T>& service) {
    std::unique_lock lock(mutex_);
    std::type_index typeIndex = std::type_index(typeid(T));
    services_[typeIndex] = service;
    serviceNames_[typeIndex] = typeid(T).name();
}

template<typename T>
SharedPtr<T> ServiceLocator::getService() const {
    std::shared_lock lock(mutex_);
    auto it = services_.find(std::type_index(typeid(T)));
    if (it != services_.end()) {
        try {
            return std::any_cast<SharedPtr<T>>(it->second);
        }
        catch (const std::bad_any_cast&) {
            return nullptr;
        }
    }
    return nullptr;
}

template<typename T>
bool ServiceLocator::hasService() const {
    std::shared_lock lock(mutex_);
    return services_.find(std::type_index(typeid(T))) != services_.end();
}

template<typename T>
std::string ServiceLocator::getServiceName() const {
    std::shared_lock lock(mutex_);
    auto it = serviceNames_.find(std::type_index(typeid(T)));
    return it != serviceNames_.end() ? it->second : "Unknown";
}

WEBSOCKET_NAMESPACE_END