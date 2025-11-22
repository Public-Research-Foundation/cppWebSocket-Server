#include "core/ComponentManager.hpp"
#include "utils/Logger.hpp"

WEBSOCKET_NAMESPACE_BEGIN

ComponentManager::ComponentManager() {
    LOG_DEBUG("ComponentManager created");
}

ComponentManager::~ComponentManager() {
    shutdownAll();
    LOG_DEBUG("ComponentManager destroyed");
}

Result ComponentManager::initializeAll() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (const auto& component : components_) {
        if (initializeComponent(component) == Result::SUCCESS) {
            successCount++;
        }
    }

    initializedCount_ = successCount;
    LOG_INFO("Initialized {}/{} components", successCount, components_.size());

    return successCount == components_.size() ? Result::SUCCESS : Result::ERROR;
}

Result ComponentManager::startAll() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (const auto& component : components_) {
        if (startComponent(component) == Result::SUCCESS) {
            successCount++;
        }
    }

    runningCount_ = successCount;
    LOG_INFO("Started {}/{} components", successCount, components_.size());

    return successCount == components_.size() ? Result::SUCCESS : Result::ERROR;
}

Result ComponentManager::stopAll() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (const auto& component : components_) {
        if (stopComponent(component) == Result::SUCCESS) {
            successCount++;
        }
    }

    runningCount_ = 0;
    LOG_INFO("Stopped {}/{} components", successCount, components_.size());

    return Result::SUCCESS;
}

Result ComponentManager::shutdownAll() {
    std::shared_lock lock(mutex_);

    size_t successCount = 0;
    for (const auto& component : components_) {
        if (shutdownComponent(component) == Result::SUCCESS) {
            successCount++;
        }
    }

    initializedCount_ = 0;
    runningCount_ = 0;
    healthyCount_ = 0;
    LOG_INFO("Shutdown {}/{} components", successCount, components_.size());

    return Result::SUCCESS;
}

void ComponentManager::updateAll() {
    std::shared_lock lock(mutex_);

    for (const auto& component : components_) {
        if (component->isEnabled() && component->isOperational()) {
            component->update();
        }
    }
}

SharedPtr<IComponent> ComponentManager::getComponentByName(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = namedComponents_.find(name);
    return it != namedComponents_.end() ? it->second : nullptr;
}

SharedPtr<IComponent> ComponentManager::getComponentById(const std::string& id) const {
    std::shared_lock lock(mutex_);
    auto it = idComponents_.find(id);
    return it != idComponents_.end() ? it->second : nullptr;
}

void ComponentManager::enableComponent(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = namedComponents_.find(name);
    if (it != namedComponents_.end()) {
        it->second->setEnabled(true);
        updateComponentCounts();
    }
}

void ComponentManager::disableComponent(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = namedComponents_.find(name);
    if (it != namedComponents_.end()) {
        it->second->setEnabled(false);
        updateComponentCounts();
    }
}

void ComponentManager::enableAllComponents() {
    std::unique_lock lock(mutex_);
    for (const auto& component : components_) {
        component->setEnabled(true);
    }
    updateComponentCounts();
}

void ComponentManager::disableAllComponents() {
    std::unique_lock lock(mutex_);
    for (const auto& component : components_) {
        component->setEnabled(false);
    }
    updateComponentCounts();
}

Result ComponentManager::resolveDependencies() {
    std::shared_lock lock(mutex_);

    for (const auto& component : components_) {
        auto dependencies = component->getDependencies();
        for (const auto& dep : dependencies) {
            if (!checkDependency(component, dep)) {
                LOG_ERROR("Missing dependency: {} for component {}", dep, component->getComponentName());
                return Result::ERROR;
            }
        }
    }

    return Result::SUCCESS;
}

bool ComponentManager::validateDependencies() const {
    std::shared_lock lock(mutex_);

    for (const auto& component : components_) {
        auto dependencies = component->getDependencies();
        for (const auto& dep : dependencies) {
            if (!checkDependency(component, dep)) {
                return false;
            }
        }
    }

    return true;
}

std::vector<std::string> ComponentManager::getDependencyErrors() const {
    std::vector<std::string> errors;
    std::shared_lock lock(mutex_);

    for (const auto& component : components_) {
        auto dependencies = component->getDependencies();
        for (const auto& dep : dependencies) {
            if (!checkDependency(component, dep)) {
                errors.push_back("Missing dependency: " + dep + " for " + component->getComponentName());
            }
        }
    }

    return errors;
}

bool ComponentManager::areAllComponentsHealthy() const {
    std::shared_lock lock(mutex_);

    for (const auto& component : components_) {
        if (component->isEnabled() && !component->isHealthy()) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> ComponentManager::getUnhealthyComponents() const {
    std::vector<std::string> unhealthy;
    std::shared_lock lock(mutex_);

    for (const auto& component : components_) {
        if (component->isEnabled() && !component->isHealthy()) {
            unhealthy.push_back(component->getComponentName());
        }
    }

    return unhealthy;
}

std::unordered_map<std::string, std::string> ComponentManager::getComponentHealthStatus() const {
    std::unordered_map<std::string, std::string> healthStatus;
    std::shared_lock lock(mutex_);

    for (const auto& component : components_) {
        healthStatus[component->getComponentName()] = component->getHealthStatus();
    }

    return healthStatus;
}

size_t ComponentManager::getComponentCount() const {
    std::shared_lock lock(mutex_);
    return components_.size();
}

size_t ComponentManager::getEnabledComponentCount() const {
    return enabledCount_;
}

size_t ComponentManager::getInitializedComponentCount() const {
    return initializedCount_;
}

size_t ComponentManager::getRunningComponentCount() const {
    return runningCount_;
}

std::vector<std::string> ComponentManager::getComponentNames() const {
    std::shared_lock lock(mutex_);
    std::vector<std::string> names;

    for (const auto& component : components_) {
        names.push_back(component->getComponentName());
    }

    return names;
}

std::vector<std::string> ComponentManager::getComponentNamesByType(const std::string& type) const {
    std::shared_lock lock(mutex_);
    std::vector<std::string> names;

    for (const auto& component : components_) {
        if (component->getComponentType() == type) {
            names.push_back(component->getComponentName());
        }
    }

    return names;
}

ComponentManager::ComponentStats ComponentManager::getStats() const {
    ComponentStats stats;
    stats.totalComponents = getComponentCount();
    stats.enabledComponents = getEnabledComponentCount();
    stats.initializedComponents = getInitializedComponentCount();
    stats.runningComponents = getRunningComponentCount();
    stats.healthyComponents = healthyCount_;
    return stats;
}

// Private methods
Result ComponentManager::initializeComponent(const SharedPtr<IComponent>& component) {
    if (!component->isEnabled()) {
        return Result::SUCCESS;
    }

    try {
        Result result = component->initialize();
        if (result == Result::SUCCESS) {
            LOG_DEBUG("Initialized component: {}", component->getComponentName());
        }
        else {
            LOG_ERROR("Failed to initialize component: {}", component->getComponentName());
        }
        return result;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception initializing component {}: {}", component->getComponentName(), e.what());
        return Result::ERROR;
    }
}

Result ComponentManager::startComponent(const SharedPtr<IComponent>& component) {
    if (!component->isEnabled() || !component->isInitialized()) {
        return Result::SUCCESS;
    }

    try {
        Result result = component->start();
        if (result == Result::SUCCESS) {
            LOG_DEBUG("Started component: {}", component->getComponentName());
        }
        else {
            LOG_ERROR("Failed to start component: {}", component->getComponentName());
        }
        return result;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception starting component {}: {}", component->getComponentName(), e.what());
        return Result::ERROR;
    }
}

Result ComponentManager::stopComponent(const SharedPtr<IComponent>& component) {
    if (!component->isEnabled() || !component->isRunning()) {
        return Result::SUCCESS;
    }

    try {
        Result result = component->stop();
        if (result == Result::SUCCESS) {
            LOG_DEBUG("Stopped component: {}", component->getComponentName());
        }
        return result;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception stopping component {}: {}", component->getComponentName(), e.what());
        return Result::ERROR;
    }
}

Result ComponentManager::shutdownComponent(const SharedPtr<IComponent>& component) {
    if (!component->isEnabled()) {
        return Result::SUCCESS;
    }

    try {
        Result result = component->destroy();
        if (result == Result::SUCCESS) {
            LOG_DEBUG("Shutdown component: {}", component->getComponentName());
        }
        return result;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception shutting down component {}: {}", component->getComponentName(), e.what());
        return Result::ERROR;
    }
}

void ComponentManager::updateComponentCounts() {
    std::unique_lock lock(mutex_);

    size_t enabled = 0;
    size_t initialized = 0;
    size_t running = 0;
    size_t healthy = 0;

    for (const auto& component : components_) {
        if (component->isEnabled()) enabled++;
        if (component->isInitialized()) initialized++;
        if (component->isRunning()) running++;
        if (component->isHealthy()) healthy++;
    }

    enabledCount_ = enabled;
    initializedCount_ = initialized;
    runningCount_ = running;
    healthyCount_ = healthy;
}

bool ComponentManager::checkDependency(const SharedPtr<IComponent>& component, const std::string& dependency) const {
    for (const auto& depComponent : components_) {
        if (depComponent->getComponentName() == dependency || depComponent->getComponentId() == dependency) {
            return depComponent->isEnabled() && depComponent->isOperational();
        }
    }
    return false;
}

void ComponentManager::buildDependencyGraph() {
    // Implementation would build a dependency graph for topological sorting
    // Used for proper initialization order
}

WEBSOCKET_NAMESPACE_END