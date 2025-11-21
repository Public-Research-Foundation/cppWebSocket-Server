#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

namespace WebSocketConstants {
    // RFC 6455 Section 1.3 - Protocol Overview
    constexpr const char* WEBSOCKET_VERSION = "13";
    constexpr const char* WEBSOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // RFC 6455 Section 4.1 - Client-Server Opening Handshake
    constexpr const char* WEBSOCKET_KEY_HEADER = "Sec-WebSocket-Key";
    constexpr const char* WEBSOCKET_VERSION_HEADER = "Sec-WebSocket-Version";
    constexpr const char* WEBSOCKET_PROTOCOL_HEADER = "Sec-WebSocket-Protocol";
    constexpr const char* WEBSOCKET_EXTENSIONS_HEADER = "Sec-WebSocket-Extensions";
    constexpr const char* WEBSOCKET_ACCEPT_HEADER = "Sec-WebSocket-Accept";
    constexpr const char* WEBSOCKET_UPGRADE_HEADER = "Upgrade";
    constexpr const char* WEBSOCKET_CONNECTION_HEADER = "Connection";

    constexpr const char* UPGRADE_WEBSOCKET = "websocket";
    constexpr const char* CONNECTION_UPGRADE = "Upgrade";

    // RFC 6455 Section 5.1 - Framing
    constexpr size_t MIN_FRAME_HEADER_SIZE = 2;
    constexpr size_t MAX_FRAME_HEADER_SIZE = 14;
    constexpr size_t MASK_KEY_SIZE = 4;

    // RFC 6455 Section 5.2 - Base Framing Protocol
    constexpr uint8_t FIN_BIT = 0x80;
    constexpr uint8_t RSV1_BIT = 0x40;
    constexpr uint8_t RSV2_BIT = 0x20;
    constexpr uint8_t RSV3_BIT = 0x10;
    constexpr uint8_t OPCODE_MASK = 0x0F;
    constexpr uint8_t MASK_BIT = 0x80;
    constexpr uint8_t PAYLOAD_LEN_MASK = 0x7F;

    // RFC 6455 Section 5.5.1 - Close
    constexpr size_t MAX_CLOSE_REASON_LENGTH = 123;

    // RFC 6455 Section 5.5.2 - Ping and Pong
    constexpr size_t MAX_PING_PONG_PAYLOAD = 125;

    // RFC 6455 Section 7.4.1 - Defined Status Codes
    constexpr uint16_t STATUS_NORMAL_CLOSURE = 1000;
    constexpr uint16_t STATUS_GOING_AWAY = 1001;
    constexpr uint16_t STATUS_PROTOCOL_ERROR = 1002;
    constexpr uint16_t STATUS_UNSUPPORTED_DATA = 1003;
    constexpr uint16_t STATUS_NO_STATUS_RCVD = 1005;
    constexpr uint16_t STATUS_ABNORMAL_CLOSURE = 1006;
    constexpr uint16_t STATUS_INVALID_FRAME_PAYLOAD_DATA = 1007;
    constexpr uint16_t STATUS_POLICY_VIOLATION = 1008;
    constexpr uint16_t STATUS_MESSAGE_TOO_BIG = 1009;
    constexpr uint16_t STATUS_MANDATORY_EXT = 1010;
    constexpr uint16_t STATUS_INTERNAL_ERROR = 1011;
    constexpr uint16_t STATUS_TLS_HANDSHAKE = 1015;

    // Operational limits (RFC compliant defaults)
    constexpr size_t DEFAULT_MAX_FRAME_SIZE = 1048576; // 1MB
    constexpr size_t DEFAULT_MAX_MESSAGE_SIZE = 8388608; // 8MB
    constexpr size_t MIN_FRAGMENT_SIZE = 1024; // 1KB
}

WEBSOCKET_NAMESPACE_END