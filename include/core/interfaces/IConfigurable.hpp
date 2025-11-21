#pragma once

#include "../../common/Types.hpp"
#include <unordered_map>
#include <any>

WEBSOCKET_NAMESPACE_BEGIN

class IConfigurable {
    WEBSOCKET_INTERFACE(IConfigurable)

public:
    // Configuration management
    virtual void configure(const std::unordered_map<std::string, std::any>& config) = 0;
    virtual std::unordered_map<std::string, std::any> getCurrentConfig() const = 0;
    virtual bool validateConfig(const std::unordered_map<std::string, std::any>& config) const = 0;

    // Configuration change handling
    virtual void onConfigChanged(const std::string& key, const std::any& value) = 0;
    virtual void onConfigLoaded(const std::unordered_map<std::string, std::any>& config) = 0;
    virtual void onConfigSaved(const std::unordered_map<std::string, std::any>& config) = 0;

    // Configuration schema
    virtual std::unordered_map<std::string, std::string> getConfigSchema() const = 0;
    virtual std::vector<std::string> getRequiredConfigKeys() const = 0;
    virtual std::vector<std::string> getOptionalConfigKeys() const = 0;

    // Configuration validation
    virtual bool isConfigValid() const = 0;
    virtual std::vector<std::string> getConfigValidationErrors() const = 0;
    virtual void clearConfigValidationErrors() = 0;

    // Configuration persistence
    virtual Result saveConfigToFile(const std::string& filePath) const = 0;
    virtual Result loadConfigFromFile(const std::string& filePath) = 0;
    virtual Result resetToDefaultConfig() = 0;

    // Configuration events
    virtual void addConfigChangeListener(const std::string& key, std::function<void(const std::any&)> listener) = 0;
    virtual void removeConfigChangeListener(const std::string& key) = 0;
    virtual void notifyConfigChange(const std::string& key, const std::any& value) = 0;

    // Configuration metadata
    virtual std::string getConfigVersion() const = 0;
    virtual std::chrono::system_clock::time_point getLastConfigChangeTime() const = 0;
    virtual std::string getConfigSource() const = 0;
};

WEBSOCKET_NAMESPACE_END