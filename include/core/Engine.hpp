#pragma once
#ifndef WEBSOCKET_ENGINE_HPP
#define WEBSOCKET_ENGINE_HPP

#include "../common/Types.hpp"
#include "../common/NonCopyable.hpp"
#include "ServiceLocator.hpp"
#include "LifecycleManager.hpp"
#include <memory>
#include <vector>
#include <string>

WEBSOCKET_NAMESPACE_BEGIN

// Forward declarations
class IService;
class IComponent;

/**
 * @class Engine
 * @brief Core coordination engine that manages all system components and services
 *
 * The Engine serves as the central nervous system of the WebSocket server,
 * responsible for:
 * - Component lifecycle management
 * - Dependency resolution and injection
 * - System initialization and shutdown sequencing
 * - Error handling and recovery
 * - Inter-component communication
 *
 * Design Pattern: Facade + Mediator
 */
class Engine : public NonCopyable {
public:
    /**
     * @brief Engine operational modes
     */
    enum class Mode {
        DEVELOPMENT,    ///< Development mode with verbose logging
        TESTING,        ///< Testing mode with mock services
        PRODUCTION      ///< Production mode with optimizations
    };

    /**
     * @brief Engine state machine
     */
    enum class State {
        UNINITIALIZED,  ///< Engine created but not initialized
        INITIALIZING,   ///< Initialization in progress
        RUNNING,        ///< Engine running normally
        STOPPING,       ///< Shutdown in progress
        STOPPED         ///< Engine fully stopped
    };

    /**
     * @brief Get singleton engine instance
     * @return Reference to global engine instance
     */
    static Engine& getInstance();

    /**
     * @brief Initialize the engine with configuration
     * @param config_path Path to configuration file
     * @return true if initialization successful
     */
    bool initialize(const std::string& config_path = "");

    /**
     * @brief Initialize with explicit configuration
     * @param config Server configuration object
     * @return true if initialization successful
     */
    bool initialize(const ServerConfig& config);

    /**
     * @brief Start all engine services and components
     * @return true if startup successful
     */
    bool start();

    /**
     * @brief Stop all engine services and components
     * @param graceful Whether to wait for graceful shutdown
     */
    void stop(bool graceful = true);

    /**
     * @brief Get current engine state
     * @return Current engine state
     */
    State getState() const;

    /**
     * @brief Check if engine is running
     * @return true if engine is in RUNNING state
     */
    bool isRunning() const;

    /**
     * @brief Register a service with the engine
     * @param service Service to register
     * @param name Service identifier
     * @return true if registration successful
     */
    bool registerService(std::shared_ptr<IService> service, const std::string& name);

    /**
     * @brief Register a component with the engine
     * @param component Component to register
     * @return true if registration successful
     */
    bool registerComponent(std::shared_ptr<IComponent> component);

    /**
     * @brief Get service by name
     * @tparam T Service type
     * @param name Service identifier
     * @return Shared pointer to service, nullptr if not found
     */
    template<typename T>
    std::shared_ptr<T> getService(const std::string& name) {
        return service_locator_.resolve<T>(name);
    }

    /**
     * @brief Get service locator for dependency injection
     * @return Reference to service locator
     */
    ServiceLocator& getServiceLocator();

    /**
     * @brief Get lifecycle manager
     * @return Reference to lifecycle manager
     */
    LifecycleManager& getLifecycleManager();

    /**
     * @brief Set engine operational mode
     * @param mode New operational mode
     */
    void setMode(Mode mode);

    /**
     * @brief Get engine operational mode
     * @return Current operational mode
     */
    Mode getMode() const;

    /**
     * @brief Get engine statistics
     * @return Engine performance and status statistics
     */
    EngineStats getStatistics() const;

    /**
     * @brief Wait for engine to stop (blocking call)
     */
    void waitForStop();

    /**
     * @brief Trigger emergency shutdown
     *
     * @note Use only in critical failure scenarios
     */
    void emergencyShutdown();

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    Engine();

    /**
     * @brief Private destructor
     */
    ~Engine();

    /**
     * @brief Initialize core components
     * @return true if core initialization successful
     */
    bool initializeCoreComponents();

    /**
     * @brief Initialize registered services
     * @return true if all services initialized successfully
     */
    bool initializeServices();

    /**
     * @brief Start registered services
     * @return true if all services started successfully
     */
    bool startServices();

    /**
     * @brief Stop registered services
     * @param graceful Whether to attempt graceful shutdown
     */
    void stopServices(bool graceful);

    /**
     * @brief Validate component dependencies
     * @return true if all dependencies are satisfied
     */
    bool validateDependencies();

    /**
     * @brief Handle component initialization errors
     * @param component_name Name of failing component
     * @param error Error description
     */
    void handleComponentError(const std::string& component_name, const std::string& error);

    // Member variables
    static std::unique_ptr<Engine> instance_;      ///< Singleton instance
    static std::once_flag init_flag_;              ///< Thread-safe initialization flag

    ServiceLocator service_locator_;               ///< Dependency injection container
    LifecycleManager lifecycle_manager_;           ///< Lifecycle sequencing
    std::atomic<State> state_{ State::UNINITIALIZED }; ///< Current engine state
    std::atomic<Mode> mode_{ Mode::PRODUCTION };     ///< Operational mode
    std::vector<std::shared_ptr<IService>> services_; ///< Registered services
    std::vector<std::shared_ptr<IComponent>> components_; ///< Registered components
    EngineStats stats_;                            ///< Engine statistics
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_ENGINE_HPP