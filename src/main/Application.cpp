#include "main/Application.hpp"
#include "utils/Logger.hpp"
#include "utils/Metrics.hpp"
#include <iostream>
#include <csignal>

WEBSOCKET_NAMESPACE_BEGIN

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

Application::Application()
    : configManager_(std::make_shared<ConfigManager>())
    , runtimeConfig_(std::make_shared<RuntimeConfig>())
    , engine_(std::make_shared<Engine>())
    , serviceLocator_(std::make_shared<ServiceLocator>())
    , lifecycleManager_(std::make_shared<LifecycleManager>()) {

    LOG_INFO("Application created");
}

Application::~Application() {
    if (isRunning_) {
        emergencyShutdown();
    }
    LOG_INFO("Application destroyed");
}

Result Application::initialize(int argc, char* argv[]) {
    if (isInitialized_) {
        LOG_WARN("Application already initialized");
        return Result::SUCCESS;
    }

    try {
        LOG_INFO("Initializing WebSocket Server...");

        // Parse command line first
        if (argc > 0) {
            parseCommandLine(argc, argv);
        }

        // Initialize subsystems in order
        Result result = initializeLogging();
        if (result != Result::SUCCESS) {
            LOG_ERROR("Failed to initialize logging");
            return result;
        }

        result = initializeConfiguration();
        if (result != Result::SUCCESS) {
            LOG_ERROR("Failed to initialize configuration");
            return result;
        }

        result = initializeServices();
        if (result != Result::SUCCESS) {
            LOG_ERROR("Failed to initialize services");
            return result;
        }

        // Setup signal handlers for graceful shutdown
        setupSignalHandlers();

        isInitialized_ = true;
        startTime_ = std::chrono::steady_clock::now();

        LOG_INFO("Application initialized successfully");
        logStartupInfo();

        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Application initialization failed: {}", e.what());
        lastError_ = errorFromException(e);
        return Result::ERROR;
    }
}

Result Application::run() {
    if (!isInitialized_) {
        LOG_ERROR("Application not initialized");
        return Result::INVALID_STATE;
    }

    if (isRunning_) {
        LOG_WARN("Application already running");
        return Result::SUCCESS;
    }

    try {
        LOG_INFO("Starting WebSocket Server...");

        // Start the engine
        Result result = engine_->start();
        if (result != Result::SUCCESS) {
            LOG_ERROR("Failed to start engine");
            return result;
        }

        isRunning_ = true;

        // Main application loop
        while (isRunning_ && !shutdownRequested_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Check for shutdown signal
            if (shutdownRequested_) {
                break;
            }
        }

        LOG_INFO("Application run loop completed");
        return shutdown();

    }
    catch (const std::exception& e) {
        LOG_ERROR("Application run failed: {}", e.what());
        lastError_ = errorFromException(e);
        return emergencyShutdown();
    }
}

Result Application::shutdown() {
    if (!isInitialized_) {
        return Result::SUCCESS;
    }

    LOG_INFO("Initiating application shutdown...");

    try {
        // Shutdown in reverse order of initialization
        Result result = shutdownNetwork();
        if (result != Result::SUCCESS) {
            LOG_WARN("Network shutdown had issues");
        }

        result = shutdownServices();
        if (result != Result::SUCCESS) {
            LOG_WARN("Service shutdown had issues");
        }

        result = cleanup();
        if (result != Result::SUCCESS) {
            LOG_WARN("Cleanup had issues");
        }

        isRunning_ = false;
        isInitialized_ = false;

        logShutdownInfo();
        LOG_INFO("Application shutdown completed");

        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Application shutdown failed: {}", e.what());
        lastError_ = errorFromException(e);
        return Result::ERROR;
    }
}

Result Application::emergencyShutdown() {
    LOG_ERROR("Initiating emergency shutdown!");

    try {
        // Force stop everything without proper sequencing
        if (engine_) {
            engine_->emergencyShutdown();
        }

        if (lifecycleManager_) {
            lifecycleManager_->emergencyShutdownAll();
        }

        isRunning_ = false;
        isInitialized_ = false;

        LOG_ERROR("Emergency shutdown completed");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Emergency shutdown failed: {}", e.what());
        return Result::ERROR;
    }
}

bool Application::isRunning() const {
    return isRunning_;
}

bool Application::isInitialized() const {
    return isInitialized_;
}

std::string Application::getStatus() const {
    if (!isInitialized_) return "Not Initialized";
    if (shutdownRequested_) return "Shutting Down";
    if (isRunning_) return "Running";
    return "Stopped";
}

void Application::setConfigFile(const std::string& configPath) {
    configPath_ = configPath;
}

void Application::setConfigOverrides(const std::unordered_map<std::string, std::any>& overrides) {
    configOverrides_ = overrides;
}

void Application::reloadConfig() {
    if (configManager_) {
        configManager_->reload();
        LOG_INFO("Configuration reloaded");
    }
}

void Application::parseCommandLine(int argc, char* argv[]) {
    try {
        cmdParser_.parse(argc, argv);

        // Apply command line overrides
        if (cmdParser_.hasOption("config")) {
            configPath_ = cmdParser_.getOptionValue<std::string>("config");
        }

        if (cmdParser_.hasOption("help")) {
            showHelp();
            exit(0);
        }

        if (cmdParser_.hasOption("version")) {
            showVersion();
            exit(0);
        }

    }
    catch (const std::exception& e) {
        LOG_ERROR("Command line parsing failed: {}", e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        showUsage();
        exit(1);
    }
}

void Application::showHelp() const {
    std::cout << "WebSocket Server Usage:" << std::endl;
    std::cout << "  --config <file>    Configuration file path" << std::endl;
    std::cout << "  --port <number>    Server port (default: 8080)" << std::endl;
    std::cout << "  --threads <num>    Worker threads (default: auto)" << std::endl;
    std::cout << "  --daemon           Run as daemon process" << std::endl;
    std::cout << "  --help             Show this help message" << std::endl;
    std::cout << "  --version          Show version information" << std::endl;
}

void Application::showVersion() const {
    std::cout << Version::getFullVersion() << std::endl;
}

Result Application::daemonize() {
    if (isDaemon_) {
        return Result::SUCCESS;
    }

    try {
        Result result = Daemonizer::getInstance().daemonize();
        if (result == Result::SUCCESS) {
            isDaemon_ = true;
            LOG_INFO("Application running as daemon");
        }
        return result;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Daemonization failed: {}", e.what());
        return Result::ERROR;
    }
}

bool Application::isDaemon() const {
    return isDaemon_;
}

void Application::setupSignalHandlers() {
    try {
        SignalHandler::getInstance().setupSignalHandlers();
        SignalHandler::getInstance().setOnSignal([this](int signal) {
            handleSignal(signal);
            });
        LOG_DEBUG("Signal handlers installed");

    }
    catch (const std::exception& e) {
        LOG_ERROR("Failed to setup signal handlers: {}", e.what());
    }
}

void Application::handleSignal(int signal) {
    LOG_INFO("Received signal: {}", signal);

    switch (signal) {
    case SIGINT:
    case SIGTERM:
        LOG_INFO("Initiating graceful shutdown...");
        shutdownRequested_ = true;
        break;
    case SIGHUP:
        LOG_INFO("Reloading configuration...");
        reloadConfig();
        break;
    default:
        LOG_WARN("Unhandled signal: {}", signal);
        break;
    }
}

std::string Application::getName() const {
    return "CppWebSocket-Server";
}

std::string Application::getVersion() const {
    return Version::toString();
}

std::string Application::getDescription() const {
    return "High-performance RFC 6455 WebSocket Server";
}

Application::AppStats Application::getStats() const {
    AppStats stats;
    stats.startTime = startTime_;
    stats.uptime = std::chrono::steady_clock::now() - startTime_;
    stats.totalConnections = engine_ ? engine_->getConnectionCount() : 0;
    stats.totalMessages = engine_ ? engine_->getMessageCount() : 0;
    stats.totalErrors = engine_ ? engine_->getErrorCount() : 0;
    stats.memoryUsageMB = 0.0; // Would get from system metrics

    return stats;
}

// Private initialization methods
Result Application::initializeLogging() {
    try {
        // Get log level from configuration or command line
        std::string logLevel = "INFO";
        if (cmdParser_.hasOption("log-level")) {
            logLevel = cmdParser_.getOptionValue<std::string>("log-level");
        }

        // Convert string to LogLevel
        LogLevel level = LogLevel::INFO;
        if (logLevel == "TRACE") level = LogLevel::TRACE;
        else if (logLevel == "DEBUG") level = LogLevel::DEBUG;
        else if (logLevel == "WARN") level = LogLevel::WARN;
        else if (logLevel == "ERROR") level = LogLevel::ERROR;
        else if (logLevel == "FATAL") level = LogLevel::FATAL;

        std::string logFile = "";
        if (cmdParser_.hasOption("log-file")) {
            logFile = cmdParser_.getOptionValue<std::string>("log-file");
        }

        Logger::getInstance().initialize(logFile, level);
        LOG_INFO("Logging initialized with level: {}", logLevel);

        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize logging: " << e.what() << std::endl;
        return Result::ERROR;
    }
}

Result Application::initializeConfiguration() {
    try {
        // Load configuration from file if specified
        if (!configPath_.empty()) {
            if (!configManager_->loadFromFile(configPath_)) {
                LOG_WARN("Failed to load configuration from: {}", configPath_);
            }
            else {
                LOG_INFO("Configuration loaded from: {}", configPath_);
            }
        }

        // Apply command line overrides to configuration
        auto config = cmdParser_.generateConfig();
        for (const auto& [key, value] : config) {
            configManager_->set(key, value);
        }

        // Apply runtime overrides
        for (const auto& [key, value] : configOverrides_) {
            configManager_->setRuntimeOverride(key, value);
        }

        // Validate configuration
        if (!configManager_->validate()) {
            auto errors = configManager_->getValidationErrors();
            for (const auto& error : errors) {
                LOG_ERROR("Configuration error: {}", error);
            }
            return Result::ERROR;
        }

        LOG_INFO("Configuration initialized successfully");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Configuration initialization failed: {}", e.what());
        return Result::ERROR;
    }
}

Result Application::initializeServices() {
    try {
        // Register core services
        serviceLocator_->registerService<Engine>(engine_);
        serviceLocator_->registerService<ConfigManager>(configManager_);
        serviceLocator_->registerService<RuntimeConfig>(runtimeConfig_);

        // Initialize services
        Result result = serviceLocator_->initializeAllServices();
        if (result != Result::SUCCESS) {
            LOG_ERROR("Service initialization failed");
            return result;
        }

        LOG_INFO("Services initialized successfully");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Service initialization failed: {}", e.what());
        return Result::ERROR;
    }
}

Result Application::initializeNetwork() {
    // Network initialization would go here
    LOG_DEBUG("Network initialization placeholder");
    return Result::SUCCESS;
}

Result Application::initializeProtocol() {
    // Protocol initialization would go here
    LOG_DEBUG("Protocol initialization placeholder");
    return Result::SUCCESS;
}

// Private shutdown methods
Result Application::shutdownNetwork() {
    LOG_DEBUG("Shutting down network...");
    return Result::SUCCESS;
}

Result Application::shutdownServices() {
    try {
        if (serviceLocator_) {
            serviceLocator_->shutdownAllServices();
        }
        LOG_DEBUG("Services shutdown completed");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Service shutdown failed: {}", e.what());
        return Result::ERROR;
    }
}

Result Application::cleanup() {
    try {
        // Cleanup resources
        if (engine_) {
            engine_->shutdown();
        }

        LOG_DEBUG("Cleanup completed");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        LOG_ERROR("Cleanup failed: {}", e.what());
        return Result::ERROR;
    }
}

void Application::handleShutdown() {
    shutdown();
}

void Application::handleConfigChange() {
    reloadConfig();
}

void Application::logStartupInfo() {
    LOG_INFO("=== WebSocket Server Starting ===");
    LOG_INFO("Version: {}", Version::getFullVersion());
    LOG_INFO("Platform: {}", getPlatformName());
    LOG_INFO("Process ID: {}", getCurrentProcessId());
    LOG_INFO("Configuration: {}", configPath_.empty() ? "default" : configPath_);
    LOG_INFO("=================================");
}

void Application::logShutdownInfo() {
    auto stats = getStats();
    LOG_INFO("=== WebSocket Server Shutting Down ===");
    LOG_INFO("Uptime: {:.2f} seconds", stats.uptime.count());
    LOG_INFO("Total Connections: {}", stats.totalConnections);
    LOG_INFO("Total Messages: {}", stats.totalMessages);
    LOG_INFO("Total Errors: {}", stats.totalErrors);
    LOG_INFO("======================================");
}

WEBSOCKET_NAMESPACE_END