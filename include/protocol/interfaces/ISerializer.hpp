#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"
#include "IMessage.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class ISerializer {
    WEBSOCKET_INTERFACE(ISerializer)

public:
    // Message serialization
    virtual ByteBuffer serializeMessage(SharedPtr<IMessage> message) = 0;
    virtual SharedPtr<IMessage> deserializeMessage(const ByteBuffer& data) = 0;

    // Frame serialization
    virtual ByteBuffer serializeFrame(SharedPtr<IMessage> message) = 0;
    virtual SharedPtr<IMessage> deserializeFrame(const ByteBuffer& frameData) = 0;

    // Batch operations
    virtual Result serializeMessages(const std::vector<SharedPtr<IMessage>>& messages, ByteBuffer& output) = 0;
    virtual Result deserializeMessages(const ByteBuffer& data, std::vector<SharedPtr<IMessage>>& messages) = 0;

    // Compression
    virtual Result compressMessage(SharedPtr<IMessage> message) = 0;
    virtual Result decompressMessage(SharedPtr<IMessage> message) = 0;
    virtual bool isCompressionSupported() const = 0;

    // Validation
    virtual bool validateSerializedData(const ByteBuffer& data) const = 0;
    virtual std::string getSerializationError() const = 0;

    // Performance optimization
    virtual void setBufferSize(size_t size) = 0;
    virtual void enableZeroCopy(bool enable) = 0;
    virtual void setCompressionLevel(int level) = 0;

    // Statistics
    virtual uint64_t getSerializedBytes() const = 0;
    virtual uint64_t getDeserializedBytes() const = 0;
    virtual double getCompressionRatio() const = 0;
};

WEBSOCKET_NAMESPACE_END