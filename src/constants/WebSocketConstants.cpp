#include "constants/WebSocketConstants.hpp"

WEBSOCKET_NAMESPACE_BEGIN

// RFC 6455 Section 1.3 - Protocol Overview
constexpr const char* WebSocketConstants::WEBSOCKET_VERSION;
constexpr const char* WebSocketConstants::WEBSOCKET_GUID;

// RFC 6455 Section 4.1 - Client-Server Opening Handshake
constexpr const char* WebSocketConstants::WEBSOCKET_KEY_HEADER;
constexpr const char* WebSocketConstants::WEBSOCKET_VERSION_HEADER;
constexpr const char* WebSocketConstants::WEBSOCKET_PROTOCOL_HEADER;
constexpr const char* WebSocketConstants::WEBSOCKET_EXTENSIONS_HEADER;
constexpr const char* WebSocketConstants::WEBSOCKET_ACCEPT_HEADER;
constexpr const char* WebSocketConstants::WEBSOCKET_UPGRADE_HEADER;
constexpr const char* WebSocketConstants::WEBSOCKET_CONNECTION_HEADER;

constexpr const char* WebSocketConstants::UPGRADE_WEBSOCKET;
constexpr const char* WebSocketConstants::CONNECTION_UPGRADE;

// RFC 6455 Section 5.1 - Framing
constexpr size_t WebSocketConstants::MIN_FRAME_HEADER_SIZE;
constexpr size_t WebSocketConstants::MAX_FRAME_HEADER_SIZE;
constexpr size_t WebSocketConstants::MASK_KEY_SIZE;

// RFC 6455 Section 5.2 - Base Framing Protocol
constexpr uint8_t WebSocketConstants::FIN_BIT;
constexpr uint8_t WebSocketConstants::RSV1_BIT;
constexpr uint8_t WebSocketConstants::RSV2_BIT;
constexpr uint8_t WebSocketConstants::RSV3_BIT;
constexpr uint8_t WebSocketConstants::OPCODE_MASK;
constexpr uint8_t WebSocketConstants::MASK_BIT;
constexpr uint8_t WebSocketConstants::PAYLOAD_LEN_MASK;

// RFC 6455 Section 5.5.1 - Close
constexpr size_t WebSocketConstants::MAX_CLOSE_REASON_LENGTH;

// RFC 6455 Section 5.5.2 - Ping and Pong
constexpr size_t WebSocketConstants::MAX_PING_PONG_PAYLOAD;

// RFC 6455 Section 7.4.1 - Defined Status Codes
constexpr uint16_t WebSocketConstants::STATUS_NORMAL_CLOSURE;
constexpr uint16_t WebSocketConstants::STATUS_GOING_AWAY;
constexpr uint16_t WebSocketConstants::STATUS_PROTOCOL_ERROR;
constexpr uint16_t WebSocketConstants::STATUS_UNSUPPORTED_DATA;
constexpr uint16_t WebSocketConstants::STATUS_NO_STATUS_RCVD;
constexpr uint16_t WebSocketConstants::STATUS_ABNORMAL_CLOSURE;
constexpr uint16_t WebSocketConstants::STATUS_INVALID_FRAME_PAYLOAD_DATA;
constexpr uint16_t WebSocketConstants::STATUS_POLICY_VIOLATION;
constexpr uint16_t WebSocketConstants::STATUS_MESSAGE_TOO_BIG;
constexpr uint16_t WebSocketConstants::STATUS_MANDATORY_EXT;
constexpr uint16_t WebSocketConstants::STATUS_INTERNAL_ERROR;
constexpr uint16_t WebSocketConstants::STATUS_TLS_HANDSHAKE;

// Operational limits (RFC compliant defaults)
constexpr size_t WebSocketConstants::DEFAULT_MAX_FRAME_SIZE;
constexpr size_t WebSocketConstants::DEFAULT_MAX_MESSAGE_SIZE;
constexpr size_t WebSocketConstants::MIN_FRAGMENT_SIZE;

WEBSOCKET_NAMESPACE_END