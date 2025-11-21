#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "interfaces/IServer.hpp"
#include "interfaces/IEndpoint.hpp"
#include "interfaces/IConnection.hpp"
#include "ConnectionPool.hpp"
#include "SessionManager.hpp"
#include "IOThreadPool.hpp"
#include <vector>
#include <unordered_map>
#include <atomic>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class ServerBase : public IServer, public IConfigurable {
public:
    ServerBase();
    virtual ~ServerBase() override;

    // IServer implementation
    Result initialize() override;
    Result start() override;
    Result stop() override;
    Result shutdown() override;

    bool isRunning() const override;
    std::string getName() const override;
    std::string getStatus() const override;

    Result startListening() override;
    Result stopListening() override;
    Result pauseListening() override;
    Result resumeListening() override;

    size_t getActiveConnectionCount() const override;
    size_t getMaxConnections() const override;
    void setMaxConnections(size_t max) override;

    Result addEndpoint(const std::string& address, uint16_t port) override;
    Result removeEndpoint(const std::string& address, uint16_t port) override;
    std::vector<std::pair<std::string, uint16_t>> getEndpoints() const override;

    uint64_t getTotalConnections() const override;
    uint64_t getTotalBytesReceived() const override;
    uint64_t getTotalBytesSent() const override;

    void setReceiveBufferSize(size_t size) override;
    void setSendBufferSize(size_t size) override;
    void setBacklogSize(int backlog) override;

    void setOnConnectionEstablished(EventCallback<IConnection> callback) override;
    void setOnConnectionClosed(EventCallback<IConnection> callback) override;
    void setOnError(EventCallback<Error> callback) override;

    // IConfigurable implementation
    void configure(const std::unordered_map<std::string, std::any>& config) override;
    std::unordered_map<std::string, std::any> getCurrentConfig() const override;
    bool validateConfig(const std::unordered_map<std::string, std::any>& config) const override;
    void onConfigChanged(const std::string& key, const std::any& value) override;

protected:
    WEBSOCKET_DISABLE_COPY(ServerBase)

        virtual SharedPtr<IEndpoint> createEndpoint(const std::string& address, uint16_t port) = 0;
    virtual SharedPtr<IConnection> createConnection(SharedPtr<IEndpoint> endpoint) = 0;

    virtual void onConnectionAccepted(SharedPtr<IConnection> connection);
    virtual void onConnectionClosed(SharedPtr<IConnection> connection);
    virtual void onDataReceived(SharedPtr<IConnection> connection, const ByteBuffer& data);
    virtual void onError(const Error& error);

private:
    mutable std::shared_mutex endpointsMutex_;
    mutable std::shared_mutex connectionsMutex_;

    std::vector<SharedPtr<IEndpoint>> endpoints_;
    std::unordered_map<std::string, SharedPtr<IConnection>> activeConnections_;

    std::atomic<bool> isRunning_{ false };
    std::atomic<bool> isListening_{ false };
    std::atomic<size_t> maxConnections_{ 1000 };
    std::atomic<uint64_t> totalConnections_{ 0 };
    std::atomic<uint64_t> totalBytesReceived_{ 0 };
    std::atomic<uint64_t> totalBytesSent_{ 0 };

    SharedPtr<ConnectionPool> connectionPool_;
    SharedPtr<SessionManager> sessionManager_;
    SharedPtr<IOThreadPool> ioThreadPool_;

    EventCallback<IConnection> onConnectionEstablished_;
    EventCallback<IConnection> onConnectionClosed_;
    EventCallback<Error> onError_;

    size_t receiveBufferSize_{ 8192 };
    size_t sendBufferSize_{ 8192 };
    int backlogSize_{ SOMAXCONN };

    void initializeThreadPool();
    void shutdownThreadPool();
    void cleanupConnections();
    void handleAccept(SharedPtr<IEndpoint> endpoint);
};

WEBSOCKET_NAMESPACE_END