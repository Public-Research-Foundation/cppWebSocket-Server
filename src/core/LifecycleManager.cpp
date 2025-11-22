#include "core/LifecycleManager.hpp"
#include "utils/Logger.hpp"

WEBSOCKET_NAMESPACE_BEGIN

LifecycleManager& LifecycleManager::getInstance() {
    static LifecycleManager instance;
    return instance;
}

LifecycleManager::LifecycleManager() {
    LOG_DEBUG("LifecycleManager created");
}

Result LifecycleManager::initializeAll() {
    std::shared_lock initLock(initMutex_);

    if (systemInitialized_) {
        LOG_WARN("System already initialized");
        return Result::SUCCESS;
    }

    LOG_INFO("Starting system initialization...");

    // Sort by priority for proper initialization order
    sortInitializablesByPriority();

    // Pre-initialization phase
    Result preInitResult = preInitializeAll();
    if (preInitResult != Result::SUCCESS) {
        LOG_ERROR("Pre-initialization failed");
        return preInitResult;
    }

    // Main initialization phase
    Result initResult = initializeAll();
    if (initResult != Result::SUCCESS) {
        LOG_ERROR("Initialization failed");
        return initResult;
    }

    // Post-initialization phase
    Result postInitResult = postInitializeAll();
    if (postInitResult != Result::SUCCESS) {
        LOG_ERROR("Post-initialization failed");
        return postInitResult;
    }

    systemInitialized_ = true;
    LOG_INFO("System initialization completed successfully");

    return Result::SUCCESS;
}

Result LifecycleManager::preInitializeAll() {
    std::shared_lock initLock(initMutex_);

    LOG_DEBUG("Starting pre-initialization phase");

    for (const auto& initializable : initializables_) {
        if (!checkInitializationDependencies(initializable)) {
            std::string errorMsg = "Dependencies not satisfied for: " + initializable->getComponentName();
            lastInitializationError_ = Error(-1, errorMsg);
            initializationErrors_.push_back(lastInitializationError_);
            LOG_ERROR(errorMsg);
            return Result::ERROR;
        }

        Result result = initializable->preInitialize();
        if (result != Result::SUCCESS) {
            std::string errorMsg = "Pre-initialization failed for: " + initializable->getComponentName();
            lastInitializationError_ = Error(-2, errorMsg);
            initializationErrors_.push_back(lastInitializationError_);
            LOG_ERROR(errorMsg);
            return result;
        }
    }

    LOG_DEBUG("Pre-initialization phase completed");
    return Result::SUCCESS;
}

Result LifecycleManager::postInitializeAll() {
    std::shared_lock initLock(initMutex_);

    LOG_DEBUG("Starting post-initialization phase");

    for (const auto& initializable : initializables_) {
        Result result = initializable->postInitialize();
        if (result != Result::SUCCESS) {
            std::string errorMsg = "Post-initialization failed for: " + initializable->getComponentName();
            lastInitializationError_ = Error(-3, errorMsg);
            initializationErrors_.push_back(lastInitializationError_);
            LOG_ERROR(errorMsg);
            return result;
        }
    }

    LOG_DEBUG("Post-initialization phase completed");
    return Result::SUCCESS;
}

Result LifecycleManager::shutdownAll() {
    return gracefulShutdownAll();
}

Result LifecycleManager::gracefulShutdownAll() {
    std::unique_lock shutdownLock(shutdownMutex_);

    if (shutdownInProgress_) {
        LOG_WARN("Shutdown already in progress");
        return Result::SUCCESS;
    }

    shutdownInProgress_ = true;
    LOG_INFO("Starting graceful shutdown...");

    // Sort shutdown handlers by priority (reverse of initialization)
    sortShutdownHandlersByPriority();

    size_t successCount = 0;
    for (const auto& handler : shutdownHandlers_) {
        if (!handler->canShutdown()) {
            LOG_WARN("Handler cannot shutdown: {}", handler->getComponentName());
            continue;
        }

        Result result = shutdownComponent(handler);
        if (result == Result::SUCCESS) {
            successCount++;
        }
        else {
            std::string errorMsg = "Shutdown failed for: " + handler->getComponentName();
            lastShutdownError_ = Error(-4, errorMsg);
            shutdownErrors_.push_back(lastShutdownError_);
            LOG_ERROR(errorMsg);
        }
    }

    systemInitialized_ = false;
    shutdownInProgress_ = false;

    LOG_INFO("Graceful shutdown completed: {}/{} handlers succeeded",
        successCount, shutdownHandlers_.size());

    return successCount == shutdownHandlers_.size() ? Result::SUCCESS : Result::ERROR;
}

Result LifecycleManager::emergencyShutdownAll() {
    std::unique_lock shutdownLock(shutdownMutex_);

    emergencyShutdown_ = true;
    LOG_ERROR("Starting emergency shutdown!");

    // Emergency shutdown - don't check dependencies or wait for graceful shutdown
    for (const auto& handler : shutdownHandlers_) {
        if (handler->supportsEmergencyShutdown()) {
            handler->onEmergencyShutdown();
        }
        else {
            handler->onShutdown();
        }
    }

    systemInitialized_ = false;
    shutdownInProgress_ = false;
    emergencyShutdown_ = false;

    LOG_ERROR("Emergency shutdown completed");
    return Result::SUCCESS;
}

void LifecycleManager::registerInitializable(const SharedPtr<IInitializable>& initializable) {
    std::unique_lock lock(initMutex_);
    initializables_.push_back(initializable);
}

void LifecycleManager::registerShutdownHandler(const SharedPtr<IShutdownHandler>& handler) {
    std::unique_lock lock(shutdownMutex_);
    shutdownHandlers_.push_back(handler);
}

void LifecycleManager::unregisterInitializable(const SharedPtr<IInitializable>& initializable) {
    std::unique_lock lock(initMutex_);
    auto it = std::find(initializables_.begin(), initializables_.end(), initializable);
    if (it != initializables_.end()) {
        initializables_.erase(it);
    }
}

void LifecycleManager::unregisterShutdownHandler(const SharedPtr<IShutdownHandler>& handler) {
    std::unique_lock lock(shutdownMutex_);
    auto it = std::find(shutdownHandlers_.begin(), shutdownHandlers_.end(), handler);
    if (it != shutdownHandlers_.end()) {
        shutdownHandlers_.erase(it);
    }
}

bool LifecycleManager::isSystemInitialized() const {
    return systemInitialized_;
}

bool LifecycleManager::isShutdownInProgress() const {
    return shutdownInProgress_;
}

bool LifecycleManager::isEmergencyShutdownInProgress() const {
    return emergencyShutdown_;
}

Result LifecycleManager::resolveInitializationDependencies() {
    std::shared_lock lock(initMutex_);

    for (const auto& initializable : initializables_) {
        if (!checkInitializationDependencies(initializable)) {
            return Result::ERROR;
        }
    }

    return Result::SUCCESS;
}

Result LifecycleManager::resolveShutdownDependencies() {
    std::shared_lock lock(shutdownMutex_);

    for (const auto& handler : shutdownHandlers_) {
        if (!checkShutdownDependencies(handler)) {
            return Result::ERROR;
        }
    }

    return Result::SUCCESS;
}

bool LifecycleManager::areInitializationDependenciesSatisfied() const {
    std::shared_lock lock(initMutex_);

    for (const auto& initializable : initializables_) {
        if (!checkInitializationDependencies(initializable)) {
            return false;
        }
    }

    return true;
}

bool LifecycleManager::areShutdownDependenciesSatisfied() const {
    std::shared_lock lock(shutdownMutex_);

    for (const auto& handler : shutdownHandlers_) {
        if (!checkShutdownDependencies(handler)) {
            return false;
        }
    }

    return true;
}

double LifecycleManager::getInitializationProgress() const {
    std::shared_lock lock(initMutex_);

    if (initializables_.empty()) return 1.0;

    size_t completed = 0;
    for (const auto& initializable : initializables_) {
        if (initializable->isInitialized()) {
            completed++;
        }
    }

    return static_cast<double>(completed) / initializables_.size();
}

double LifecycleManager::getShutdownProgress() const {
    std::shared_lock lock(shutdownMutex_);

    if (shutdownHandlers_.empty()) return 1.0;

    size_t completed = 0;
    for (const auto& handler : shutdownHandlers_) {
        if (handler->isShutdownComplete()) {
            completed++;
        }
    }

    return static_cast<double>(completed) / shutdownHandlers_.size();
}

std::string LifecycleManager::getInitializationStatus() const {
    if (!systemInitialized_) {
        return "Not initialized";
    }

    double progress = getInitializationProgress();
    if (progress >= 1.0) {
        return "Fully initialized";
    }
    else {
        return "Partially initialized: " + std::to_string(static_cast<int>(progress * 100)) + "%";
    }
}

std::string LifecycleManager::getShutdownStatus() const {
    if (shutdownInProgress_) {
        double progress = getShutdownProgress();
        return "Shutdown in progress: " + std::to_string(static_cast<int>(progress * 100)) + "%";
    }
    else if (emergencyShutdown_) {
        return "Emergency shutdown";
    }
    else {
        return "Not shutting down";
    }
}

void LifecycleManager::setInitializationTimeout(uint32_t timeoutMs) {
    initializationTimeout_ = timeoutMs;
}

void LifecycleManager::setShutdownTimeout(uint32_t timeoutMs) {
    shutdownTimeout_ = timeoutMs;
}

void LifecycleManager::setEmergencyShutdownTimeout(uint32_t timeoutMs) {
    emergencyShutdownTimeout_ = timeoutMs;
}

Error LifecycleManager::getLastInitializationError() const {
    return lastInitializationError_;
}

Error LifecycleManager::getLastShutdownError() const {
    return lastShutdownError_;
}

std::vector<Error> LifecycleManager::getAllInitializationErrors() const {
    return initializationErrors_;
}

std::vector<Error> LifecycleManager::getAllShutdownErrors() const {
    return shutdownErrors_;
}

void LifecycleManager::clearErrors() {
    initializationErrors_.clear();
    shutdownErrors_.clear();
    lastInitializationError_ = Error();
    lastShutdownError_ = Error();
}

size_t LifecycleManager::getInitializableCount() const {
    std::shared_lock lock(initMutex_);
    return initializables_.size();
}

size_t LifecycleManager::getShutdownHandlerCount() const {
    std::shared_lock lock(shutdownMutex_);
    return shutdownHandlers_.size();
}

size_t LifecycleManager::getInitializedCount() const {
    std::shared_lock lock(initMutex_);

    size_t count = 0;
    for (const auto& initializable : initializables_) {
        if (initializable->isInitialized()) {
            count++;
        }
    }

    return count;
}

size_t LifecycleManager::getShutdownCount() const {
    std::shared_lock lock(shutdownMutex_);

    size_t count = 0;
    for (const auto& handler : shutdownHandlers_) {
        if (handler->isShutdownComplete()) {
            count++;
        }
    }

    return count;
}

// Private methods
void LifecycleManager::sortInitializablesByPriority() {
    std::sort(initializables_.begin(), initializables_.end(),
        [](const SharedPtr<IInitializable>& a, const SharedPtr<IInitializable>& b) {
            return a->getInitializationPriority() < b->getInitializationPriority();
        });
}

void LifecycleManager::sortShutdownHandlersByPriority() {
    std::sort(shutdownHandlers_.begin(), shutdownHandlers_.end(),
        [](const SharedPtr<IShutdownHandler>& a, const SharedPtr<IShutdownHandler>& b) {
            return a->getShutdownPriority() > b->getShutdownPriority(); // Reverse order for shutdown
        });
}

Result LifecycleManager::initializeComponent(const SharedPtr<IInitializable>& initializable) {
    try {
        return initializable->initialize();
    }
    catch (const std::exception& e) {
        std::string errorMsg = "Exception initializing " + initializable->getComponentName() + ": " + e.what();
        LOG_ERROR(errorMsg);
        return Result::ERROR;
    }
}

Result LifecycleManager::shutdownComponent(const SharedPtr<IShutdownHandler>& handler) {
    try {
        handler->onShutdown();
        return Result::SUCCESS;
    }
    catch (const std::exception& e) {
        std::string errorMsg = "Exception shutting down " + handler->getComponentName() + ": " + e.what();
        LOG_ERROR(errorMsg);
        return Result::ERROR;
    }
}

bool LifecycleManager::checkInitializationDependencies(const SharedPtr<IInitializable>& initializable) const {
    auto dependencies = initializable->getDependencies();
    for (const auto& dep : dependencies) {
        bool found = false;
        for (const auto& other : initializables_) {
            if (other->getComponentName() == dep && other->isInitialized()) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

bool LifecycleManager::checkShutdownDependencies(const SharedPtr<IShutdownHandler>& handler) const {
    auto dependencies = handler->getShutdownDependencies();
    for (const auto& dep : dependencies) {
        bool found = false;
        for (const auto& other : shutdownHandlers_) {
            if (other->getComponentName() == dep && other->isShutdownComplete()) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

void LifecycleManager::updateInitializationProgress() {
    // Implementation would update progress metrics
}

void LifecycleManager::updateShutdownProgress() {
    // Implementation would update progress metrics
}

WEBSOCKET_NAMESPACE_END