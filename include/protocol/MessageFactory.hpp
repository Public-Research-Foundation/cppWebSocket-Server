#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "../constants/FrameOpcodes.hpp"
#include "../constants/StatusCodes.hpp"
#include "interfaces/IMessage.hpp"
#include "interfaces/IProtocol.hpp"
#include <memory>
#include <unordered_map>

WEBSOCKET_NAMESPACE_BEGIN

class MessageFactory {
public:
    static MessageFactory& getInstance();

    // Message creation
    SharedPtr<IMessage> createMessage(FrameOpcodes::Opcode opcode, const ByteBuffer& payload = ByteBuffer());
    SharedPtr<IMessage> createTextMessage(const std::string& text);
    SharedPtr<IMessage> createBinaryMessage(const ByteBuffer& data);
    SharedPtr<IMessage> createPingMessage(const ByteBuffer& data = ByteBuffer());
    SharedPtr<IMessage> createPongMessage(const ByteBuffer& data = ByteBuffer());
    SharedPtr<IMessage> createCloseMessage(uint16_t code = StatusCodes::NORMAL_CLOSURE, const std::string& reason = "");

    // Control message helpers
    SharedPtr<IMessage> createConnectionCloseMessage();
    SharedPtr<IMessage> createProtocolErrorMessage();
    SharedPtr<IMessage> createMessageTooBigError();

    // Message parsing
    SharedPtr<IMessage> parseMessage(const ByteBuffer& data);
    Result parseMessages(const ByteBuffer& data, std::vector<SharedPtr<IMessage>>& messages);

    // Message validation
    bool validateMessage(SharedPtr<IMessage> message) const;
    bool validateControlFrame(SharedPtr<IMessage> message) const;
    bool validateDataFrame(SharedPtr<IMessage> message) const;

    // Fragmentation support
    SharedPtr<IMessage> createFragmentedMessage(const std::vector<SharedPtr<IMessage>>& fragments);
    Result splitMessage(SharedPtr<IMessage> message, size_t fragmentSize, std::vector<SharedPtr<IMessage>>& fragments);

    // Registration system for custom message types
    void registerMessageCreator(FrameOpcodes::Opcode opcode, std::function<SharedPtr<IMessage>(const ByteBuffer&)> creator);
    void registerMessageValidator(FrameOpcodes::Opcode opcode, std::function<bool(SharedPtr<IMessage>)> validator);

    // Statistics
    struct FactoryStats {
        uint64_t messagesCreated;
        uint64_t messagesParsed;
        uint64_t textMessages;
        uint64_t binaryMessages;
        uint64_t controlMessages;
        uint64_t validationErrors;
    };

    FactoryStats getStats() const;
    void resetStats();

private:
    MessageFactory();
    ~MessageFactory() = default;

    std::unordered_map<FrameOpcodes::Opcode, std::function<SharedPtr<IMessage>(const ByteBuffer&)>> messageCreators_;
    std::unordered_map<FrameOpcodes::Opcode, std::function<bool(SharedPtr<IMessage>)>> messageValidators_;

    std::atomic<uint64_t> messagesCreated_{ 0 };
    std::atomic<uint64_t> messagesParsed_{ 0 };
    std::atomic<uint64_t> textMessages_{ 0 };
    std::atomic<uint64_t> binaryMessages_{ 0 };
    std::atomic<uint64_t> controlMessages_{ 0 };
    std::atomic<uint64_t> validationErrors_{ 0 };

    void initializeDefaultCreators();
    void initializeDefaultValidators();

    SharedPtr<IMessage> createDefaultMessage(FrameOpcodes::Opcode opcode, const ByteBuffer& payload);
    bool validateDefaultMessage(SharedPtr<IMessage> message);
    bool validateControlMessage(SharedPtr<IMessage> message);
};

WEBSOCKET_NAMESPACE_END