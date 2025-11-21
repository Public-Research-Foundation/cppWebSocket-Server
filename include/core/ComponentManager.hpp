#pragma once

#include "../common/Types.hpp"
#include "interfaces/IComponent.hpp"
#include "interfaces/IConfigurable.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class ComponentManager : public IConfigurable {
public:
    ComponentManager();
    ~ComponentManager() override;

    // Component lifecycle management
    Result initializeAll();
    Result startAll();
    Result stopAll();
    Result shutdownAll();
    void updateAll();

    // Component registration
    template<typename T>
    SharedPtr<T> registerComponent(const SharedPtr<T>& component);

    template<typename T>
    SharedPtr<T> getComponent() const;

    template<typename T>
    bool hasComponent() const;

    SharedPtr<IComponent> getComponentByName(const std::string& name) const;
    SharedPtr<IComponent> getComponentById(const std::string& id) const;

    // Component control
    void enableComponent(const std::string& name);
    void disableComponent(const std::string& name);
    void enableAllComponents();
    void disableAllComponents();

    // Dependency management
    Result resolveDependencies();
    bool validateDependencies() const;
    std::vector<std::string> getDependencyErrors() const;

    // Health monitoring
    bool areAllComponentsHealthy() const;
    std::vector<std::string> getUnhealthyComponents() const;
    std::unordered_map<std::string, std::string> getComponentHealthStatus() const;

    // Statistics
    size_t getComponentCount() const;
    size_t getEnabledComponentCount() const;
    size_t getInitializedComponentCount() const;
    size_t getRunningComponentCount() const;

    std::vector<std::string> getComponentNames() const;
    std::vector<std::string> getComponentNamesByType(const std::string& type) const;

    struct ComponentStats {
        size_t totalComponents;
        size_t enabledComponents;
        size_t initializedComponents;
        size_t runningComponents;
        size_t healthyComponents;
    };

    ComponentStats getStats() const;

    // IConfigurable implementation
    void configure(const std::unordered_map<std::string, std::any>& config) override;
    std::unordered_map<std::string, std::any> getCurrentConfig() const override;
    bool validateConfig(const std::unordered_map<std::string, std::any>& config) const override;
    void onConfigChanged(const std::string& key, const std::any& value) override;

private:
    mutable std::shared_mutex mutex_;
    std::vector<SharedPtr<IComponent>> components_;
    std::unordered_map<std::type_index, SharedPtr<IComponent>> componentMap_;
    std::unordered_map<std::string, SharedPtr<IComponent>> namedComponents_;
    std::unordered_map<std::string, SharedPtr<IComponent>> idComponents_;

    std::atomic<size_t> enabledCount_{ 0 };
    std::atomic<size_t> initializedCount_{ 0 };
    std::atomic<size_t> runningCount_{ 0 };
    std::atomic<size_t> healthyCount_{ 0 };

    Result initializeComponent(const SharedPtr<IComponent>& component);
    Result startComponent(const SharedPtr<IComponent>& component);
    Result stopComponent(const SharedPtr<IComponent>& component);
    Result shutdownComponent(const SharedPtr<IComponent>& component);

    void updateComponentCounts();
    bool checkDependency(const SharedPtr<IComponent>& component, const std::string& dependency) const;
    void buildDependencyGraph();
};

// Template implementations
template<typename T>
SharedPtr<T> ComponentManager::registerComponent(const SharedPtr<T>& component) {
    static_assert(std::is_base_of<IComponent, T>::value, "T must derive from IComponent");

    std::unique_lock lock(mutex_);
    components_.push_back(component);
    componentMap_[std::type_index(typeid(T))] = component;
    namedComponents_[component->getComponentName()] = component;
    idComponents_[component->getComponentId()] = component;

    return component;
}

template<typename T>
SharedPtr<T> ComponentManager::getComponent() const {
    std::shared_lock lock(mutex_);
    auto it = componentMap_.find(std::type_index(typeid(T)));
    return it != componentMap_.end() ? std::static_pointer_cast<T>(it->second) : nullptr;
}

template<typename T>
bool ComponentManager::hasComponent() const {
    std::shared_lock lock(mutex_);
    return componentMap_.find(std::type_index(typeid(T))) != componentMap_.end();
}

WEBSOCKET_NAMESPACE_END