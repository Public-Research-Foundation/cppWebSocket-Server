#pragma once
#ifndef WEBSOCKET_CONSTANTS_HPP
#define WEBSOCKET_CONSTANTS_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @namespace WebSocketConstants
 * @brief RFC 6455 WebSocket protocol constants and definitions
 *
 * Contains all magic strings, header names, and protocol-specific
 * constants required for WebSocket implementation compliance.
 */
    namespace WebSocketConstants {
    // ============================================================================
    // PROTOCOL IDENTIFICATION
    // ============================================================================

    /**
     * @brief WebSocket version supported by this implementation (RFC 6455)
     */
    constexpr const char* WEBSOCKET_VERSION = "13";

    /**
     * @brief GUID for WebSocket key hashing (RFC 6455 Section 1.3)
     *
     * Used in handshake: base64(sha1(key + GUID))
     */
    constexpr const char* WEBSOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // ============================================================================
    // HTTP HEADER NAMES (RFC 6455 Section 4.1)
    // ============================================================================

    /**
     * @brief Client WebSocket key header
     */
    constexpr const char* WEBSOCKET_KEY_HEADER = "Sec-WebSocket-Key";

    /**
     * @brief WebSocket version header
     */
    constexpr const char* WEBSOCKET_VERSION_HEADER = "Sec-WebSocket-Version";

    /**
     * @brief WebSocket subprotocol header
     */
    constexpr const char* WEBSOCKET_PROTOCOL_HEADER = "Sec-WebSocket-Protocol";

    /**
     * @brief WebSocket extensions header
     */
    constexpr const char* WEBSOCKET_EXTENSIONS_HEADER = "Sec-WebSocket-Extensions";

    /**
     * @brief Server WebSocket accept header
     */
    constexpr const char* WEBSOCKET_ACCEPT_HEADER = "Sec-WebSocket-Accept";

    /**
     * @brief HTTP upgrade header
     */
    constexpr const char* WEBSOCKET_UPGRADE_HEADER = "Upgrade";

    /**
     * @brief HTTP connection header
     */
    constexpr const char* WEBSOCKET_CONNECTION_HEADER = "Connection";

    // ============================================================================
    // HTTP HEADER VALUES
    // ============================================================================

    /**
     * @brief Upgrade header value for WebSocket
     */
    constexpr const char* UPGRADE_WEBSOCKET = "websocket";

    /**
     * @brief Connection header value for upgrade
     */
    constexpr const char* CONNECTION_UPGRADE = "Upgrade";

    // ============================================================================
    // FRAME FORMAT CONSTANTS (RFC 6455 Section 5.2)
    // ============================================================================

    /**
     * @brief Minimum WebSocket frame header size (2 bytes)
     */
    constexpr size_t MIN_FRAME_HEADER_SIZE = 2;

    /**
     * @brief Maximum WebSocket frame header size (2 + 8 + 4 bytes)
     */
    constexpr size_t MAX_FRAME_HEADER_SIZE = 14;

    /**
     * @brief WebSocket masking key size in bytes
     */
    constexpr size_t MASK_KEY_SIZE = 4;

    // ============================================================================
    // FRAME BIT MASKS
    // ============================================================================

    /**
     * @brief FIN bit mask (final frame in message)
     */
    constexpr uint8_t FIN_BIT = 0x80;

    /**
     * @brief RSV1 bit mask (reserved for extensions)
     */
    constexpr uint8_t RSV1_BIT = 0x40;

    /**
     * @brief RSV2 bit mask (reserved for extensions)
     */
    constexpr uint8_t RSV2_BIT = 0x20;

    /**
     * @brief RSV3 bit mask (reserved for extensions)
     */
    constexpr uint8_t RSV3_BIT = 0x10;

    /**
     * @brief Opcode bit mask (extract opcode from first byte)
     */
    constexpr uint8_t OPCODE_MASK = 0x0F;

    /**
     * @brief Mask bit (indicates if payload is masked)
     */
    constexpr uint8_t MASK_BIT = 0x80;

    /**
     * @brief Payload length bit mask (extract length from second byte)
     */
    constexpr uint8_t PAYLOAD_LEN_MASK = 0x7F;

    // ============================================================================
    // CONTROL FRAME LIMITS
    // ============================================================================

    /**
     * @brief Maximum close reason length in bytes (RFC 6455 Section 5.5.1)
     */
    constexpr size_t MAX_CLOSE_REASON_LENGTH = 123;

    /**
     * @brief Maximum ping/pong payload size in bytes (RFC 6455 Section 5.5.2)
     */
    constexpr size_t MAX_PING_PONG_PAYLOAD = 125;

    // ============================================================================
    // OPERATIONAL DEFAULTS
    // ============================================================================

    /**
     * @brief Default maximum frame size (1MB)
     */
    constexpr size_t DEFAULT_MAX_FRAME_SIZE = 1048576;

    /**
     * @brief Default maximum message size (8MB)
     */
    constexpr size_t DEFAULT_MAX_MESSAGE_SIZE = 8388608;

    /**
     * @brief Minimum fragment size for efficient fragmentation (1KB)
     */
    constexpr size_t MIN_FRAGMENT_SIZE = 1024;
}

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_CONSTANTS_HPP