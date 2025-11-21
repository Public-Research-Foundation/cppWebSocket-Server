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

    Result initializeAll();
    Result shutdownAll();
    void updateAll();

    template<typename T>
    SharedPtr<T> registerComponent(const SharedPtr<T>& component);

    template<typename T>
    SharedPtr<T> getComponent() const;

    template<typename T>
    bool hasComponent() const;

    void enableComponent(const std::string& name);
    void disableComponent(const std::string& name);

    // IConfigurable implementation
    void configure(const std::unordered_map<std::string, std::any>& config) override;
    std::unordered_map<std::string, std::any> getCurrentConfig() const override;
    bool validateConfig(const std::unordered_map<std::string, std::any>& config) const override;
    void onConfigChanged(const std::string& key, const std::any& value) override;

    size_t getComponentCount() const;
    std::vector<std::string> getComponentNames() const;

private:
    mutable std::shared_mutex mutex_;
    std::vector<SharedPtr<IComponent>> components_;
    std::unordered_map<std::type_index, SharedPtr<IComponent>> componentMap_;
    std::unordered_map<std::string, SharedPtr<IComponent>> namedComponents_;

    Result initializeComponent(const SharedPtr<IComponent>& component);
    Result shutdownComponent(const SharedPtr<IComponent>& component);
};

WEBSOCKET_NAMESPACE_END