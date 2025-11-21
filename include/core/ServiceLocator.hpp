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

    template<typename T>
    void registerService(const SharedPtr<T>& service);

    template<typename T>
    SharedPtr<T> getService() const;

    template<typename T>
    bool hasService() const;

    void unregisterService(const std::type_index& typeIndex);
    void clearAllServices();

    size_t getServiceCount() const;
    std::vector<std::string> getRegisteredServiceNames() const;

private:
    ServiceLocator() = default;
    ~ServiceLocator() = default;

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::type_index, std::any> services_;
};

// Template implementations
template<typename T>
void ServiceLocator::registerService(const SharedPtr<T>& service) {
    std::unique_lock lock(mutex_);
    services_[std::type_index(typeid(T))] = service;
}

template<typename T>
SharedPtr<T> ServiceLocator::getService() const {
    std::shared_lock lock(mutex_);
    auto it = services_.find(std::type_index(typeid(T)));
    if (it != services_.end()) {
        return std::any_cast<SharedPtr<T>>(it->second);
    }
    return nullptr;
}

template<typename T>
bool ServiceLocator::hasService() const {
    std::shared_lock lock(mutex_);
    return services_.find(std::type_index(typeid(T))) != services_.end();
}

WEBSOCKET_NAMESPACE_END