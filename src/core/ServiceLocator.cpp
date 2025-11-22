#include "core/ServiceLocator.hpp"
#include "utils/Logger.hpp"

WEBSOCKET_NAMESPACE_BEGIN

ServiceLocator& ServiceLocator::getInstance() {
    static ServiceLocator instance;
    return instance;
}

ServiceLocator::ServiceLocator() {
    LOG_DEBUG("ServiceLocator created");
}

ServiceLocator::~ServiceLocator() {
    clearAllServices();
    LOG_DEBUG("ServiceLocator destroyed");
}

void ServiceLocator::unregisterService(const std::type_index& typeIndex) {
    std::unique_lock lock(mutex_);
    services_.erase(typeIndex);
    serviceNames_.erase(typeIndex);
    updateServiceCounts();
}

void ServiceLocator::clearAllServices() {
    std::unique_lock lock(mutex_);
    services_.clear();
    serviceNames_.clear();
    initializedServices_ = 0;
    runningServices_ = 0;
}

Result ServiceLocator::initializeAllServices() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            if (baseService->initialize() == Result::SUCCESS) {
                successCount++;
            }
        }
        catch (const std::bad_any_cast&) {
            LOG_ERROR("Failed to cast service for initialization");
        }
    }

    initializedServices_ = successCount;
    LOG_INFO("Initialized {}/{} services", successCount, services_.size());

    return successCount == services_.size() ? Result::SUCCESS : Result::ERROR;
}

Result ServiceLocator::startAllServices() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            if (baseService->start() == Result::SUCCESS) {
                successCount++;
            }
        }
        catch (const std::bad_any_cast&) {
            LOG_ERROR("Failed to cast service for starting");
        }
    }

    runningServices_ = successCount;
    LOG_INFO("Started {}/{} services", successCount, services_.size());

    return successCount == services_.size() ? Result::SUCCESS : Result::ERROR;
}

Result ServiceLocator::stopAllServices() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            if (baseService->stop() == Result::SUCCESS) {
                successCount++;
            }
        }
        catch (const std::bad_any_cast&) {
            LOG_ERROR("Failed to cast service for stopping");
        }
    }

    runningServices_ = 0;
    LOG_INFO("Stopped {}/{} services", successCount, services_.size());

    return Result::SUCCESS;
}

Result ServiceLocator::shutdownAllServices() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            if (baseService->shutdown() == Result::SUCCESS) {
                successCount++;
            }
        }
        catch (const std::bad_any_cast&) {
            LOG_ERROR("Failed to cast service for shutdown");
        }
    }

    initializedServices_ = 0;
    runningServices_ = 0;
    LOG_INFO("Shutdown {}/{} services", successCount, services_.size());

    return Result::SUCCESS;
}

Result ServiceLocator::resolveDependencies() {
    // Simple dependency resolution - in production would use topological sort
    std::shared_lock lock(mutex_);

    for (auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            auto dependencies = baseService->getDependencies();

            for (const auto& dep : dependencies) {
                // Check if dependency is available
                bool found = false;
                for (auto& [depType, depService] : services_) {
                    auto depBaseService = std::any_cast<SharedPtr<IService>>(depService);
                    if (depBaseService->getName() == dep) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    LOG_ERROR("Missing dependency: {} for service {}", dep, baseService->getName());
                    return Result::ERROR;
                }
            }
        }
        catch (const std::bad_any_cast&) {
            LOG_ERROR("Failed to cast service for dependency resolution");
        }
    }

    return Result::SUCCESS;
}

bool ServiceLocator::areDependenciesSatisfied() const {
    std::shared_lock lock(mutex_);

    for (auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            auto dependencies = baseService->getDependencies();

            for (const auto& dep : dependencies) {
                bool found = false;
                for (auto& [depType, depService] : services_) {
                    auto depBaseService = std::any_cast<SharedPtr<IService>>(depService);
                    if (depBaseService->getName() == dep) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }
        }
        catch (const std::bad_any_cast&) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> ServiceLocator::getUnsatisfiedDependencies() const {
    std::vector<std::string> unsatisfied;
    std::shared_lock lock(mutex_);

    for (auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            auto dependencies = baseService->getDependencies();

            for (const auto& dep : dependencies) {
                bool found = false;
                for (auto& [depType, depService] : services_) {
                    auto depBaseService = std::any_cast<SharedPtr<IService>>(depService);
                    if (depBaseService->getName() == dep) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    unsatisfied.push_back(dep + " for " + baseService->getName());
                }
            }
        }
        catch (const std::bad_any_cast&) {
            unsatisfied.push_back("Invalid service cast");
        }
    }

    return unsatisfied;
}

std::vector<std::type_index> ServiceLocator::getRegisteredServiceTypes() const {
    std::shared_lock lock(mutex_);
    std::vector<std::type_index> types;

    for (const auto& [type, service] : services_) {
        types.push_back(type);
    }

    return types;
}

std::vector<std::string> ServiceLocator::getRegisteredServiceNames() const {
    std::shared_lock lock(mutex_);
    std::vector<std::string> names;

    for (const auto& [type, name] : serviceNames_) {
        names.push_back(name);
    }

    return names;
}

size_t ServiceLocator::getServiceCount() const {
    std::shared_lock lock(mutex_);
    return services_.size();
}

size_t ServiceLocator::getInitializedServiceCount() const {
    return initializedServices_;
}

size_t ServiceLocator::getRunningServiceCount() const {
    return runningServices_;
}

ServiceLocator::ServiceInfo ServiceLocator::getAllServiceInfo() const {
    std::shared_lock lock(mutex_);
    std::vector<ServiceInfo> infoList;

    for (const auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            ServiceInfo info;
            info.type = type;
            info.name = baseService->getName();
            info.version = baseService->getVersion();
            info.isInitialized = baseService->isInitialized();
            info.isRunning = baseService->isRunning();
            infoList.push_back(info);
        }
        catch (const std::bad_any_cast&) {
            // Skip invalid services
        }
    }

    return infoList;
}

Error ServiceLocator::getLastError() const {
    return lastError_;
}

void ServiceLocator::clearErrors() {
    lastError_ = Error();
}

void ServiceLocator::updateServiceCounts() {
    std::shared_lock lock(mutex_);

    size_t initialized = 0;
    size_t running = 0;

    for (const auto& [type, service] : services_) {
        try {
            auto baseService = std::any_cast<SharedPtr<IService>>(service);
            if (baseService->isInitialized()) initialized++;
            if (baseService->isRunning()) running++;
        }
        catch (const std::bad_any_cast&) {
            // Skip invalid services
        }
    }

    initializedServices_ = initialized;
    runningServices_ = running;
}

WEBSOCKET_NAMESPACE_END