#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "../common/Version.hpp"
#include "../config/ConfigManager.hpp"
#include "../config/RuntimeConfig.hpp"
#include "../core/Engine.hpp"
#include "../core/ServiceLocator.hpp"
#include "../core/LifecycleManager.hpp"
#include "../utils/Logger.hpp"
#include "CommandLineParser.hpp"
#include "Daemonizer.hpp"
#include "SignalHandler.hpp"
#include <memory>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Main Application Class - Central coordinator for the WebSocket server
 *
 * PURPOSE:
 * - Serves as the entry point and main controller for the entire application
 * - Manages application lifecycle (initialize, run, shutdown)
 * - Coordinates all subsystems and services
 * - Handles configuration, logging, and signal management
 * - Provides singleton access to the application instance
 *
 * FEATURES:
 * - Singleton pattern for global access
 * - Graceful startup and shutdown sequencing
 * - Configuration management integration
 * - Daemon mode support for background operation
 * - Comprehensive signal handling
 * - Service locator integration
 */
    class Application {
    public:
        /**
         * Get singleton instance of Application
         * Implements thread-safe singleton pattern
         */
        static Application& getInstance();

        // ===== APPLICATION LIFECYCLE MANAGEMENT =====

        /**
         * Initialize application with command line arguments
         * Performs phased initialization of all subsystems
         *
         * @param argc Command line argument count
         * @param argv Command line argument values
         * @return Result indicating success or failure
         */
        Result initialize(int argc = 0, char* argv[] = nullptr);

        /**
         * Main application execution loop
         * Starts all services and enters the main run loop
         *
         * @return Result indicating normal or abnormal termination
         */
        Result run();

        /**
         * Graceful application shutdown
         * Stops all services in proper order and cleans up resources
         *
         * @return Result indicating shutdown success
         */
        Result shutdown();

        /**
         * Emergency shutdown for critical failures
         * Bypasses normal shutdown sequence for immediate termination
         *
         * @return Result (usually indicates forced termination)
         */
        Result emergencyShutdown();

        // ===== APPLICATION STATE QUERIES =====

        bool isRunning() const;
        bool isInitialized() const;
        std::string getStatus() const;

        // ===== CONFIGURATION MANAGEMENT =====

        /**
         * Set configuration file path
         * Can be called before initialize() to use custom config
         */
        void setConfigFile(const std::string& configPath);

        /**
         * Set runtime configuration overrides
         * These values take precedence over config file values
         */
        void setConfigOverrides(const std::unordered_map<std::string, std::any>& overrides);

        /**
         * Reload configuration from file and apply changes
         * Supports hot-reloading without restarting application
         */
        void reloadConfig();

        // ===== COMMAND LINE INTERFACE =====

        /**
         * Parse command line arguments
         * Extracts configuration options, flags, and operational parameters
         */
        void parseCommandLine(int argc, char* argv[]);

        /**
         * Display help information to stdout
         * Shows available commands, options, and usage examples
         */
        void showHelp() const;

        /**
         * Display version information
         * Shows application name, version, and build information
         */
        void showVersion() const;

        // ===== DAEMON MODE SUPPORT =====

        /**
         * Convert application to daemon (background) process
         * Detaches from terminal and runs in background
         *
         * @return Result indicating daemonization success
         */
        Result daemonize();

        bool isDaemon() const;

        // ===== SIGNAL HANDLING =====

        /**
         * Setup signal handlers for graceful shutdown
         * Handles SIGTERM, SIGINT, SIGHUP, etc.
         */
        void setupSignalHandlers();

        /**
         * Handle received signal
         * Called by signal handler for appropriate signal processing
         */
        void handleSignal(int signal);

        // ===== SERVICE ACCESS =====

        /**
         * Template method to access registered services
         * Uses ServiceLocator pattern for dependency injection
         *
         * @tparam T Service interface type
         * @return Shared pointer to service instance
         */
        template<typename T>
        SharedPtr<T> getService() const;

        // ===== APPLICATION INFORMATION =====

        std::string getName() const;
        std::string getVersion() const;
        std::string getDescription() const;

        // ===== STATISTICS AND MONITORING =====

        /**
         * Application performance and operational statistics
         * Used for monitoring and health checking
         */
        struct AppStats {
            std::chrono::steady_clock::time_point startTime;
            std::chrono::duration<double> uptime;
            uint64_t totalConnections;
            uint64_t totalMessages;
            uint64_t totalErrors;
            double memoryUsageMB;
        };

        AppStats getStats() const;

    private:
        Application();
        ~Application();

        WEBSOCKET_DISABLE_COPY(Application)

            // ===== INITIALIZATION PHASES =====

            Result initializeLogging();          // Setup logging system first
        Result initializeConfiguration();    // Load and validate configuration
        Result initializeServices();         // Initialize core services
        Result initializeNetwork();          // Setup networking components
        Result initializeProtocol();         // Initialize protocol handlers

        // ===== SHUTDOWN PHASES =====

        Result shutdownNetwork();            // Gracefully close network connections
        Result shutdownServices();           // Stop all services in reverse order
        Result cleanup();                    // Final resource cleanup

        // ===== INTERNAL STATE =====

        std::atomic<bool> isRunning_{ false };
        std::atomic<bool> isInitialized_{ false };
        std::atomic<bool> isDaemon_{ false };
        std::atomic<bool> shutdownRequested_{ false };

        // ===== CORE COMPONENTS =====

        SharedPtr<ConfigManager> configManager_;     // Configuration management
        SharedPtr<RuntimeConfig> runtimeConfig_;     // Runtime configuration
        SharedPtr<Engine> engine_;                   // Main engine coordinator
        SharedPtr<ServiceLocator> serviceLocator_;   // Dependency injection
        SharedPtr<LifecycleManager> lifecycleManager_; // Startup/shutdown sequencing

        // ===== COMMAND LINE PROCESSING =====

        CommandLineParser cmdParser_;        // CLI argument parser

        // ===== APPLICATION METADATA =====

        std::string configPath_;             // Active configuration file path
        std::unordered_map<std::string, std::any> configOverrides_; // Runtime overrides
        std::chrono::steady_clock::time_point startTime_; // Application start time

        // ===== SIGNAL HANDLING =====

        SignalHandler signalHandler_;        // OS signal management

        // ===== ERROR HANDLING =====

        Error lastError_;                    // Last encountered error

        // ===== PRIVATE METHODS =====

        void handleShutdown();               // Coordinated shutdown procedure
        void handleConfigChange();           // Configuration change handler
        void logStartupInfo();               // Log application startup details
        void logShutdownInfo();              // Log application shutdown details
};

// Template implementation for service access
template<typename T>
SharedPtr<T> Application::getService() const {
    if (!serviceLocator_) {
        return nullptr;
    }
    return serviceLocator_->getService<T>();
}

WEBSOCKET_NAMESPACE_END