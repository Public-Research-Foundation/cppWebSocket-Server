#pragma once
#ifndef WEBSOCKET_APPLICATION_HPP
#define WEBSOCKET_APPLICATION_HPP

#include "common/Types.hpp"
#include "common/Macros.hpp"
#include "config/ConfigManager.hpp"
#include "config/RuntimeConfig.hpp"
#include "core/Engine.hpp"
#include "core/ServiceLocator.hpp"
#include "core/LifecycleManager.hpp"
#include "main/CommandLineParser.hpp"
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class Application
 * @brief Main application class implementing singleton pattern for WebSocket server
 *
 * Manages the complete lifecycle of the WebSocket server including:
 * - Initialization and configuration
 * - Service dependency management
 * - Signal handling and graceful shutdown
 * - Runtime statistics and monitoring
 */
    class Application {
    public:
        // ============================================================================
        // SINGLETON PATTERN
        // ============================================================================

        /**
         * @brief Get the singleton instance of Application
         * @return Reference to the application instance
         *
         * @note Thread-safe initialization guaranteed by C++11 static local variables
         */
        static Application& getInstance();

        /**
         * @brief Delete copy constructor to enforce singleton pattern
         */
        Application(const Application&) = delete;

        /**
         * @brief Delete assignment operator to enforce singleton pattern
         */
        Application& operator=(const Application&) = delete;

        // ============================================================================
        // LIFECYCLE MANAGEMENT
        // ============================================================================

        /**
         * @brief Initialize the application with command line arguments
         * @param argc Argument count from main()
         * @param argv Argument values from main()
         * @return Result indicating success or failure
         */
        Result initialize(int argc, char* argv[]);

        /**
         * @brief Run the main application loop
         * @return Result indicating success or failure
         */
        Result run();

        /**
         * @brief Shutdown the application gracefully
         * @return Result indicating success or failure
         */
        Result shutdown();

        /**
         * @brief Emergency shutdown when normal shutdown fails
         * @return Result indicating success or failure
         */
        Result emergencyShutdown();

        // ============================================================================
        // STATUS QUERIES
        // ============================================================================

        /**
         * @brief Check if application is running
         * @return true if application is in running state
         */
        bool isRunning() const;

        /**
         * @brief Check if application is initialized
         * @return true if application is initialized
         */
        bool isInitialized() const;

        /**
         * @brief Get current application status as string
         * @return Human-readable status string
         */
        std::string getStatus() const;

        // ============================================================================
        // CONFIGURATION MANAGEMENT
        // ============================================================================

        /**
         * @brief Set configuration file path
         * @param configPath Path to configuration file
         */
        void setConfigFile(const std::string& configPath);

        /**
         * @brief Set runtime configuration overrides
         * @param overrides Map of configuration key-value overrides
         */
        void setConfigOverrides(const std::unordered_map<std::string, std::any>& overrides);

        /**
         * @brief Reload configuration from disk
         */
        void reloadConfig();

        // ============================================================================
        // COMMAND LINE INTERFACE
        // ============================================================================

        /**
         * @brief Parse command line arguments
         * @param argc Argument count
         * @param argv Argument values
         */
        void parseCommandLine(int argc, char* argv[]);

        /**
         * @brief Show help message and exit
         */
        void showHelp() const;

        /**
         * @brief Show version information and exit
         */
        void showVersion() const;

        // ============================================================================
        // DAEMON SUPPORT
        // ============================================================================

        /**
         * @brief Daemonize the application (run as background process)
         * @return Result indicating success or failure
         */
        Result daemonize();

        /**
         * @brief Check if application is running as daemon
         * @return true if running as daemon
         */
        bool isDaemon() const;

        // ============================================================================
        // INFORMATION METHODS
        // ============================================================================

        /**
         * @brief Get application name
         * @return Application name string
         */
        std::string getName() const;

        /**
         * @brief Get application version
         * @return Version string
         */
        std::string getVersion() const;

        /**
         * @brief Get application description
         * @return Description string
         */
        std::string getDescription() const;

        // ============================================================================
        // STATISTICS AND MONITORING
        // ============================================================================

        /**
         * @brief Application statistics structure
         */
        struct AppStats {
            std::chrono::steady_clock::time_point startTime;    ///< Application start time
            std::chrono::duration<double> uptime;               ///< Application uptime
            size_t totalConnections{ 0 };                         ///< Total connections handled
            size_t totalMessages{ 0 };                            ///< Total messages processed
            size_t totalErrors{ 0 };                              ///< Total errors encountered
            double memoryUsageMB{ 0.0 };                          ///< Current memory usage in MB
            ApplicationState state{ ApplicationState::UNINITIALIZED }; ///< Current application state
            bool isDaemon{ false };                               ///< Daemon mode status
        };

        /**
         * @brief Get current application statistics
         * @return Application statistics
         */
        AppStats getStats() const;

    private:
        // ============================================================================
        // APPLICATION STATES
        // ============================================================================

        /**
         * @brief Application state enumeration
         */
        enum class ApplicationState {
            UNINITIALIZED,      ///< Application not initialized
            INITIALIZING,       ///< Application initializing
            INITIALIZED,        ///< Application initialized but not running
            RUNNING,            ///< Application running normally
            SHUTTING_DOWN,      ///< Application shutting down
            SHUTDOWN,           ///< Application shut down completely
            ERROR               ///< Application in error state
        };

        // ============================================================================
        // PRIVATE CONSTRUCTOR AND DESTRUCTOR
        // ============================================================================

        /**
         * @brief Private constructor for singleton pattern
         */
        Application();

        /**
         * @brief Destructor
         */
        ~Application();

        // ============================================================================
        // PRIVATE INITIALIZATION METHODS
        // ============================================================================

        /**
         * @brief Initialize all application subsystems
         * @return Result indicating success or failure
         */
        Result initializeSubsystems();

        /**
         * @brief Initialize logging subsystem
         * @return Result indicating success or failure
         */
        Result initializeLogging();

        /**
         * @brief Initialize configuration subsystem
         * @return Result indicating success or failure
         */
        Result initializeConfiguration();

        /**
         * @brief Initialize service subsystem
         * @return Result indicating success or failure
         */
        Result initializeServices();

        /**
         * @brief Initialize network subsystem
         * @return Result indicating success or failure
         */
        Result initializeNetwork();

        /**
         * @brief Initialize protocol subsystem
         * @return Result indicating success or failure
         */
        Result initializeProtocol();

        // ============================================================================
        // PRIVATE SHUTDOWN METHODS
        // ============================================================================

        /**
         * @brief Shutdown network subsystem
         * @return Result indicating success or failure
         */
        Result shutdownNetwork();

        /**
         * @brief Shutdown engine subsystem
         * @return Result indicating success or failure
         */
        Result shutdownEngine();

        /**
         * @brief Shutdown service subsystem
         * @return Result indicating success or failure
         */
        Result shutdownServices();

        /**
         * @brief Cleanup application resources
         * @return Result indicating success or failure
         */
        Result cleanup();

        // ============================================================================
        // PRIVATE OPERATIONAL METHODS
        // ============================================================================

        /**
         * @brief Run the main application loop
         * @return Result indicating success or failure
         */
        Result runMainLoop();

        /**
         * @brief Setup signal handlers for graceful shutdown
         * @return true if signal handlers installed successfully
         */
        bool setupSignalHandlers();

        /**
         * @brief Handle system signals
         * @param signal Signal number received
         */
        void handleSignal(int signal);

        /**
         * @brief Perform periodic maintenance tasks
         */
        void performMaintenance();

        /**
         * @brief Check system health and resource usage
         */
        void checkSystemHealth();

        // ============================================================================
        // PRIVATE UTILITY METHODS
        // ============================================================================

        /**
         * @brief Set application state
         * @param newState New application state
         */
        void setState(ApplicationState newState);

        /**
         * @brief Get initialization time duration
         * @return Initialization time in seconds
         */
        std::chrono::duration<double> getInitializationTime() const;

        /**
         * @brief Get current memory usage in MB
         * @return Memory usage in megabytes
         */
        double getCurrentMemoryUsageMB() const;

        /**
         * @brief Check if memory limit is exceeded
         * @return true if memory limit exceeded
         */
        bool isMemoryLimitExceeded() const;

        /**
         * @brief Configure logging subsystem
         */
        void configureLogging();

        /**
         * @brief Log startup information
         */
        void logStartupInfo();

        /**
         * @brief Log shutdown information
         */
        void logShutdownInfo();

        // ============================================================================
        // STATIC UTILITY METHODS
        // ============================================================================

        /**
         * @brief Convert application state to string
         * @param state Application state
         * @return String representation of state
         */
        static std::string applicationStateToString(ApplicationState state);

        /**
         * @brief Convert signal number to string
         * @param signal Signal number
         * @return String representation of signal
         */
        static std::string signalToString(int signal);

        /**
         * @brief Get platform name
         * @return Platform name string
         */
        static std::string getPlatformName();

        /**
         * @brief Get current process ID
         * @return Process ID
         */
        static int getCurrentProcessId();

        // ============================================================================
        // MEMBER VARIABLES
        // ============================================================================

        // Core components
        std::shared_ptr<ConfigManager> configManager_;          ///< Configuration management
        std::shared_ptr<RuntimeConfig> runtimeConfig_;          ///< Runtime configuration
        std::shared_ptr<Engine> engine_;                        ///< Core engine
        std::shared_ptr<ServiceLocator> serviceLocator_;        ///< Service dependency injection
        std::shared_ptr<LifecycleManager> lifecycleManager_;    ///< Lifecycle management

        // Command line processing
        CommandLineParser cmdParser_;                           ///< Command line argument parser
        std::string configPath_;                               ///< Configuration file path
        std::unordered_map<std::string, std::any> configOverrides_; ///< Configuration overrides

        // Application state
        mutable std::mutex stateMutex_;                         ///< State synchronization
        ApplicationState state_{ ApplicationState::UNINITIALIZED }; ///< Current application state
        std::atomic<bool> isRunning_{ false };                    ///< Running flag
        std::atomic<bool> shutdownRequested_{ false };            ///< Shutdown request flag
        std::atomic<bool> isDaemon_{ false };                     ///< Daemon mode flag

        // Timing and statistics
        std::chrono::steady_clock::time_point startTime_;       ///< Application start time

        // Error handling
        Error lastError_;                                       ///< Last error encountered

        // Main loop synchronization
        std::mutex mainLoopMutex_;                              ///< Main loop synchronization
        std::condition_variable mainLoopCondition_;             ///< Main loop condition variable

        // Configuration synchronization
        mutable std::mutex configMutex_;                        ///< Configuration synchronization
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_APPLICATION_HPP