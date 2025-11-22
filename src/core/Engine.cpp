#include "core/Engine.hpp"
#include "utils/Logger.hpp"
#include "utils/Metrics.hpp"
#include <thread>
#include <chrono>

WEBSOCKET_NAMESPACE_BEGIN

Engine::Engine(){} : serviceLocator_(std::make_shared<ServiceLocator>())
    , componentManager_(std::make_shared<ComponentManager>())
    , startTime_(std::chrono::steady_clock::now()) {

    LOG_INFO("Engine created");
}

Engine::~Engine() {
    if (isRunning_) {
        shutdown();
    }
    LOG_INFO("Engine destroyed");
}

Result Engine::initialize() {
    std::unique_lock lock(stateMutex_);

    if (isInitialized_) {
        LOG_WARN("Engine already initialized");
        return Result::SUCCESS;
    }

    try {
        LOG_INFO("Initializing engine...");

        // Initialize service locator first
        serviceLocator_->initializeAllServices();

        // Initialize component manager
        componentManager_->initializeAll();

        // Initialize worker threads
        initializeComponents();

        state_ = ServiceState::INITIALIZED;
        isInitialized_ = true;

        LOG_INFO("Engine initialized successfully");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        Error error = errorFromException(e);
        handleError(error);
        return Result::ERROR;
    }
}

Result Engine::start() {
    std::unique_lock lock(stateMutex_);

    if (!isInitialized_) {
        Error error(-1, "Engine not initialized");
        handleError(error);
        return Result::INVALID_STATE;
    }

    if (isRunning_) {
        LOG_WARN("Engine already running");
        return Result::SUCCESS;
    }

    try {
        LOG_INFO("Starting engine...");

        // Start all services
        serviceLocator_->startAllServices();

        // Start all components
        componentManager_->startAll();

        // Start worker threads
        for (size_t i = 0; i < processingThreads_; ++i) {
            workerThreads_.emplace_back(&Engine::workerThreadFunction, this);
        }

        isRunning_ = true;
        state_ = ServiceState::RUNNING;

        LOG_INFO("Engine started with {} worker threads", processingThreads_);
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        Error error = errorFromException(e);
        handleError(error);
        return Result::ERROR;
    }
}

Result Engine::stop() {
    std::unique_lock lock(stateMutex_);

    if (!isRunning_) {
        return Result::SUCCESS;
    }

    try {
        LOG_INFO("Stopping engine...");

        isRunning_ = false;
        state_ = ServiceState::STOPPING;

        // Notify all worker threads to stop
        queueCondition_.notify_all();

        // Wait for worker threads to finish
        for (auto& thread : workerThreads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        workerThreads_.clear();

        // Stop components and services
        componentManager_->stopAll();
        serviceLocator_->stopAllServices();

        state_ = ServiceState::STOPPED;

        LOG_INFO("Engine stopped");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        Error error = errorFromException(e);
        handleError(error);
        return Result::ERROR;
    }
}

Result Engine::shutdown() {
    std::unique_lock lock(stateMutex_);

    LOG_INFO("Shutting down engine...");

    // Stop if running
    if (isRunning_) {
        stop();
    }

    try {
        // Shutdown components and services
        componentManager_->shutdownAll();
        serviceLocator_->shutdownAllServices();

        // Clear message queue
        std::queue<MessageTask> emptyQueue;
        std::swap(messageQueue_, emptyQueue);

        isInitialized_ = false;
        state_ = ServiceState::SHUTDOWN;

        LOG_INFO("Engine shutdown complete");
        return Result::SUCCESS;

    }
    catch (const std::exception& e) {
        Error error = errorFromException(e);
        handleError(error);
        return Result::ERROR;
    }
}

bool Engine::isRunning() const {
    return isRunning_;
}

std::string Engine::getName() const {
    return "WebSocketEngine";
}

std::string Engine::getStatus() const {
    switch (state_) {
    case ServiceState::UNINITIALIZED: return "Uninitialized";
    case ServiceState::INITIALIZING: return "Initializing";
    case ServiceState::INITIALIZED: return "Initialized";
    case ServiceState::STARTING: return "Starting";
    case ServiceState::RUNNING: return "Running";
    case ServiceState::STOPPING: return "Stopping";
    case ServiceState::STOPPED: return "Stopped";
    case ServiceState::SHUTDOWN: return "Shutdown";
    case ServiceState::ERROR: return "Error";
    default: return "Unknown";
    }
}

Result Engine::processMessage(const ByteBuffer& message, const std::string& sessionId) {
    if (!isRunning_) {
        return Result::INVALID_STATE;
    }

    if (message.size() > maxMessageSize_) {
        LOG_WARN("Message too large: {} bytes", message.size());
        return Result::ERROR;
    }

    return processMessageInternal(message, sessionId);
}

Result Engine::processMessageAsync(const ByteBuffer& message, const std::string& sessionId, Callback completionCallback) {
    if (!isRunning_) {
        return Result::INVALID_STATE;
    }

    if (!canAcceptMessage()) {
        queueDrops_++;
        LOG_WARN("Message queue full, dropping message");
        return Result::ERROR;
    }

    MessageTask task;
    task.message = message;
    task.sessionId = sessionId;
    task.completionCallback = completionCallback;
    task.queueTime = std::chrono::steady_clock::now();

    {
        std::lock_guard lock(queueMutex_);
        messageQueue_.push(std::move(task));
    }

    queueCondition_.notify_one();
    return Result::SUCCESS;
}

Result Engine::broadcastMessage(const ByteBuffer& message) {
    // Implementation would send to all active sessions
    METRICS_INCREMENT("broadcast_messages");
    return Result::SUCCESS;
}

Result Engine::broadcastMessage(const ByteBuffer& message, const std::function<bool(const std::string&)>& filter) {
    // Implementation would send to filtered sessions
    METRICS_INCREMENT("filtered_broadcast_messages");
    return Result::SUCCESS;
}

Result Engine::sendToSession(const std::string& sessionId, const ByteBuffer& message) {
    // Implementation would send to specific session
    METRICS_INCREMENT("direct_messages");
    return Result::SUCCESS;
}

Result Engine::closeSession(const std::string& sessionId, uint16_t code, const std::string& reason) {
    // Implementation would close specific session
    METRICS_INCREMENT("session_closures");
    return Result::SUCCESS;
}

size_t Engine::getConnectionCount() const {
    return 0; // Implementation would return actual count
}

size_t Engine::getActiveSessionCount() const {
    return 0; // Implementation would return actual count
}

void Engine::setMaxConnections(size_t max) {
    maxConnections_ = max;
}

void Engine::setMaxSessions(size_t max) {
    maxSessions_ = max;
}

size_t Engine::getMessageCount() const {
    return totalMessagesProcessed_;
}

size_t Engine::getQueuedMessageCount() const {
    std::lock_guard lock(queueMutex_);
    return messageQueue_.size();
}

void Engine::setMaxMessageSize(size_t max) {
    maxMessageSize_ = max;
}

void Engine::setMaxQueueSize(size_t max) {
    maxQueueSize_ = max;
}

void Engine::setProcessingThreads(size_t count) {
    processingThreads_ = count;
}

void Engine::setQueueTimeout(uint32_t timeoutMs) {
    queueTimeout_ = timeoutMs;
}

void Engine::enableCompression(bool enable) {
    compressionEnabled_ = enable;
}

void Engine::setOnMessageHandler(std::function<Result(const ByteBuffer&, const std::string&)> handler) {
    onMessageHandler_ = handler;
}

void Engine::setOnSessionCreatedHandler(std::function<void(const std::string&)> handler) {
    onSessionCreatedHandler_ = handler;
}

void Engine::setOnSessionClosedHandler(std::function<void(const std::string&, uint16_t, const std::string&)> handler) {
    onSessionClosedHandler_ = handler;
}

void Engine::setOnErrorHandler(std::function<void(const Error&)> handler) {
    onErrorHandler_ = handler;
}

uint64_t Engine::getTotalMessagesProcessed() const {
    return totalMessagesProcessed_;
}

uint64_t Engine::getTotalBytesProcessed() const {
    return totalBytesProcessed_;
}

double Engine::getAverageProcessingTime() const {
    if (processingCount_ == 0) return 0.0;
    return static_cast<double>(processingTimeTotal_) / processingCount_ / 1000.0; // Convert to milliseconds
}

std::unordered_map<std::string, uint64_t> Engine::getMessageTypeStatistics() const {
    std::lock_guard lock(statsMutex_);
    return messageTypeStats_;
}

Result Engine::pauseProcessing() {
    isProcessingPaused_ = true;
    return Result::SUCCESS;
}

Result Engine::resumeProcessing() {
    isProcessingPaused_ = false;
    queueCondition_.notify_all();
    return Result::SUCCESS;
}

bool Engine::isProcessingPaused() const {
    return isProcessingPaused_;
}

void Engine::setMemoryLimit(size_t limitBytes) {
    memoryLimit_ = limitBytes;
}

size_t Engine::getMemoryUsage() const {
    return 0; // Implementation would return actual memory usage
}

bool Engine::isMemoryLimitExceeded() const {
    return memoryLimit_ > 0 && getMemoryUsage() > memoryLimit_;
}

// Private methods
void Engine::initializeComponents() {
    // Initialize core components here
    LOG_DEBUG("Initializing engine components");
}

void Engine::shutdownComponents() {
    // Cleanup core components here
    LOG_DEBUG("Shutting down engine components");
}

void Engine::workerThreadFunction() {
    while (isRunning_) {
        MessageTask task;

        {
            std::unique_lock lock(queueMutex_);

            // Wait for messages or shutdown
            queueCondition_.wait(lock, [this]() {
                return !isRunning_ || (!messageQueue_.empty() && !isProcessingPaused_);
                });

            if (!isRunning_) {
                break;
            }

            if (messageQueue_.empty() || isProcessingPaused_) {
                continue;
            }

            task = std::move(messageQueue_.front());
            messageQueue_.pop();
        }

        // Process the message
        auto startTime = std::chrono::steady_clock::now();
        Result result = processMessageInternal(task.message, task.sessionId);
        auto endTime = std::chrono::steady_clock::now();

        auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        updateStatistics(task.message, processingTime.count());

        // Call completion callback if provided
        if (task.completionCallback) {
            try {
                task.completionCallback();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Completion callback failed: {}", e.what());
            }
        }

        // Cleanup expired queue items periodically
        if (totalMessagesProcessed_ % 100 == 0) {
            cleanupExpiredQueueItems();
        }
    }
}

Result Engine::processMessageInternal(const ByteBuffer& message, const std::string& sessionId) {
    if (onMessageHandler_) {
        try {
            return onMessageHandler_(message, sessionId);
        }
        catch (const std::exception& e) {
            Error error = errorFromException(e);
            handleError(error);
            return Result::ERROR;
        }
    }

    // Default message processing
    METRICS_INCREMENT("messages_processed");
    return Result::SUCCESS;
}

void Engine::handleError(const Error& error) {
    lastError_ = error;
    totalErrors_++;

    LOG_ERROR("Engine error: {}", error.message);

    if (onErrorHandler_) {
        try {
            onErrorHandler_(error);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Error handler failed: {}", e.what());
        }
    }
}

void Engine::updateStatistics(const ByteBuffer& message, uint64_t processingTimeMicros) {
    totalMessagesProcessed_++;
    totalBytesProcessed_ += message.size();
    processingTimeTotal_ += processingTimeMicros;
    processingCount_++;

    // Update message type statistics
    std::string messageType = "unknown";
    if (message.size() < 100) messageType = "small";
    else if (message.size() < 1000) messageType = "medium";
    else messageType = "large";

    std::lock_guard lock(statsMutex_);
    messageTypeStats_[messageType]++;
}

void Engine::cleanupExpiredQueueItems() {
    auto now = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(queueTimeout_);

    std::lock_guard lock(queueMutex_);

    size_t initialSize = messageQueue_.size();
    std::queue<MessageTask> tempQueue;

    while (!messageQueue_.empty()) {
        auto task = std::move(messageQueue_.front());
        messageQueue_.pop();

        if (now - task.queueTime < timeout) {
            tempQueue.push(std::move(task));
        }
    }

    messageQueue_ = std::move(tempQueue);

    if (initialSize != messageQueue_.size()) {
        LOG_DEBUG("Cleaned up {} expired queue items", initialSize - messageQueue_.size());
    }
}

bool Engine::canAcceptMessage() const {
    std::lock_guard lock(queueMutex_);
    return messageQueue_.size() < maxQueueSize_;
}

WEBSOCKET_NAMESPACE_END