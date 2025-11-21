#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Daemonizer - Background Process Management
 *
 * PURPOSE:
 * - Convert application to daemon (background) process
 * - Manage PID files for process tracking
 * - Handle privilege dropping for security
 * - Provide daemon lifecycle management
 *
 * FEATURES:
 * - Cross-platform daemonization (Unix/Windows)
 * - PID file management with locking
 * - Privilege dropping for security
 * - Signal handling for daemon processes
 * - Standard stream redirection
 */
    class Daemonizer {
    public:
        static Daemonizer& getInstance();

        // ===== DAEMON LIFECYCLE MANAGEMENT =====

        /**
         * Convert current process to daemon
         *
         * @param pidFile Path to PID file for process tracking
         * @param workingDir Working directory for daemon
         * @return Result indicating daemonization success
         */
        Result daemonize(const std::string& pidFile = "", const std::string& workingDir = "");

        /**
         * Stop running daemon process
         * Sends termination signal to daemon process
         */
        Result stopDaemon(const std::string& pidFile = "");

        /**
         * Reload daemon configuration
         * Sends reload signal (SIGHUP) to daemon process
         */
        Result reloadDaemon(const std::string& pidFile = "");

        // ===== DAEMON STATUS QUERIES =====

        bool isDaemon() const;
        bool isDaemonRunning(const std::string& pidFile = "") const;
        int getDaemonPid(const std::string& pidFile = "") const;

        // ===== DAEMON CONFIGURATION =====

        void setDaemonName(const std::string& name);
        void setUser(const std::string& username);
        void setGroup(const std::string& groupname);
        void setUmask(mode_t umask);

        // ===== SIGNAL HANDLING FOR DAEMON =====

        void setupDaemonSignalHandlers();
        void handleDaemonSignal(int signal);

        // ===== PID FILE MANAGEMENT =====

        Result createPidFile(const std::string& pidFile = "");
        Result removePidFile(const std::string& pidFile = "");
        Result readPidFile(const std::string& pidFile = "") const;

        // ===== DAEMON LOGGING =====

        void redirectStandardStreams(const std::string& logDir = "");
        void setupDaemonLogging(const std::string& logFile = "");

        // ===== SECURITY =====

        /**
         * Drop elevated privileges for security
         * Changes to non-privileged user after binding privileged ports
         */
        Result dropPrivileges();
        bool hasPrivileges() const;

        // ===== DAEMON INFORMATION =====

        std::string getDaemonStatus() const;
        std::string getPidFilePath() const;

    private:
        Daemonizer();
        ~Daemonizer();

        WEBSOCKET_DISABLE_COPY(Daemonizer)

            std::atomic<bool> isDaemon_{ false };
        std::string daemonName_;
        std::string userName_;
        std::string groupName_;
        std::string pidFilePath_;
        std::string workingDirectory_;
        mode_t umask_{ 0 };
        int originalPid_{ 0 };
        int daemonPid_{ 0 };

        // ===== PLATFORM-SPECIFIC IMPLEMENTATIONS =====

        Result daemonizeUnix(const std::string& pidFile, const std::string& workingDir);
        Result daemonizeWindows(const std::string& pidFile, const std::string& workingDir);

        Result stopDaemonUnix(const std::string& pidFile);
        Result stopDaemonWindows(const std::string& pidFile);

        bool isDaemonRunningUnix(const std::string& pidFile) const;
        bool isDaemonRunningWindows(const std::string& pidFile) const;

        Result createPidFileUnix(const std::string& pidFile);
        Result createPidFileWindows(const std::string& pidFile);

        Result removePidFileUnix(const std::string& pidFile);
        Result removePidFileWindows(const std::string& pidFile);

        Result readPidFileUnix(const std::string& pidFile) const;
        Result readPidFileWindows(const std::string& pidFile) const;

        Result dropPrivilegesUnix();
        Result dropPrivilegesWindows();

        void redirectStandardStreamsUnix(const std::string& logDir);
        void redirectStandardStreamsWindows(const std::string& logDir);

        bool checkPidFile(const std::string& pidFile) const;
        bool sendSignalToProcess(int pid, int signal) const;
};

WEBSOCKET_NAMESPACE_END