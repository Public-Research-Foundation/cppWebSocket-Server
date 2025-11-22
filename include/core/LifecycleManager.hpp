#pragma once
#ifndef WEBSOCKET_LIFECYCLE_MANAGER_HPP
#define WEBSOCKET_LIFECYCLE_MANAGER_HPP

#include "../common/Types.hpp"
#include "../common/NonCopyable.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

// Forward declarations
class IComponent;
class IService;

/**
 * @class LifecycleManager
 * @brief Manages initialization and shutdown sequencing for system components
 *
 * Ensures proper ordering of component lifecycle events:
 * - Dependency-based initialization sequencing
 * - Ordered shutdown (reverse of initialization)
 * - Error handling and recovery during startup/shutdown
 * - Timeout management for slow operations
 *
 * Design Pattern: Dependency-based Orchestrator
 */
class LifecycleManager : public NonCopyable {
public:
    /**
     * @brief Component lifecycle states
     */
    enum class State {
        UNINITIALIZED,  ///< Component not initialized
        INITIALIZING,   ///< Initialization in progress
        INITIALIZED,    ///< Successfully initialized
        STARTING,       ///< Startup in progress
        RUNNING,        ///< Component running
        STOPPING,       ///< Shutdown in progress
        STOPPED         ///< Component fully stopped
    };

    /**
     * @brief Lifecycle phase definitions
     */
    enum class Phase {
        CORE,           ///< Core system components (logging, config)
        INFRASTRUCTURE, ///< Infrastructure services (thread pools, networking)
        SERVICES,       ///< Business logic services
        APPLICATION     ///< Application-level components
    };

    /**
     * @brief Lifecycle event information
     */
    struct LifecycleEvent {
        std::string component_name;     ///< Name of affected component
        State old_state;                ///< Previous state
        State new_state;                ///< New state
        std::string message;            ///< Event description
        std::chrono::steady_clock::time_point timestamp; ///< Event time
    };

    /**
     * @brief Component registration information
     */
    struct ComponentInfo {
        std::string name;                       ///< Component name
        std::shared_ptr<IComponent> component;  ///< Component instance
        Phase phase;                            ///< Initialization phase
        std::vector<std::string> dependencies;  ///< Required components
        State state{ State::UNINITIALIZED };      ///< Current state
        std::string error_message;              ///< Last error message
    };

    /**
     * @brief Default constructor
     */
    LifecycleManager();

    /**
     * @brief Destructor
     */
    ~LifecycleManager();

    /**
     * @brief Register a component with lifecycle management
     * @param component Component to register
     * @param name Component identifier
     * @param phase Initialization phase
     * @param dependencies List of required component names
     * @return true if registration successful
     */
    bool registerComponent(std::shared_ptr<IComponent> component,
        const std::string& name,
        Phase phase = Phase::SERVICES,
        const std::vector<std::string>& dependencies = {});

    /**
     * @brief Initialize all registered components
     * @return true if all components initialized successfully
     *
     * @note Components are initialized in dependency order within their phases
     */
    bool initializeAll();

    /**
     * @brief Start all registered components
     * @return true if all components started successfully
     */
    bool startAll();

    /**
     * @brief Stop all registered components
     * @param graceful Whether to attempt graceful shutdown
     * @return true if all components stopped successfully
     */
    bool stopAll(bool graceful = true);

    /**
     * @brief Initialize a specific component
     * @param name Component name to initialize
     * @return true if component initialized successfully
     */
    bool initializeComponent(const std::string& name);

    /**
     * @brief Start a specific component
     * @param name Component name to start
     * @return true if component started successfully
     */
    bool startComponent(const std::string& name);

    /**
     * @brief Stop a specific component
     * @param name Component name to stop
     * @param graceful Whether to attempt graceful shutdown
     * @return true if component stopped successfully
     */
    bool stopComponent(const std::string& name, bool graceful = true);

    /**
     * @brief Get component current state
     * @param name Component name
     * @return Current state, or UNINITIALIZED if not found
     */
    State getComponentState(const std::string& name) const;

    /**
     * @brief Check if all components are running
     * @return true if all components in RUNNING state
     */
    bool allComponentsRunning() const;

    /**
     * @brief Check if all components are stopped
     * @return true if all components in STOPPED state
     */
    bool allComponentsStopped() const;

    /**
     * @brief Get lifecycle event history
     * @return Vector of recent lifecycle events
     */
    std::vector<LifecycleEvent> getEventHistory() const;

    /**
     * @brief Set initialization timeout
     * @param timeout Timeout duration in milliseconds
     */
    void setInitializationTimeout(uint64_t timeout);

    /**
     * @brief Set shutdown timeout
     * @param timeout Timeout duration in milliseconds
     */
    void setShutdownTimeout(uint64_t timeout);

    /**
     * @brief Add lifecycle event listener
     * @param callback Function to call on lifecycle events
     */
    void addEventListener(std::function<void(const LifecycleEvent&)> callback);

    /**
     * @brief Get dependency graph as DOT format (for visualization)
     * @return DOT format string representing component dependencies
     */
    std::string getDependencyGraph() const;

private:
    /**
     * @brief Find component info by name
     * @param name Component name
     * @return Pointer to component info, nullptr if not found
     */
    ComponentInfo* findComponentInfo(const std::string& name);

    /**
     * @brief Find component info by name (const version)
     * @param name Component name
     * @return Pointer to component info, nullptr if not found
     */
    const ComponentInfo* findComponentInfo(const std::string& name) const;

    /**
     * @brief Resolve component initialization order
     * @return Vector of component names in initialization order
     */
    std::vector<std::string> resolveInitializationOrder();

    /**
     * @brief Check for circular dependencies
     * @return true if circular dependencies detected
     */
    bool hasCircularDependencies() const;

    /**
     * @brief Perform topological sort of components
     * @param components Component names to sort
     * @return Sorted component names
     */
    std::vector<std::string> topologicalSort(const std::vector<std::string>& components) const;

    /**
     * @brief Record lifecycle event
     * @param event Event to record
     */
    void recordEvent(const LifecycleEvent& event);

    /**
     * @brief Update component state
     * @param name Component name
     * @param new_state New state
     * @param message State change message
     */
    void updateComponentState(const std::string& name, State new_state, const std::string& message = "");

    /**
     * @brief Wait for component to reach specific state
     * @param name Component name
     * @param target_state Desired state
     * @param timeout_ms Timeout in milliseconds
     * @return true if state reached within timeout
     */
    bool waitForComponentState(const std::string& name, State target_state, uint64_t timeout_ms);

    // Member variables
    std::vector<ComponentInfo> components_;
    std::vector<LifecycleEvent> event_history_;
    std::vector<std::function<void(const LifecycleEvent&)>> event_listeners_;
    uint64_t initialization_timeout_{ 30000 };  ///< 30 seconds default
    uint64_t shutdown_timeout_{ 30000 };        ///< 30 seconds default
    mutable std::mutex mutex_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_LIFECYCLE_MANAGER_HPP