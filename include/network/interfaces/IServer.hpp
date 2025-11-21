#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IServer : public IService {
    WEBSOCKET_INTERFACE(IServer)

public:
    // Server lifecycle
    virtual Result startListening() = 0;
    virtual Result stopListening() = 0;
    virtual Result pauseListening() = 0;
    virtual Result resumeListening() = 0;

    // Connection management
    virtual size_t getActiveConnectionCount() const = 0;
    virtual size_t getMaxConnections() const = 0;
    virtual void setMaxConnections(size_t max) = 0;

    // Endpoint management
    virtual Result addEndpoint(const std::string& address, uint16_t port) = 0;
    virtual Result removeEndpoint(const std::string& address, uint16_t port) = 0;
    virtual std::vector<std::pair<std::string, uint16_t>> getEndpoints() const = 0;

    // Statistics
    virtual uint64_t getTotalConnections() const = 0;
    virtual uint64_t getTotalBytesReceived() const = 0;
    virtual uint64_t getTotalBytesSent() const = 0;

    // Configuration
    virtual void setReceiveBufferSize(size_t size) = 0;
    virtual void setSendBufferSize(size_t size) = 0;
    virtual void setBacklogSize(int backlog) = 0;

    // Event callbacks
    virtual void setOnConnectionEstablished(EventCallback<IConnection> callback) = 0;
    virtual void setOnConnectionClosed(EventCallback<IConnection> callback) = 0;
    virtual void setOnError(EventCallback<Error> callback) = 0;
};

WEBSOCKET_NAMESPACE_END