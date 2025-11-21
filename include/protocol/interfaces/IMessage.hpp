#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"
#include "../../constants/FrameOpcodes.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IMessage {
    WEBSOCKET_INTERFACE(IMessage)

public:
    // Message type and properties
    virtual FrameOpcodes::Opcode getOpcode() const = 0;
    virtual bool isControlFrame() const = 0;
    virtual bool isDataFrame() const = 0;
    virtual bool isText() const = 0;
    virtual bool isBinary() const = 0;
    virtual bool isFinal() const = 0;

    // Payload access
    virtual const ByteBuffer& getPayload() const = 0;
    virtual size_t getPayloadSize() const = 0;
    virtual std::string getPayloadAsString() const = 0;

    // Message metadata
    virtual uint64_t getMessageId() const = 0;
    virtual std::chrono::steady_clock::time_point getTimestamp() const = 0;
    virtual bool isCompressed() const = 0;
    virtual bool isMasked() const = 0;

    // Message manipulation
    virtual void setPayload(const ByteBuffer& payload) = 0;
    virtual void setPayload(const std::string& payload) = 0;
    virtual void setOpcode(FrameOpcodes::Opcode opcode) = 0;
    virtual void setFinal(bool isFinal) = 0;

    // Validation
    virtual bool isValid() const = 0;
    virtual std::string getValidationError() const = 0;

    // Serialization
    virtual ByteBuffer serialize() const = 0;
    virtual Result deserialize(const ByteBuffer& data) = 0;

    // Fragmentation support
    virtual bool isFragmented() const = 0;
    virtual size_t getFragmentCount() const = 0;
    virtual SharedPtr<IMessage> getFragment(size_t index) const = 0;
    virtual void addFragment(SharedPtr<IMessage> fragment) = 0;
};

WEBSOCKET_NAMESPACE_END