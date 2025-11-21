#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "interfaces/IConnection.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

struct SessionInfo {
    std::string sessionId;
    SharedPtr<IConnection> connection;
    std::string remoteAddress;
    uint16_t remotePort;
    std::chrono::steady_clock::time_point createdTime;
    std::chrono::steady_clock::time_point lastActivity;
    uint64_t messagesSent{ 0 };
    uint64_t messagesReceived{ 0 };
    uint64_t bytesSent{ 0 };
    uint64_t bytesReceived{ 0 };
    std::unordered_map<std::string, std::any> attributes;
};

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    WEBSOCKET_DISABLE_COPY(SessionManager)

        // Session management
        std::string createSession(SharedPtr<IConnection> connection);
    bool destroySession(const std::string& sessionId);
    bool destroySession(SharedPtr<IConnection> connection);

    // Session access
    SharedPtr<SessionInfo> getSession(const std::string& sessionId) const;
    SharedPtr<SessionInfo> getSession(SharedPtr<IConnection> connection) const;
    std::vector<SharedPtr<SessionInfo>> getAllSessions() const;

    // Session validation
    bool isValidSession(const std::string& sessionId) const;
    bool isSessionActive(const std::string& sessionId) const;
    void updateSessionActivity(const std::string& sessionId);

    // Session attributes
    void setSessionAttribute(const std::string& sessionId, const std::string& key, const std::any& value);
    std::any getSessionAttribute(const std::string& sessionId, const std::string& key) const;
    bool hasSessionAttribute(const std::string& sessionId, const std::string& key) const;
    void removeSessionAttribute(const std::string& sessionId, const std::string& key);

    // Statistics and monitoring
    size_t getActiveSessionCount() const;
    size_t getTotalSessionsCreated() const;
    size_t getTotalSessionsDestroyed() const;

    void broadcastMessage(const ByteBuffer& message, const std::function<bool(SharedPtr<SessionInfo>)>& filter = nullptr);

    // Session cleanup
    void cleanupInactiveSessions(uint32_t maxInactiveTimeMs = 300000); // 5 minutes default
    void cleanupAllSessions();

    // Configuration
    void setMaxSessions(size_t maxSessions);
    void setSessionTimeout(uint32_t timeoutMs);
    void enableSessionPing(bool enable);

    struct Stats {
        size_t activeSessions;
        size_t totalCreated;
        size_t totalDestroyed;
        uint64_t totalMessages;
        uint64_t totalBytes;
    };

    Stats getStats() const;

private:
    mutable std::shared_mutex mutex_;

    std::unordered_map<std::string, SharedPtr<SessionInfo>> sessions_;
    std::unordered_map<SharedPtr<IConnection>, std::string> connectionToSessionMap_;

    std::atomic<size_t> maxSessions_{ 10000 };
    std::atomic<uint32_t> sessionTimeout_{ 1800000 }; // 30 minutes default
    std::atomic<bool> enablePing_{ true };

    std::atomic<size_t> totalSessionsCreated_{ 0 };
    std::atomic<size_t> totalSessionsDestroyed_{ 0 };

    std::string generateSessionId() const;
    void onConnectionClosed(SharedPtr<IConnection> connection);
    void performSessionHealthChecks();
};

WEBSOCKET_NAMESPACE_END