#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "interfaces/IEngine.hpp"
#include "interfaces/IConfigurable.hpp"
#include "ServiceLocator.hpp"
#include "ComponentManager.hpp"
#include <atomic>
#include <memory>
#include <unordered_map>
#include <queue>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class Engine : public IEngine, public IConfigurable {
public:
    Engine();
    ~Engine() override;

    // IEngine implementation
    Result initialize() override;
    Result start() override;
    Result stop() override;
    Result shutdown() override;

    bool isRunning() const override;
    std::string getName() const override;
    std::string getStatus() const override;

    Result processMessage(const ByteBuffer& message, const std::string& sessionId = "") override;
    Result processMessageAsync(const ByteBuffer& message, const std::string& sessionId = "", Callback completionCallback = nullptr) override;

    Result broadcastMessage(const ByteBuffer& message) override;
    Result broadcastMessage(const ByteBuffer& message, const std::function<bool(const std::string&)>& filter) override;

    Result sendToSession(const std::string& sessionId, const ByteBuffer& message) override;
    Result closeSession(const std::string& sessionId, uint16_t code = 1000, const std::string& reason = "") override;

    size_t getConnectionCount() const override;
    size_t getActiveSessionCount() const override;
    void setMaxConnections(size_t max) override;
    void setMaxSessions(size_t max) override;

    size_t getMessageCount() const override;
    size_t getQueuedMessageCount() const override;
    void setMaxMessageSize(size_t max) override;
    void setMaxQueueSize(size_t max) override;

    void setProcessingThreads(size_t count) override;
    void setQueueTimeout(uint32_t timeoutMs) override;
    void enableCompression(bool enable) override;

    void setOnMessageHandler(std::function<Result(const ByteBuffer&, const std::string&)> handler) override;
    void setOnSessionCreatedHandler(std::function<void(const std::string&)> handler) override;
    void setOnSessionClosedHandler(std::function<void(const std::string&, uint16_t, const std::string&)> handler) override;
    void setOnErrorHandler(std::function<void(const Error&)> handler) override;

    uint64_t getTotalMessagesProcessed() const override;
    uint64_t getTotalBytesProcessed() const override;
    double getAverageProcessingTime() const override;
    std::unordered_map<std::string, uint64_t> getMessageTypeStatistics() const override;

    Result pauseProcessing() override;
    Result resumeProcessing() override;
    bool isProcessingPaused() const override;

    void setMemoryLimit(size_t limitBytes) override;
    size_t getMemoryUsage() const override;
    bool isMemoryLimitExceeded() const override;

    // IService implementation
    ServiceState getState() const override;
    std::string getStatusMessage() const override;
    std::string getVersion() const override;
    std::string getDescription() const override;
    std::vector<std::string> getDependencies() const override;
    bool hasDependency(const std::string& serviceName) const override;
    Error getLastError() const override;
    void clearError() override;
    bool hasError() const override;
    void setConfiguration(const std::unordered_map<std::string, std::any>& config) override;
    std::unordered_map<std::string, std::any> getConfiguration() const override;
    std::chrono::steady_clock::time_point getStartTime() const override;
    std::chrono::duration<double> getUptime() const override;
    uint64_t getRequestCount() const override;
    uint64_t getErrorCount() const override;

    // IConfigurable implementation
    void configure(const std::unordered_map<std::string, std::any>& config) override;
    std::unordered_map<std::string, std::any> getCurrentConfig() const override;
    bool validateConfig(const std::unordered_map<std::string, std::any>& config) const override;
    void onConfigChanged(const std::string& key, const std::any& value) override;

private:
    WEBSOCKET_DISABLE_COPY(Engine)

        struct MessageTask {
        ByteBuffer message;
        std::string sessionId;
        Callback completionCallback;
        std::chrono::steady_clock::time_point queueTime;
    };

    mutable std::shared_mutex stateMutex_;
    mutable std::mutex queueMutex_;
    mutable std::mutex statsMutex_;

    std::atomic<ServiceState> state_{ ServiceState::UNINITIALIZED };
    std::atomic<bool> isRunning_{ false };
    std::atomic<bool> isProcessingPaused_{ false };

    std::atomic<size_t> maxConnections_{ 1000 };
    std::atomic<size_t> maxSessions_{ 10000 };
    std::atomic<size_t> maxMessageSize_{ 1048576 }; // 1MB
    std::atomic<size_t> maxQueueSize_{ 10000 };
    std::atomic<size_t> processingThreads_{ 4 };
    std::atomic<uint32_t> queueTimeout_{ 30000 }; // 30 seconds
    std::atomic<bool> compressionEnabled_{ false };
    std::atomic<size_t> memoryLimit_{ 0 }; // 0 = no limit

    std::queue<MessageTask> messageQueue_;
    std::condition_variable queueCondition_;

    SharedPtr<ServiceLocator> serviceLocator_;
    SharedPtr<ComponentManager> componentManager_;

    // Event handlers
    std::function<Result(const ByteBuffer&, const std::string&)> onMessageHandler_;
    std::function<void(const std::string&)> onSessionCreatedHandler_;
    std::function<void(const std::string&, uint16_t, const std::string&)> onSessionClosedHandler_;
    std::function<void(const Error&)> onErrorHandler_;

    // Statistics
    std::atomic<uint64_t> totalMessagesProcessed_{ 0 };
    std::atomic<uint64_t> totalBytesProcessed_{ 0 };
    std::atomic<uint64_t> totalErrors_{ 0 };
    std::atomic<uint64_t> queueDrops_{ 0 };
    std::atomic<uint64_t> processingTimeTotal_{ 0 }; // microseconds
    std::atomic<uint64_t> processingCount_{ 0 };

    std::unordered_map<std::string, uint64_t> messageTypeStats_;
    std::chrono::steady_clock::time_point startTime_;
    Error lastError_;

    std::vector<std::thread> workerThreads_;

    void initializeComponents();
    void shutdownComponents();
    void workerThreadFunction();
    Result processMessageInternal(const ByteBuffer& message, const std::string& sessionId);
    void handleError(const Error& error);
    void updateStatistics(const ByteBuffer& message, uint64_t processingTimeMicros);
    void cleanupExpiredQueueItems();
    bool canAcceptMessage() const;
};

WEBSOCKET_NAMESPACE_END