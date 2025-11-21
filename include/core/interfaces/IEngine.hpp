#pragma once

#include "../../common/Types.hpp"
#include "IService.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IEngine : public IService {
    WEBSOCKET_INTERFACE(IEngine)

public:
    // Message processing
    virtual Result processMessage(const ByteBuffer& message, const std::string& sessionId = "") = 0;
    virtual Result processMessageAsync(const ByteBuffer& message, const std::string& sessionId = "", Callback completionCallback = nullptr) = 0;

    // Broadcast operations
    virtual Result broadcastMessage(const ByteBuffer& message) = 0;
    virtual Result broadcastMessage(const ByteBuffer& message, const std::function<bool(const std::string&)>& filter) = 0;

    // Session management
    virtual Result sendToSession(const std::string& sessionId, const ByteBuffer& message) = 0;
    virtual Result closeSession(const std::string& sessionId, uint16_t code = 1000, const std::string& reason = "") = 0;

    // Connection management
    virtual size_t getConnectionCount() const = 0;
    virtual size_t getActiveSessionCount() const = 0;
    virtual void setMaxConnections(size_t max) = 0;
    virtual void setMaxSessions(size_t max) = 0;

    // Message management
    virtual size_t getMessageCount() const = 0;
    virtual size_t getQueuedMessageCount() const = 0;
    virtual void setMaxMessageSize(size_t max) = 0;
    virtual void setMaxQueueSize(size_t max) = 0;

    // Performance tuning
    virtual void setProcessingThreads(size_t count) = 0;
    virtual void setQueueTimeout(uint32_t timeoutMs) = 0;
    virtual void enableCompression(bool enable) = 0;

    // Event handlers
    virtual void setOnMessageHandler(std::function<Result(const ByteBuffer&, const std::string&)> handler) = 0;
    virtual void setOnSessionCreatedHandler(std::function<void(const std::string&)> handler) = 0;
    virtual void setOnSessionClosedHandler(std::function<void(const std::string&, uint16_t, const std::string&)> handler) = 0;
    virtual void setOnErrorHandler(std::function<void(const Error&)> handler) = 0;

    // Statistics and monitoring
    virtual uint64_t getTotalMessagesProcessed() const = 0;
    virtual uint64_t getTotalBytesProcessed() const = 0;
    virtual double getAverageProcessingTime() const = 0;
    virtual std::unordered_map<std::string, uint64_t> getMessageTypeStatistics() const = 0;

    // Engine control
    virtual Result pauseProcessing() = 0;
    virtual Result resumeProcessing() = 0;
    virtual bool isProcessingPaused() const = 0;

    // Resource management
    virtual void setMemoryLimit(size_t limitBytes) = 0;
    virtual size_t getMemoryUsage() const = 0;
    virtual bool isMemoryLimitExceeded() const = 0;
};

WEBSOCKET_NAMESPACE_END