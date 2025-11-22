#include "common/GlobalConfig.hpp"
#include "config/ConfigManager.hpp"
#include <memory>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * GlobalConfig - Singleton for global configuration access
 * Provides centralized configuration management across the entire application
 */

 // Static singleton instance access
    GlobalConfig& GlobalConfig::getInstance() {
    // Meyer's singleton - thread-safe and lazy-initialized
    static GlobalConfig instance;
    return instance;
}

/**
 * Initialize the global configuration system
 *
 * @param configPath: Path to configuration file (empty for default config)
 *
 * Purpose: Sets up configuration manager and loads initial configuration
 * - Creates ConfigManager instance
 * - Loads configuration from file if path provided
 * - Sets default values for essential configuration
 */
void GlobalConfig::initialize(const std::string& configPath) {
    // Store the configuration file path for potential reloads
    configPath_ = configPath;

    // Create the configuration manager instance
    configManager_ = std::make_shared<ConfigManager>();

    // Load configuration from file if a path was provided
    if (!configPath.empty()) {
        configManager_->loadFromFile(configPath);
    }

    // Set default values for critical configuration if not already set
    // This ensures the application has minimal working configuration
    if (!configManager_->has("server.port")) {
        configManager_->set("server.port", 8080);  // Default WebSocket port
    }
}

/**
 * Reload configuration from the original configuration file
 *
 * Purpose: Supports hot-reloading of configuration without restarting application
 * - Re-reads configuration file
 * - Applies new configuration values
 * - Maintains runtime overrides
 */
void GlobalConfig::reload() {
    // Only reload if we have a valid configuration file path and manager
    if (!configPath_.empty() && configManager_) {
        configManager_->loadFromFile(configPath_);
    }
}

/**
 * Set a runtime configuration override
 *
 * @param key: Configuration key to override (e.g., "server.port")
 * @param value: String value to set (type conversion handled by ConfigManager)
 *
 * Purpose: Allows dynamic configuration changes at runtime
 * - Overrides file-based configuration
 * - Takes effect immediately
 * - Persists until cleared or application restart
 */
void GlobalConfig::setRuntimeOverride(const std::string& key, const std::string& value) {
    if (configManager_) {
        // Delegate to ConfigManager for actual type conversion and storage
        configManager_->setRuntimeOverride(key, value);
    }
}

/**
 * Get the configuration manager instance
 *
 * @return: Shared pointer to ConfigManager for direct configuration access
 *
 * Purpose: Provides access to the underlying configuration manager
 * - Used by components that need direct configuration access
 * - Allows advanced configuration operations
 */
std::shared_ptr<ConfigManager> GlobalConfig::getConfigManager() {
    return configManager_;
}

WEBSOCKET_NAMESPACE_END