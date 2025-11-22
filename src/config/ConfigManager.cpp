#include "config/ConfigManager.hpp"
#include "config/ConfigParser.hpp"
#include "config/ConfigValidator.hpp"
#include "utils/FileUtils.hpp"
#include "utils/StringUtils.hpp"
#include <fstream>
#include <sstream>

WEBSOCKET_NAMESPACE_BEGIN

ConfigManager::ConfigManager() {
    // Initialize with empty configuration
    configLoaded_ = false;
}

bool ConfigManager::loadFromFile(const std::string& filePath) {
    if (!FileUtils::exists(filePath)) {
        return false;
    }

    configFilePath_ = filePath;
    std::string fileContent = FileUtils::readTextFile(filePath);
    return loadFromString(fileContent);
}

bool ConfigManager::loadFromString(const std::string& configData) {
    std::unique_ptr<ConfigParser> parser;
    std::string extension = getFileExtension(configFilePath_);

    if (extension == "json") {
        parser = std::make_unique<JsonConfigParser>();
    }
    else if (extension == "yaml" || extension == "yml") {
        parser = std::make_unique<YamlConfigParser>();
    }
    else {
        // Default to JSON if unknown extension
        parser = std::make_unique<JsonConfigParser>();
    }

    bool success = parser->parse(configData, configMap_);
    if (success) {
        configLoaded_ = true;

        // Validate configuration after loading
        ConfigValidator validator;
        if (!validator.validate(configMap_)) {
            // Log validation errors but don't fail loading
            auto errors = validator.getErrors();
        }
    }

    return success;
}

void ConfigManager::set(const std::string& key, const std::any& value) {
    configMap_[key] = value;
}

void ConfigManager::setRuntimeOverride(const std::string& key, const std::any& value) {
    runtimeOverrides_[key] = value;
}

template<typename T>
T ConfigManager::get(const std::string& key, const T& defaultValue) const {
    // Check runtime overrides first (highest priority)
    auto overrideIt = runtimeOverrides_.find(key);
    if (overrideIt != runtimeOverrides_.end()) {
        try {
            return std::any_cast<T>(overrideIt->second);
        }
        catch (const std::bad_any_cast&) {
            // Type mismatch in override, fall through
        }
    }

    // Check main configuration
    auto it = configMap_.find(key);
    if (it != configMap_.end()) {
        try {
            return std::any_cast<T>(it->second);
        }
        catch (const std::bad_any_cast&) {
            // Type mismatch, return default
        }
    }

    return defaultValue;
}

template<typename T>
T ConfigManager::getWithFallback(const std::string& primaryKey, const std::string& fallbackKey, const T& defaultValue) const {
    if (has(primaryKey)) {
        return get<T>(primaryKey, defaultValue);
    }
    return get<T>(fallbackKey, defaultValue);
}

bool ConfigManager::has(const std::string& key) const {
    return runtimeOverrides_.find(key) != runtimeOverrides_.end() ||
        configMap_.find(key) != configMap_.end();
}

void ConfigManager::clearRuntimeOverrides() {
    runtimeOverrides_.clear();
}

void ConfigManager::reload() {
    if (!configFilePath_.empty()) {
        loadFromFile(configFilePath_);
    }
}

bool ConfigManager::validate() const {
    ConfigValidator validator;
    return validator.validate(configMap_);
}

std::vector<std::string> ConfigManager::getValidationErrors() const {
    ConfigValidator validator;
    validator.validate(configMap_);
    return validator.getErrors();
}

bool ConfigManager::parseJsonConfig(const std::string& jsonData) {
    JsonConfigParser parser;
    return parser.parse(jsonData, configMap_);
}

bool ConfigManager::parseYamlConfig(const std::string& yamlData) {
    YamlConfigParser parser;
    return parser.parse(yamlData, configMap_);
}

std::string ConfigManager::getFileExtension(const std::string& filePath) const {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    return StringUtils::toLower(filePath.substr(dotPos + 1));
}

// Explicit template instantiations for common types
template int ConfigManager::get<int>(const std::string&, const int&) const;
template bool ConfigManager::get<bool>(const std::string&, const bool&) const;
template std::string ConfigManager::get<std::string>(const std::string&, const std::string&) const;
template double ConfigManager::get<double>(const std::string&, const double&) const;

template int ConfigManager::getWithFallback<int>(const std::string&, const std::string&, const int&) const;
template bool ConfigManager::getWithFallback<bool>(const std::string&, const std::string&, const bool&) const;

WEBSOCKET_NAMESPACE_END