#pragma once
#ifndef WEBSOCKET_CONFIG_OVERRIDES_HPP
#define WEBSOCKET_CONFIG_OVERRIDES_HPP

#include "../config/ServerConfig.hpp"
#include <string>
#include <unordered_map>
#include <vector>

WEBSOCKET_NAMESPACE_BEGIN


    /**
     * @class ConfigOverrides
     * @brief Applies command line configuration overrides to server configuration
     *
     * Supports overriding configuration values via command line in the format:
     * --config.key=value or --config.nested.key=value
     */
    class ConfigOverrides {
    public:
        ConfigOverrides() = default;
        ~ConfigOverrides() = default;

        // Delete copy constructor and assignment operator
        ConfigOverrides(const ConfigOverrides&) = delete;
        ConfigOverrides& operator=(const ConfigOverrides&) = delete;

        /**
         * @brief Apply command line overrides to server configuration
         * @param config Server configuration to modify
         * @param overrides Map of key-value overrides
         * @return true if all overrides applied successfully, false otherwise
         */
        bool applyOverrides(ServerConfig& config,
            const std::unordered_map<std::string, std::string>& overrides);

        /**
         * @brief Validate if a configuration key can be overridden
         * @param key Configuration key to check
         * @return true if key can be overridden, false otherwise
         */
        static bool isValidOverrideKey(const std::string& key);

        /**
         * @brief Validate if a value is valid for a specific configuration key
         * @param key Configuration key
         * @param value Proposed value
         * @return true if value is valid for the key, false otherwise
         */
        static bool isValidOverrideValue(const std::string& key, const std::string& value);

        /**
         * @brief Get list of all supported override keys
         * @return Vector of supported configuration keys
         */
        static std::vector<std::string> getSupportedKeys();

    private:
        // Individual override methods for specific configuration sections
        bool applyPortOverride(ServerConfig& config, const std::string& value);
        bool applyAddressOverride(ServerConfig& config, const std::string& value);
        bool applyMaxConnectionsOverride(ServerConfig& config, const std::string& value);
        bool applyThreadsOverride(ServerConfig& config, const std::string& value);
        bool applyTimeoutOverride(ServerConfig& config, const std::string& value);
        bool applySecurityOverride(ServerConfig& config, const std::string& value);
        bool applyLoggingOverride(ServerConfig& config, const std::string& value);

        /**
         * @brief Parse string value to specific type with validation
         */
        template<typename T>
        T parseValue(const std::string& key, const std::string& value) const;

        /**
         * @brief Log override application for debugging
         */
        void logOverride(const std::string& key, const std::string& value, bool success);
    };

WEBSOCKET_NAMESPACE_END


#endif // WEBSOCKET_CONFIG_OVERRIDES_HPP