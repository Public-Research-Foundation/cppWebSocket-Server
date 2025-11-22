#include "core/interfaces/IService.hpp"
#include "utils/Logger.hpp"
#include <chrono>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * ServiceBase - Partial implementation of IService interface
 * Provides common functionality for service implementations
 */
    class ServiceBase : public IService {
    protected:
        std::atomic<ServiceState> state_{ ServiceState::UNINITIALIZED };
        std::atomic<bool> isRunning_{ false };
        std::chrono::steady_clock::time_point startTime_;
        Error lastError_;
        std::atomic<uint64_t> requestCount_{ 0 };
        std::atomic<uint64_t> errorCount_{ 0 };
        std::unordered_map<std::string, std::any> configuration_;

    public:
        ServiceBase() = default;
        virtual ~ServiceBase() = default;

        // Common service lifecycle implementation
        virtual Result initialize() override {
            if (state_ != ServiceState::UNINITIALIZED) {
                return Result::INVALID_STATE;
            }

            state_ = ServiceState::INITIALIZING;
            try {
                Result result = onInitialize();
                if (result == Result::SUCCESS) {
                    state_ = ServiceState::INITIALIZED;
                    LOG_INFO("Service initialized: {}", getName());
                }
                else {
                    state_ = ServiceState::ERROR;
                    LOG_ERROR("Service initialization failed: {}", getName());
                }
                return result;
            }
            catch (const std::exception& e) {
                state_ = ServiceState::ERROR;
                lastError_ = errorFromException(e);
                LOG_ERROR("Service initialization exception: {} - {}", getName(), e.what());
                return Result::ERROR;
            }
        }

        virtual Result start() override {
            if (state_ != ServiceState::INITIALIZED) {
                return Result::INVALID_STATE;
            }

            state_ = ServiceState::STARTING;
            try {
                Result result = onStart();
                if (result == Result::SUCCESS) {
                    state_ = ServiceState::RUNNING;
                    isRunning_ = true;
                    startTime_ = std::chrono::steady_clock::now();
                    LOG_INFO("Service started: {}", getName());
                }
                else {
                    state_ = ServiceState::ERROR;
                    LOG_ERROR("Service start failed: {}", getName());
                }
                return result;
            }
            catch (const std::exception& e) {
                state_ = ServiceState::ERROR;
                lastError_ = errorFromException(e);
                LOG_ERROR("Service start exception: {} - {}", getName(), e.what());
                return Result::ERROR;
            }
        }

        virtual Result stop() override {
            if (state_ != ServiceState::RUNNING) {
                return Result::SUCCESS;
            }

            state_ = ServiceState::STOPPING;
            try {
                Result result = onStop();
                if (result == Result::SUCCESS) {
                    state_ = ServiceState::STOPPED;
                    isRunning_ = false;
                    LOG_INFO("Service stopped: {}", getName());
                }
                else {
                    state_ = ServiceState::ERROR;
                    LOG_ERROR("Service stop failed: {}", getName());
                }
                return result;
            }
            catch (const std::exception& e) {
                state_ = ServiceState::ERROR;
                lastError_ = errorFromException(e);
                LOG_ERROR("Service stop exception: {} - {}", getName(), e.what());
                return Result::ERROR;
            }
        }

        virtual Result shutdown() override {
            if (state_ == ServiceState::SHUTDOWN) {
                return Result::SUCCESS;
            }

            // Stop if running
            if (isRunning_) {
                stop();
            }

            try {
                Result result = onShutdown();
                state_ = ServiceState::SHUTDOWN;
                isRunning_ = false;
                LOG_INFO("Service shutdown: {}", getName());
                return result;
            }
            catch (const std::exception& e) {
                state_ = ServiceState::ERROR;
                lastError_ = errorFromException(e);
                LOG_ERROR("Service shutdown exception: {} - {}", getName(), e.what());
                return Result::ERROR;
            }
        }

        // Common state queries
        virtual bool isRunning() const override {
            return isRunning_;
        }

        virtual bool isInitialized() const override {
            return state_ >= ServiceState::INITIALIZED && state_ != ServiceState::ERROR;
        }

        virtual ServiceState getState() const override {
            return state_;
        }

        virtual std::string getStatusMessage() const override {
            switch (state_) {
            case ServiceState::UNINITIALIZED: return "Uninitialized";
            case ServiceState::INITIALIZING: return "Initializing";
            case ServiceState::INITIALIZED: return "Initialized";
            case ServiceState::STARTING: return "Starting";
            case ServiceState::RUNNING: return "Running";
            case ServiceState::STOPPING: return "Stopping";
            case ServiceState::STOPPED: return "Stopped";
            case ServiceState::SHUTDOWN: return "Shutdown";
            case ServiceState::ERROR: return "Error: " + lastError_.message;
            default: return "Unknown";
            }
        }

        // Common error handling
        virtual Error getLastError() const override {
            return lastError_;
        }

        virtual void clearError() override {
            lastError_ = Error();
            if (state_ == ServiceState::ERROR) {
                state_ = ServiceState::UNINITIALIZED;
            }
        }

        virtual bool hasError() const override {
            return lastError_.code != 0;
        }

        // Common configuration
        virtual void setConfiguration(const std::unordered_map<std::string, std::any>& config) override {
            configuration_ = config;
            onConfigurationChanged(config);
        }

        virtual std::unordered_map<std::string, std::any> getConfiguration() const override {
            return configuration_;
        }

        // Common statistics
        virtual std::chrono::steady_clock::time_point getStartTime() const override {
            return startTime_;
        }

        virtual std::chrono::duration<double> getUptime() const override {
            if (!isRunning_) {
                return std::chrono::duration<double>::zero();
            }
            return std::chrono::steady_clock::now() - startTime_;
        }

        virtual uint64_t getRequestCount() const override {
            return requestCount_;
        }

        virtual uint64_t getErrorCount() const override {
            return errorCount_;
        }

    protected:
        // Protected methods for derived classes to implement
        virtual Result onInitialize() = 0;
        virtual Result onStart() = 0;
        virtual Result onStop() = 0;
        virtual Result onShutdown() = 0;
        virtual void onConfigurationChanged(const std::unordered_map<std::string, std::any>& config) {}

        // Utility methods for derived classes
        void incrementRequestCount() { requestCount_++; }
        void incrementErrorCount() { errorCount_++; }
        void setError(const Error& error) { lastError_ = error; errorCount_++; }
};

WEBSOCKET_NAMESPACE_END