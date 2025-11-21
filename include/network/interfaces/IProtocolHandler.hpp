#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IProtocolHandler {
    WEBSOCKET_INTERFACE(IProtocolHandler)

public:
    // Protocol identification
    virtual std::string getProtocolName() const = 0;
    virtual std::string getProtocolVersion() const = 0;
    virtual bool supportsProtocol(const std::string& protocol) const = 0;

    // Message processing
    virtual Result processHandshake(SharedPtr<IConnection> connection, const ByteBuffer& data) = 0;
    virtual Result processMessage(SharedPtr<IConnection> connection, const ByteBuffer& data) = 0;
    virtual Result processFrame(SharedPtr<IConnection> connection, const ByteBuffer& data) = 0;

    // Message creation
    virtual ByteBuffer createHandshakeResponse(const std::string& key) = 0;
    virtual ByteBuffer createMessage(const ByteBuffer& payload, bool isBinary = true) = 0;
    virtual ByteBuffer createFrame(const ByteBuffer& payload, uint8_t opcode, bool isFinal = true) = 0;

    // Protocol control
    virtual Result handlePing(SharedPtr<IConnection> connection, const ByteBuffer& data = ByteBuffer()) = 0;
    virtual Result handlePong(SharedPtr<IConnection> connection, const ByteBuffer& data = ByteBuffer()) = 0;
    virtual Result handleClose(SharedPtr<IConnection> connection, uint16_t code = 1000, const std::string& reason = "") = 0;

    // Protocol validation
    virtual bool validateHandshake(const ByteBuffer& data) const = 0;
    virtual bool validateMessage(const ByteBuffer& data) const = 0;
    virtual bool validateFrame(const ByteBuffer& data) const = 0;

    // Protocol extensions
    virtual Result enableCompression() = 0;
    virtual Result addExtension(const std::string& extension) = 0;
    virtual bool hasExtension(const std::string& extension) const = 0;

    // Statistics
    virtual uint64_t getMessagesProcessed() const = 0;
    virtual uint64_t getFramesProcessed() const = 0;
    virtual uint64_t getErrorsEncountered() const = 0;
};

WEBSOCKET_NAMESPACE_END