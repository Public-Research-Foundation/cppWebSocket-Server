#pragma once
#ifndef WEBSOCKET_GLOBAL_CONFIG_HPP
#define WEBSOCKET_GLOBAL_CONFIG_HPP

#include "Macros.hpp"
#include "Types.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

// Forward declaration
namespace CppWebSocket {
    class ConfigManager;
}

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class GlobalConfig
 * @brief Singleton class providing global configuration access with thread-safe operations
 *
 * Features:
 * - Thread-safe configuration access
 * - Hot-reload capability for runtime updates
 * - Configuration override system
 * - Fallback to default values
 * - Configuration validation
 *
 * @note This class follows the Meyer's singleton pattern for thread safety
 */
    class GlobalConfig {
    public:
        /**
         * @brief Get the singleton instance of GlobalConfig
         * @return Reference to the global configuration instance
         *
         * @note Thread-safe initialization guaranteed by C++11 standard
         */
        static GlobalConfig& getInstance();

        /**
         * @brief Initialize the configuration system
         * @param configPath Path to configuration file (empty for default location)
         * @return true if initialization successful, false otherwise
         *
         * @throws std::runtime_error if configuration file cannot be loaded
         */
        bool initialize(const std::string& configPath = "");

        /**
         * @brief Reload configuration from disk
         * @return true if reload successful, false otherwise
         *
         * @note Maintains runtime overrides during reload
         */
        bool reload();

        /**
         * @brief Set runtime configuration override
         * @param key Configuration key in dot notation (e.g., "server.port")
         * @param value Configuration value as string
         * @return true if override set successfully
         *
         * @note Runtime overrides take precedence over file configuration
         */
        bool setRuntimeOverride(const std::string& key, const std::string& value);

        /**
         * @brief Remove runtime configuration override
         * @param key Configuration key to remove
         * @return true if override removed successfully
         */
        bool removeRuntimeOverride(const std::string& key);

        /**
         * @brief Clear all runtime overrides
         */
        void clearRuntimeOverrides();

        /**
         * @brief Get the configuration manager instance
         * @return Shared pointer to ConfigManager
         *
         * @throws std::runtime_error if configuration not initialized
         */
        std::shared_ptr<ConfigManager> getConfigManager();

        /**
         * @brief Check if configuration system is initialized
         * @return true if initialized and ready for use
         */
        bool isInitialized() const;

        /**
         * @brief Get configuration file path
         * @return Current configuration file path
         */
        std::string getConfigPath() const;

        // Delete copy and move operations for singleton
        WEBSOCKET_DISABLE_COPY(GlobalConfig)
            WEBSOCKET_DISABLE_MOVE(GlobalConfig)

    private:
        /**
         * @brief Private constructor for singleton pattern
         */
        GlobalConfig() = default;

        /**
         * @brief Private destructor
         */
        ~GlobalConfig() = default;

        // Member variables
        std::shared_ptr<ConfigManager> configManager_;          ///< Configuration manager instance
        std::string configPath_;                               ///< Configuration file path
        std::unordered_map<std::string, std::string> overrides_; ///< Runtime configuration overrides
        mutable std::mutex mutex_;                             ///< Thread safety mutex
        std::atomic<bool> initialized_{ false };                 ///< Initialization flag
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_GLOBAL_CONFIG_HPP
