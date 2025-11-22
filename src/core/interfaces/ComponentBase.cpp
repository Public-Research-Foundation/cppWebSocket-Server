#include "core/interfaces/IComponent.hpp"
#include "utils/Logger.hpp"

WEBSOCKET_NAMESPACE_BEGIN

/**
 * ComponentBase - Partial implementation of IComponent interface
 * Provides common functionality for component implementations
 */
    class ComponentBase : public IComponent {
    protected:
        std::atomic<ComponentState> state_{ ComponentState::CREATED };
        std::atomic<bool> enabled_{ true };
        std::string componentName_;
        std::string componentId_;
        std::string componentType_;
        std::string componentVersion_;
        Error lastError_;
        std::vector<Error> errorHistory_;
        std::unordered_map<std::string, std::any> configuration_;

    public:
        ComponentBase(const std::string& name, const std::string& type = "generic")
            : componentName_(name), componentType_(type) {
            // Generate unique ID
            componentId_ = generateComponentId();
            componentVersion_ = "1.0.0";
        }

        virtual ~ComponentBase() = default;

        // Common component lifecycle
        virtual Result initialize() override {
            if (state_ != ComponentState::CREATED && state_ != ComponentState::CONFIGURED) {
                return Result::INVALID_STATE;
            }

            if (!enabled_) {
                state_ = ComponentState::INITIALIZED;
                return Result::SUCCESS;
            }

            try {
                Result result = onInitialize();
                if (result == Result::SUCCESS) {
                    state_ = ComponentState::INITIALIZED;
                    LOG_DEBUG("Component initialized: {}", componentName_);
                }
                else {
                    state_ = ComponentState::ERROR;
                    LOG_ERROR("Component initialization failed: {}", componentName_);
                }
                return result;
            }
            catch (const std::exception& e) {
                state_ = ComponentState::ERROR;
                lastError_ = errorFromException(e);
                errorHistory_.push_back(lastError_);
                LOG_ERROR("Component initialization exception: {} - {}", componentName_, e.what());
                return Result::ERROR;
            }
        }

        virtual Result configure() override {
            if (state_ != ComponentState::CREATED) {
                return Result::INVALID_STATE;
            }

            try {
                Result result = onConfigure();
                if (result == Result::SUCCESS) {
                    state_ = ComponentState::CONFIGURED;
                    LOG_DEBUG("Component configured: {}", componentName_);
                }
                return result;
            }
            catch (const std::exception& e) {
                state_ = ComponentState::ERROR;
                lastError_ = errorFromException(e);
                errorHistory_.push_back(lastError_);
                LOG_ERROR("Component configuration exception: {} - {}", componentName_, e.what());
                return Result::ERROR;
            }
        }

        virtual Result start() override {
            if (state_ != ComponentState::INITIALIZED) {
                return Result::INVALID_STATE;
            }

            if (!enabled_) {
                state_ = ComponentState::STARTED;
                return Result::SUCCESS;
            }

            try {
                Result result = onStart();
                if (result == Result::SUCCESS) {
                    state_ = ComponentState::STARTED;
                    LOG_DEBUG("Component started: {}", componentName_);
                }
                else {
                    state_ = ComponentState::ERROR;
                    LOG_ERROR("Component start failed: {}", componentName_);
                }
                return result;
            }
            catch (const std::exception& e) {
                state_ = ComponentState::ERROR;
                lastError_ = errorFromException(e);
                errorHistory_.push_back(lastError_);
                LOG_ERROR("Component start exception: {} - {}", componentName_, e.what());
                return Result::ERROR;
            }
        }

        virtual Result stop() override {
            if (state_ != ComponentState::STARTED) {
                return Result::SUCCESS;
            }

            try {
                Result result = onStop();
                if (result == Result::SUCCESS) {
                    state_ = ComponentState::STOPPED;
                    LOG_DEBUG("Component stopped: {}", componentName_);
                }
                return result;
            }
            catch (const std::exception& e) {
                state_ = ComponentState::ERROR;
                lastError_ = errorFromException(e);
                errorHistory_.push_back(lastError_);
                LOG_ERROR("Component stop exception: {} - {}", componentName_, e.what());
                return Result::ERROR;
            }
        }

        virtual Result destroy() override {
            try {
                Result result = onDestroy();
                state_ = ComponentState::DESTROYED;
                enabled_ = false;
                LOG_DEBUG("Component destroyed: {}", componentName_);
                return result;
            }
            catch (const std::exception& e) {
                state_ = ComponentState::ERROR;
                lastError_ = errorFromException(e);
                errorHistory_.push_back(lastError_);
                LOG_ERROR("Component destroy exception: {} - {}", componentName_, e.what());
                return Result::ERROR;
            }
        }

        // Common state management
        virtual bool isEnabled() const override {
            return enabled_;
        }

        virtual void setEnabled(bool enabled) override {
            enabled_ = enabled;
        }

        virtual ComponentState getState() const override {
            return state_;
        }

        virtual bool isOperational() const override {
            return enabled_ && (state_ == ComponentState::STARTED || state_ == ComponentState::INITIALIZED);
        }

        // Common identification
        virtual std::string getComponentName() const override {
            return componentName_;
        }

        virtual std::string getComponentVersion() const override {
            return componentVersion_;
        }

        virtual std::string getComponentType() const override {
            return componentType_;
        }

        virtual std::string getComponentId() const override {
            return componentId_;
        }

        // Common dependencies (empty by default)
        virtual std::vector<std::string> getDependencies() const override {
            return {};
        }

        virtual std::vector<std::string> getDependents() const override {
            return {};
        }

        virtual bool hasDependency(const std::string& componentId) const override {
            auto deps = getDependencies();
            return std::find(deps.begin(), deps.end(), componentId) != deps.end();
        }

        // Common configuration
        virtual void setConfigurationParameter(const std::string& key, const std::any& value) override {
            configuration_[key] = value;
            onConfigurationParameterChanged(key, value);
        }

        virtual std::any getConfigurationParameter(const std::string& key) const override {
            auto it = configuration_.find(key);
            return it != configuration_.end() ? it->second : std::any();
        }

        virtual bool hasConfigurationParameter(const std::string& key) const override {
            return configuration_.find(key) != configuration_.end();
        }

        // Common health and metrics
        virtual bool isHealthy() const override {
            return enabled_ && state_ != ComponentState::ERROR && lastError_.code == 0;
        }

        virtual std::string getHealthStatus() const override {
            if (!enabled_) return "Disabled";
            if (state_ == ComponentState::ERROR) return "Error: " + lastError_.message;
            if (!isHealthy()) return "Unhealthy";
            return "Healthy";
        }

        virtual std::unordered_map<std::string, std::any> getMetrics() const override {
            return {
                {"enabled", enabled_},
                {"state", static_cast<int>(state_)},
                {"error_count", static_cast<int>(errorHistory_.size())},
                {"has_error", lastError_.code != 0}
            };
        }

        // Common error handling
        virtual Error getLastError() const override {
            return lastError_;
        }

        virtual std::vector<Error> getErrorHistory() const override {
            return errorHistory_;
        }

        virtual void clearErrors() override {
            lastError_ = Error();
            errorHistory_.clear();
            if (state_ == ComponentState::ERROR) {
                state_ = ComponentState::CREATED;
            }
        }

        // Common resource management
        virtual size_t getMemoryUsage() const override {
            return 0; // Default implementation
        }

        virtual size_t getResourceCount() const override {
            return 0; // Default implementation
        }

        virtual void setResourceLimit(const std::string& resourceType, size_t limit) override {
            // Default implementation does nothing
        }

    protected:
        // Protected methods for derived classes to implement
        virtual Result onInitialize() { return Result::SUCCESS; }
        virtual Result onConfigure() { return Result::SUCCESS; }
        virtual Result onStart() { return Result::SUCCESS; }
        virtual Result onStop() { return Result::SUCCESS; }
        virtual Result onDestroy() { return Result::SUCCESS; }
        virtual void onConfigurationParameterChanged(const std::string& key, const std::any& value) {}
        virtual void update() override {} // Default empty implementation

    private:
        std::string generateComponentId() {
            // Simple ID generation - in production use UUID
            static std::atomic<uint64_t> counter{ 0 };
            return componentName_ + "_" + std::to_string(++counter);
        }
};

WEBSOCKET_NAMESPACE_END