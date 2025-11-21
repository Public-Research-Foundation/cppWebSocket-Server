#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"
#include "IMessage.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IProtocol : public IComponent {
    WEBSOCKET_INTERFACE(IProtocol)

public:
    // Protocol identification
    virtual std::string getProtocolName() const = 0;
    virtual std::string getProtocolVersion() const = 0;
    virtual std::vector<std::string> getSupportedVersions() const = 0;

    // Handshake management
    virtual Result processHandshakeRequest(const ByteBuffer& request) = 0;
    virtual ByteBuffer createHandshakeResponse() = 0;
    virtual bool validateHandshake(const ByteBuffer& request) const = 0;

    // Message processing
    virtual SharedPtr<IMessage> createMessage(FrameOpcodes::Opcode opcode, const ByteBuffer& payload = ByteBuffer()) = 0;
    virtual Result processIncomingData(const ByteBuffer& data, std::vector<SharedPtr<IMessage>>& messages) = 0;
    virtual ByteBuffer prepareOutgoingMessage(SharedPtr<IMessage> message) = 0;

    // Frame processing
    virtual Result processFrame(const ByteBuffer& frameData, SharedPtr<IMessage>& message) = 0;
    virtual ByteBuffer createFrame(SharedPtr<IMessage> message) = 0;
    virtual bool validateFrame(const ByteBuffer& frameData) const = 0;

    // Control frames
    virtual ByteBuffer createPingFrame(const ByteBuffer& data = ByteBuffer()) = 0;
    virtual ByteBuffer createPongFrame(const ByteBuffer& data = ByteBuffer()) = 0;
    virtual ByteBuffer createCloseFrame(uint16_t statusCode = 1000, const std::string& reason = "") = 0;

    // Protocol extensions
    virtual bool supportsExtension(const std::string& extension) const = 0;
    virtual Result enableExtension(const std::string& extension) = 0;
    virtual Result disableExtension(const std::string& extension) = 0;
    virtual std::vector<std::string> getActiveExtensions() const = 0;

    // Compression
    virtual bool supportsCompression() const = 0;
    virtual Result enableCompression() = 0;
    virtual Result disableCompression() = 0;
    virtual bool isCompressionEnabled() const = 0;

    // Statistics
    virtual uint64_t getMessagesProcessed() const = 0;
    virtual uint64_t getFramesProcessed() const = 0;
    virtual uint64_t getBytesProcessed() const = 0;
    virtual uint64_t getErrorsCount() const = 0;

    // Configuration
    virtual void setMaxFrameSize(size_t maxSize) = 0;
    virtual void setMaxMessageSize(size_t maxSize) = 0;
    virtual void setMaskOutgoingFrames(bool mask) = 0;
};

WEBSOCKET_NAMESPACE_END