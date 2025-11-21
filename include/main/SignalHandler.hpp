#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Signal Handler - OS Signal Management and Processing
 *
 * PURPOSE:
 * - Handle operating system signals for graceful application control
 * - Provide unified signal handling across platforms
 * - Support for custom signal handlers and routing
 * - Emergency shutdown and graceful termination support
 *
 * FEATURES:
 * - Cross-platform signal handling (Unix signals, Windows events)
 * - Custom handler registration for any signal
 * - Signal masking and blocking control
 * - Emergency shutdown signal detection
 * - Signal statistics and monitoring
 */
    class SignalHandler {
    public:
        static SignalHandler& getInstance();

        // ===== SIGNAL REGISTRATION =====

        /**
         * Register custom handler for specific signal
         *
         * @param signal OS signal number (e.g., SIGINT, SIGTERM)
         * @param handler Callback function to handle signal
         * @return Result indicating registration success
         */
        Result registerSignal(int signal, std::function<void(int)> handler);

        /**
         * Register default handler for signal
         * Uses system default behavior for the signal
         */
        Result registerDefaultHandler(int signal);

        void unregisterSignal(int signal);
        void unregisterAllSignals();

        // ===== SIGNAL HANDLING CONTROL =====

        Result setupSignalHandlers();
        Result restoreSignalHandlers();
        void ignoreSignal(int signal);
        void blockSignal(int signal);
        void unblockSignal(int signal);

        // ===== SIGNAL STATE QUERIES =====

        bool isSignalHandled(int signal) const;
        bool isSignalBlocked(int signal) const;
        std::vector<int> getRegisteredSignals() const;
        std::vector<int> getPendingSignals() const;

        // ===== SIGNAL INFORMATION =====

        static std::string signalToString(int signal);
        static int stringToSignal(const std::string& signalName);
        static std::string getSignalDescription(int signal);

        // ===== SIGNAL MASKING =====

        Result setSignalMask(const std::vector<int>& signals);
        Result getSignalMask(std::vector<int>& signals) const;

        // ===== SIGNAL WAITING =====

        int waitForSignal(const std::vector<int>& signals, int timeoutMs = -1);
        int waitForAnySignal(int timeoutMs = -1);

        // ===== SIGNAL STATISTICS =====

        struct SignalStats {
            uint64_t totalSignalsReceived;
            uint64_t handledSignals;
            uint64_t ignoredSignals;
            uint64_t blockedSignals;
            std::unordered_map<int, uint64_t> signalCounts;
        };

        SignalStats getStats() const;
        void resetStats();

        // ===== EMERGENCY SHUTDOWN SIGNALS =====

        void setEmergencySignals(const std::vector<int>& signals);
        std::vector<int> getEmergencySignals() const;
        bool isEmergencySignal(int signal) const;

        // ===== GRACEFUL SHUTDOWN SUPPORT =====

        void setShutdownSignals(const std::vector<int>& signals);
        std::vector<int> getShutdownSignals() const;
        void requestGracefulShutdown();
        bool isShutdownRequested() const;

    private:
        SignalHandler();
        ~SignalHandler();

        WEBSOCKET_DISABLE_COPY(SignalHandler)

            struct SignalInfo {
            std::function<void(int)> handler;
            struct sigaction originalAction;
            bool isDefault;
            uint64_t count;
        };

        mutable std::mutex mutex_;
        std::unordered_map<int, SignalInfo> signalHandlers_;
        std::vector<int> emergencySignals_;
        std::vector<int> shutdownSignals_;

        std::atomic<bool> shutdownRequested_{ false };
        std::atomic<bool> handlersInstalled_{ false };

        std::atomic<uint64_t> totalSignalsReceived_{ 0 };
        std::atomic<uint64_t> handledSignals_{ 0 };
        std::atomic<uint64_t> ignoredSignals_{ 0 };
        std::atomic<uint64_t> blockedSignals_{ 0 };
        std::unordered_map<int, std::atomic<uint64_t>> signalCounts_;

        // ===== PLATFORM-SPECIFIC IMPLEMENTATIONS =====

        static void signalDispatcher(int signal);
        Result installSignalHandler(int signal, const SignalInfo& info);
        Result restoreSignalHandler(int signal);

        void handleShutdownSignal(int signal);
        void handleEmergencySignal(int signal);
        void handleDefaultSignal(int signal);

        // Signal set operations
        Result createSignalSet(const std::vector<int>& signals, sigset_t& set) const;
        Result addSignalsToSet(const std::vector<int>& signals, sigset_t& set) const;
        Result removeSignalsFromSet(const std::vector<int>& signals, sigset_t& set) const;

        // Windows compatibility
#ifdef _WIN32
        static BOOL WINAPI consoleHandler(DWORD signal);
        Result registerWindowsSignal(DWORD signal, std::function<void(int)> handler);
        Result unregisterWindowsSignal(DWORD signal);
#endif

        void updateSignalStats(int signal);
        void initializeDefaultHandlers();
};

WEBSOCKET_NAMESPACE_END