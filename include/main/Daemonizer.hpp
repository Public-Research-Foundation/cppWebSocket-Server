#pragma once
#ifndef WEBSOCKET_DAEMONIZER_HPP
#define WEBSOCKET_DAEMONIZER_HPP

#include <string>

WEBSOCKET_NAMESPACE_BEGIN

    /**
     * @class Daemonizer
     * @brief Handles daemon process creation and management
     *
     * Provides functionality to:
     * - Detach process and run in background
     * - Manage PID files for process tracking
     * - Handle standard stream redirection
     * - Ensure single instance operation
     */
    class Daemonizer {
    public:
        Daemonizer() = delete;
        ~Daemonizer() = delete;

        // Delete copy constructor and assignment operator
        Daemonizer(const Daemonizer&) = delete;
        Daemonizer& operator=(const Daemonizer&) = delete;

        /**
         * @brief Convert current process to daemon
         * @param work_dir Working directory for daemon (empty = no change)
         * @return true if daemonization successful, false otherwise
         */
        static bool daemonize(const std::string& work_dir = "");

        /**
         * @brief Check if current process is a daemon
         * @return true if process is a daemon, false otherwise
         */
        static bool isDaemon();

        /**
         * @brief Create PID file for current process
         * @param pid_file Path to PID file (empty = use default)
         * @return true if PID file created successfully, false otherwise
         */
        static bool createPidFile(const std::string& pid_file = "");

        /**
         * @brief Remove PID file
         * @param pid_file Path to PID file (empty = use default)
         * @return true if PID file removed successfully, false otherwise
         */
        static bool removePidFile(const std::string& pid_file = "");

        /**
         * @brief Check if another instance is already running
         * @param pid_file Path to PID file to check
         * @return true if another instance is running, false otherwise
         */
        static bool isAlreadyRunning(const std::string& pid_file = "");

        /**
         * @brief Get default PID file path
         * @return Default PID file path
         */
        static std::string getDefaultPidFile();

    private:
        /**
         * @brief Redirect standard input, output, and error streams
         * @param input_file File for stdin (empty = /dev/null)
         * @param output_file File for stdout (empty = /dev/null)
         * @param error_file File for stderr (empty = /dev/null)
         */
        static void redirectStandardStreams(const std::string& input_file = "",
            const std::string& output_file = "",
            const std::string& error_file = "");

        /**
         * @brief Fork current process
         * @return Process ID of child, or -1 on error
         */
        static int forkProcess();

        /**
         * @brief Create new session and detach from terminal
         * @return true if session creation successful, false otherwise
         */
        static bool createNewSession();
    };

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_DAEMONIZER_HPP