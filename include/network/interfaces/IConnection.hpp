#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IConnection : public IComponent {
    WEBSOCKET_INTERFACE(IConnection)

public:
    // Connection information
    virtual std::string getRemoteAddress() const = 0;
    virtual uint16_t getRemotePort() const = 0;
    virtual std::string getLocalAddress() const = 0;
    virtual uint16_t getLocalPort() const = 0;
    virtual std::string getConnectionId() const = 0;

    // Connection state
    virtual bool isConnected() const = 0;
    virtual bool isEncrypted() const = 0;
    virtual ConnectionState getState() const = 0;

    // Data transfer
    virtual Result send(const ByteBuffer& data) = 0;
    virtual Result send(const void* data, size_t length) = 0;
    virtual Result receive(ByteBuffer& buffer) = 0;
    virtual size_t getBytesAvailable() const = 0;

    // Async operations
    virtual Result asyncSend(const ByteBuffer& data, Callback completionCallback = nullptr) = 0;
    virtual Result asyncReceive(Callback dataReadyCallback = nullptr) = 0;

    // Connection control
    virtual Result close() = 0;
    virtual Result shutdown() = 0;
    virtual Result setTimeout(uint32_t milliseconds) = 0;

    // Statistics
    virtual uint64_t getBytesSent() const = 0;
    virtual uint64_t getBytesReceived() const = 0;
    virtual std::chrono::steady_clock::time_point getConnectionTime() const = 0;

    // Security
    virtual Result enableEncryption() = 0;
    virtual Result verifyCertificate() const = 0;

    // Quality of Service
    virtual Result setPriority(int priority) = 0;
    virtual Result setNoDelay(bool noDelay) = 0;
    virtual Result setKeepAlive(bool enable, uint32_t interval = 0) = 0;
};

WEBSOCKET_NAMESPACE_END