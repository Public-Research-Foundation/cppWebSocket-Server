#pragma once
#ifndef WEBSOCKET_CONFIG_MANAGER_HPP
#define WEBSOCKET_CONFIG_MANAGER_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "ConfigParser.hpp"
#include "ConfigValidator.hpp"
#include <unordered_map>
#include <any>
#include <atomic>
#include <shared_mutex>
#include <memory>
#include <functional>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class ConfigManager
 * @brief Central configuration management system with multi-format support and hot-reload capabilities
 *
 * Features:
 * - Multi-format configuration parsing (JSON, YAML)
 * - Runtime configuration overrides
 * - Thread-safe configuration access
 * - Hot-reload support
 * - Validation and error reporting
 * - Change notification system
 */
    class ConfigManager {
    public:
        using ChangeCallback = std::function<void(const std::string& key, const std::any& oldValue, const std::any& newValue)>;

        /**
         * @brief Constructor
         */
        ConfigManager();

        /**
         * @brief Destructor
         */
        ~ConfigManager() = default;

        // Delete copy and move operations
        WEBSOCKET_DISABLE_COPY(ConfigManager)
            WEBSOCKET_DISABLE_MOVE(ConfigManager)

            /**
             * @brief Load configuration from file
             * @param filePath Path to configuration file
             * @return true if loading successful, false otherwise
             */
            bool loadFromFile(const std::string& filePath);

        /**
         * @brief Load configuration from string data
         * @param configData Configuration data as string
         * @param format Format of the data ("json", "yaml", or auto-detect)
         * @return true if parsing successful, false otherwise
         */
        bool loadFromString(const std::string& configData, const std::string& format = "auto");

        /**
         * @brief Set configuration value
         * @param key Configuration key in dot notation (e.g., "server.port")
         * @param value Configuration value
         */
        void set(const std::string& key, const std::any& value);

        /**
         * @brief Set runtime override (takes precedence over file configuration)
         * @param key Configuration key
         * @param value Override value
         */
        void setRuntimeOverride(const std::string& key, const std::any& value);

        /**
         * @brief Get configuration value with type safety
         * @tparam T Type of the configuration value
         * @param key Configuration key
         * @param defaultValue Default value if key not found
         * @return Configuration value or default if not found
         */
        template<typename T>
        T get(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Get configuration value with fallback key
         * @tparam T Type of the configuration value
         * @param primaryKey Primary configuration key to try first
         * @param fallbackKey Fallback key if primary not found
         * @param defaultValue Default value if neither key found
         * @return Configuration value from primary, fallback, or default
         */
        template<typename T>
        T getWithFallback(const std::string& primaryKey, const std::string& fallbackKey, const T& defaultValue = T{}) const;

        /**
         * @brief Check if configuration key exists
         * @param key Configuration key to check
         * @return true if key exists in configuration
         */
        bool has(const std::string& key) const;

        /**
         * @brief Clear all runtime overrides
         */
        void clearRuntimeOverrides();

        /**
         * @brief Reload configuration from original source
         * @return true if reload successful, false otherwise
         */
        bool reload();

        /**
         * @brief Validate current configuration
         * @return true if configuration is valid, false otherwise
         */
        bool validate() const;

        /**
         * @brief Get validation errors
         * @return Vector of validation error messages
         */
        std::vector<std::string> getValidationErrors() const;

        /**
         * @brief Check if configuration is loaded
         * @return true if configuration has been successfully loaded
         */
        bool isLoaded() const;

        /**
         * @brief Get configuration file path
         * @return Current configuration file path
         */
        std::string getConfigPath() const;

        /**
         * @brief Add change listener for configuration key
         * @param key Configuration key to monitor
         * @param callback Function to call when value changes
         */
        void addChangeListener(const std::string& key, ChangeCallback callback);

        /**
         * @brief Remove change listener for configuration key
         * @param key Configuration key
         * @param callback Callback to remove (if empty, remove all for key)
         */
        void removeChangeListener(const std::string& key, ChangeCallback callback = nullptr);

        /**
         * @brief Export current configuration to string
         * @param format Output format ("json" or "yaml")
         * @return Configuration as string
         */
        std::string exportToString(const std::string& format = "json") const;

    private:
        /**
         * @brief Parse JSON configuration
         * @param jsonData JSON configuration data
         * @return true if parsing successful
         */
        bool parseJsonConfig(const std::string& jsonData);

        /**
         * @brief Parse YAML configuration
         * @param yamlData YAML configuration data
         * @return true if parsing successful
         */
        bool parseYamlConfig(const std::string& yamlData);

        /**
         * @brief Get file extension from path
         * @param filePath File path
         * @return File extension in lowercase
         */
        std::string getFileExtension(const std::string& filePath) const;

        /**
         * @brief Detect configuration format from content
         * @param data Configuration data
         * @return Detected format ("json", "yaml", or "unknown")
         */
        std::string detectFormat(const std::string& data) const;

        /**
         * @brief Notify change listeners for a key
         * @param key Configuration key that changed
         * @param oldValue Previous value
         * @param newValue New value
         */
        void notifyChangeListeners(const std::string& key, const std::any& oldValue, const std::any& newValue);

        /**
         * @brief Get value from configuration map with key parsing
         * @param key Configuration key in dot notation
         * @return Pointer to value if found, nullptr otherwise
         */
        const std::any* getValue(const std::string& key) const;

        // Member variables
        std::string configFilePath_;
        std::unordered_map<std::string, std::any> configMap_;
        std::unordered_map<std::string, std::any> runtimeOverrides_;
        std::unordered_map<std::string, std::vector<ChangeCallback>> changeListeners_;
        std::unique_ptr<ConfigValidator> validator_;
        mutable std::shared_mutex mutex_;
        std::atomic<bool> configLoaded_{ false };
        std::atomic<bool> validationEnabled_{ true };
};

// Template method implementations
template<typename T>
T ConfigManager::get(const std::string& key, const T& defaultValue) const {
    std::shared_lock lock(mutex_);

    // Check runtime overrides first (highest priority)
    auto overrideIt = runtimeOverrides_.find(key);
    if (overrideIt != runtimeOverrides_.end()) {
        try {
            return std::any_cast<T>(overrideIt->second);
        }
        catch (const std::bad_any_cast&) {
            // Type mismatch in override, fall through to main config
        }
    }

    // Check main configuration
    auto configIt = configMap_.find(key);
    if (configIt != configMap_.end()) {
        try {
            return std::any_cast<T>(configIt->second);
        }
        catch (const std::bad_any_cast&) {
            // Type mismatch, return default
            return defaultValue;
        }
    }

    return defaultValue;
}

template<typename T>
T ConfigManager::getWithFallback(const std::string& primaryKey, const std::string& fallbackKey, const T& defaultValue) const {
    std::shared_lock lock(mutex_);

    // Try primary key first
    if (has(primaryKey)) {
        return get<T>(primaryKey, defaultValue);
    }

    // Try fallback key
    if (has(fallbackKey)) {
        return get<T>(fallbackKey, defaultValue);
    }

    return defaultValue;
}

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_CONFIG_MANAGER_HPP