#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "../constants/FrameOpcodes.hpp"
#include "../constants/Limits.hpp"
#include "../constants/StatusCodes.hpp"
#include "interfaces/IMessage.hpp"
#include <vector>
#include <memory>

WEBSOCKET_NAMESPACE_BEGIN

class FrameBuilder {
public:
    FrameBuilder();
    ~FrameBuilder() = default;

    WEBSOCKET_DISABLE_COPY(FrameBuilder)

        // Frame construction
        ByteBuffer buildFrame(SharedPtr<IMessage> message);
    ByteBuffer buildFrame(FrameOpcodes::Opcode opcode, const ByteBuffer& payload, bool isFinal = true);
    ByteBuffer buildTextFrame(const std::string& text, bool isFinal = true);
    ByteBuffer buildBinaryFrame(const ByteBuffer& data, bool isFinal = true);
    ByteBuffer buildPingFrame(const ByteBuffer& data = ByteBuffer());
    ByteBuffer buildPongFrame(const ByteBuffer& data = ByteBuffer());
    ByteBuffer buildCloseFrame(uint16_t statusCode = StatusCodes::NORMAL_CLOSURE, const std::string& reason = "");

    // Fragmentation support
    std::vector<ByteBuffer> buildFragmentedMessage(SharedPtr<IMessage> message, size_t fragmentSize = Limits::DEFAULT_MAX_FRAME_SIZE);
    std::vector<ByteBuffer> buildFragmentedMessage(const ByteBuffer& payload, FrameOpcodes::Opcode opcode, size_t fragmentSize = Limits::DEFAULT_MAX_FRAME_SIZE);

    // Frame parsing
    SharedPtr<IMessage> parseFrame(const ByteBuffer& frameData);
    Result parseFrames(const ByteBuffer& data, std::vector<SharedPtr<IMessage>>& messages);
    size_t getFrameSize(const ByteBuffer& data) const;

    // Frame validation
    bool validateFrame(const ByteBuffer& frameData) const;
    bool validateFrameHeader(const ByteBuffer& frameData) const;
    std::string getFrameValidationError(const ByteBuffer& frameData) const;

    // Masking operations
    void setMaskOutgoingFrames(bool mask);
    void setMaskKey(const ByteBuffer& maskKey);
    ByteBuffer generateMaskKey();
    void applyMask(ByteBuffer& data, const ByteBuffer& maskKey);

    // Configuration
    void setMaxFrameSize(size_t maxSize);
    void setCompressionEnabled(bool enabled);
    void setRSVBits(uint8_t rsv1, uint8_t rsv2, uint8_t rsv3);

    // Performance optimization
    void reserveBuffer(size_t size);
    void clearBuffer();

    // Statistics
    struct BuilderStats {
        uint64_t framesBuilt;
        uint64_t framesParsed;
        uint64_t totalBytesProcessed;
        uint64_t compressionBytesSaved;
        uint64_t validationErrors;
    };

    BuilderStats getStats() const;
    void resetStats();

private:
    ByteBuffer buffer_;
    ByteBuffer maskKey_;

    std::atomic<bool> maskOutgoingFrames_{ false };
    std::atomic<size_t> maxFrameSize_{ Limits::DEFAULT_MAX_FRAME_SIZE };
    std::atomic<bool> compressionEnabled_{ false };

    std::atomic<uint8_t> rsv1_{ 0 };
    std::atomic<uint8_t> rsv2_{ 0 };
    std::atomic<uint8_t> rsv3_{ 0 };

    std::atomic<uint64_t> framesBuilt_{ 0 };
    std::atomic<uint64_t> framesParsed_{ 0 };
    std::atomic<uint64_t> totalBytesProcessed_{ 0 };
    std::atomic<uint64_t> compressionBytesSaved_{ 0 };
    std::atomic<uint64_t> validationErrors_{ 0 };

    // Frame construction helpers
    void buildFrameHeader(uint8_t opcode, uint64_t payloadLength, bool isFinal, ByteBuffer& output);
    void buildExtendedLength(uint64_t payloadLength, ByteBuffer& output);
    void buildMaskingKey(ByteBuffer& output);

    // Frame parsing helpers
    Result parseFrameHeader(const ByteBuffer& data, size_t& offset,
        uint8_t& opcode, uint64_t& payloadLength,
        bool& isFinal, bool& isMasked, ByteBuffer& maskingKey);
    Result parseFramePayload(const ByteBuffer& data, size_t& offset,
        uint64_t payloadLength, bool isMasked,
        const ByteBuffer& maskingKey, ByteBuffer& payload);

    // Validation helpers
    bool validateOpcode(uint8_t opcode) const;
    bool validatePayloadLength(uint64_t length) const;
    bool validateControlFrame(uint8_t opcode, uint64_t payloadLength) const;
    bool validateFragmentation(uint8_t opcode, bool isFinal) const;

    // Utility functions
    uint8_t buildFirstByte(FrameOpcodes::Opcode opcode, bool isFinal) const;
    void updateStats(size_t bytesProcessed);
};

WEBSOCKET_NAMESPACE_END