#pragma once

#include "../common/Types.hpp"
#include <unordered_map>
#include <any>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;

    bool loadFromFile(const std::string& filePath);
    bool loadFromString(const std::string& configData);
    void set(const std::string& key, const std::any& value);
    void setRuntimeOverride(const std::string& key, const std::any& value);

    template<typename T>
    T get(const std::string& key, const T& defaultValue = T()) const;

    template<typename T>
    T getWithFallback(const std::string& primaryKey, const std::string& fallbackKey, const T& defaultValue = T()) const;

    bool has(const std::string& key) const;
    void clearRuntimeOverrides();
    void reload();

    // Validation
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;

private:
    std::string configFilePath_;
    std::unordered_map<std::string, std::any> configMap_;
    std::unordered_map<std::string, std::any> runtimeOverrides_;
    mutable std::atomic<bool> configLoaded_{ false };

    bool parseJsonConfig(const std::string& jsonData);
    bool parseYamlConfig(const std::string& yamlData);
    std::string getFileExtension(const std::string& filePath) const;
};

WEBSOCKET_NAMESPACE_END