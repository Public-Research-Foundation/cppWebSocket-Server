#pragma once
#ifndef WEBSOCKET_APPLICATION_HPP
#define WEBSOCKET_APPLICATION_HPP

#include "../core/WebSocketServer.hpp"
#include "../config/ConfigManager.hpp"
#include "CommandLineParser.hpp"
#include "SignalHandler.hpp"
#include "Daemonizer.hpp"
#include <memory>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

    /**
     * @class Application
     * @brief Main application class that orchestrates the WebSocket server lifecycle
     *
     * This class handles:
     * - Command line argument parsing
     * - Configuration loading and validation
     * - Server initialization and shutdown
     * - Signal handling for graceful termination
     * - Daemon process management
     */
    class Application {
    public:
        Application();
        ~Application();

        // Delete copy constructor and assignment operator
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /**
         * @brief Main application entry point
         * @param argc Argument count from main()
         * @param argv Argument values from main()
         * @return Exit code (0 = success, non-zero = error)
         */
        int run(int argc, char* argv[]);

        /**
         * @brief Check if application is running
         * @return true if server is active, false otherwise
         */
        bool isRunning() const;

        /**
         * @brief Request graceful shutdown
         */
        void requestShutdown();

    private:
        /**
         * @brief Initialize application components
         * @return true if initialization successful, false otherwise
         */
        bool initialize();

        /**
         * @brief Cleanup application resources
         */
        void shutdown();

        /**
         * @brief Parse and validate command line arguments
         * @param argc Argument count
         * @param argv Argument values
         * @return true if parsing successful, false otherwise
         */
        bool parseCommandLine(int argc, char* argv[]);

        /**
         * @brief Load configuration from file and environment
         * @return true if configuration loaded successfully, false otherwise
         */
        bool loadConfiguration();

        /**
         * @brief Apply command line overrides to configuration
         * @return true if overrides applied successfully, false otherwise
         */
        bool applyConfigOverrides();

        /**
         * @brief Setup signal handlers for graceful shutdown
         */
        void setupSignalHandlers();

        /**
         * @brief Main application event loop
         */
        void mainLoop();

        /**
         * @brief Handle graceful shutdown sequence
         */
        void gracefulShutdown();

        // Member variables
        std::unique_ptr<WebSocketServer> server_;
        std::unique_ptr<ConfigManager> config_manager_;
        CommandLineOptions cmd_options_;
        SignalHandler signal_handler_;
        std::atomic<bool> running_{ false };
        std::atomic<bool> shutdown_requested_{ false };
    };

WEBSOCKET_NAMESPACE_END


#endif // WEBSOCKET_APPLICATION_HPP