#pragma once

#pragma once
#ifndef WEBSOCKET_LIMITS_HPP
#define WEBSOCKET_LIMITS_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @namespace Limits
 * @brief WebSocket protocol limits and operational boundaries (RFC 6455)
 *
 * Defines size limits, timeouts, and operational boundaries for:
 * - Frame and message sizing
 * - Handshake and header processing
 * - Buffer management and resource limits
 * - Timeout values for various operations
 */
    namespace Limits {
    // ============================================================================
    // FRAME SIZE LIMITS (RFC 6455 Section 5.2)
    // ============================================================================

    /**
     * @brief Maximum WebSocket frame header size in bytes
     *
     * 2 bytes (basic header) + 8 bytes (extended length) + 4 bytes (masking key)
     */
    constexpr size_t MAX_FRAME_HEADER_SIZE = 14;

    /**
     * @brief Maximum control frame payload size in bytes (RFC 6455 Section 5.5)
     *
     * Control frames (CLOSE, PING, PONG) must have payload ≤ 125 bytes.
     */
    constexpr size_t MAX_CONTROL_FRAME_PAYLOAD = 125;

    /**
     * @brief Minimum fragment size for efficient message fragmentation
     */
    constexpr size_t MIN_FRAGMENT_SIZE = 1;

    // ============================================================================
    // PAYLOAD LENGTH ENCODING (RFC 6455 Section 5.2)
    // ============================================================================

    /**
     * @brief Maximum payload length for 7-bit encoding
     */
    constexpr uint64_t PAYLOAD_LEN_7BIT_MAX = 125;

    /**
     * @brief Maximum payload length for 16-bit encoding
     */
    constexpr uint64_t PAYLOAD_LEN_16BIT_MAX = 65535;

    /**
     * @brief Indicator value for 16-bit extended payload length
     */
    constexpr uint64_t PAYLOAD_LEN_16BIT = 126;

    /**
     * @brief Indicator value for 64-bit extended payload length
     */
    constexpr uint64_t PAYLOAD_LEN_64BIT = 127;

    // ============================================================================
    // MESSAGE SIZE LIMITS
    // ============================================================================

    /**
     * @brief Theoretical maximum payload size (2^63-1 per RFC 6455)
     */
    constexpr uint64_t MAX_64BIT_PAYLOAD = 0x7FFFFFFFFFFFFFFF;

    /**
     * @brief Practical maximum message size for implementation safety
     */
    constexpr uint64_t PRACTICAL_MAX_MESSAGE_SIZE = 0x4000000000000000; // 2^62

    // ============================================================================
    // CLOSE FRAME LIMITS (RFC 6455 Section 5.5.1)
    // ============================================================================

    /**
     * @brief Maximum close reason size in bytes
     *
     * Close frame: 2 bytes status code + reason bytes ≤ 125 total
     */
    constexpr size_t MAX_CLOSE_REASON_BYTES = 123;

    // ============================================================================
    // HANDSHAKE LIMITS
    // ============================================================================

    /**
     * @brief Maximum HTTP handshake size in bytes
     */
    constexpr size_t MAX_HANDSHAKE_SIZE = 8192;

    /**
     * @brief Maximum individual HTTP header size in bytes
     */
    constexpr size_t MAX_HEADER_SIZE = 4096;

    /**
     * @brief Maximum number of HTTP headers in handshake
     */
    constexpr size_t MAX_HEADER_COUNT = 100;

    // ============================================================================
    // OPERATIONAL LIMITS (RFC COMPLIANT BUT CONFIGURABLE)
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
     * @brief Default maximum concurrent connections
     */
    constexpr size_t DEFAULT_MAX_CONNECTIONS = 65536;

    /**
     * @brief Minimum ping interval in milliseconds (1 second)
     */
    constexpr size_t MIN_PING_INTERVAL = 1000;

    // ============================================================================
    // BUFFER SIZES
    // ============================================================================

    /**
     * @brief Default buffer size for I/O operations (8KB)
     */
    constexpr size_t DEFAULT_BUFFER_SIZE = 8192;

    /**
     * @brief Maximum buffer pool size to prevent memory exhaustion
     */
    constexpr size_t MAX_BUFFER_POOL_SIZE = 10000;

    // ============================================================================
    // TIMEOUTS (MILLISECONDS)
    // ============================================================================

    /**
     * @brief Default handshake timeout (30 seconds)
     */
    constexpr uint32_t DEFAULT_HANDSHAKE_TIMEOUT = 30000;

    /**
     * @brief Default ping interval (30 seconds)
     */
    constexpr uint32_t DEFAULT_PING_INTERVAL = 30000;

    /**
     * @brief Default pong response timeout (10 seconds)
     */
    constexpr uint32_t DEFAULT_PONG_TIMEOUT = 10000;

    /**
     * @brief Default close handshake timeout (5 seconds)
     */
    constexpr uint32_t DEFAULT_CLOSE_TIMEOUT = 5000;

    // ============================================================================
    // VALIDATION FUNCTIONS
    // ============================================================================

    /**
     * @brief Check if frame size is valid according to RFC 6455
     * @param size The frame size to validate
     * @return true if size is within protocol limits
     */
    constexpr bool isValidFrameSize(uint64_t size) {
        return size <= MAX_64BIT_PAYLOAD;
    }

    /**
     * @brief Check if control frame size is valid
     * @param size The control frame payload size
     * @return true if size ≤ MAX_CONTROL_FRAME_PAYLOAD
     */
    constexpr bool isValidControlFrameSize(uint64_t size) {
        return size <= MAX_CONTROL_FRAME_PAYLOAD;
    }

    /**
     * @brief Check if close reason size is valid
     * @param size The close reason string size
     * @return true if size ≤ MAX_CLOSE_REASON_BYTES
     */
    constexpr bool isValidCloseReasonSize(size_t size) {
        return size <= MAX_CLOSE_REASON_BYTES;
    }

    /**
     * @brief Check if payload length requires extended length encoding
     * @param payloadLength The payload length to check
     * @return true if length > 125 bytes
     */
    constexpr bool requiresExtendedLength(uint64_t payloadLength) {
        return payloadLength > PAYLOAD_LEN_7BIT_MAX;
    }

    /**
     * @brief Check if payload length requires 64-bit length encoding
     * @param payloadLength The payload length to check
     * @return true if length > 65535 bytes
     */
    constexpr bool requires64BitLength(uint64_t payloadLength) {
        return payloadLength > PAYLOAD_LEN_16BIT_MAX;
    }

    /**
     * @brief Get the required header size for given payload length
     * @param payloadLength The payload length
     * @param masked Whether the frame is masked
     * @return Total header size in bytes
     */
    constexpr size_t getHeaderSize(uint64_t payloadLength, bool masked = false) {
        size_t size = 2; // Basic header

        if (requiresExtendedLength(payloadLength)) {
            size += (payloadLength > PAYLOAD_LEN_16BIT_MAX) ? 8 : 2;
        }

        if (masked) {
            size += 4; // Masking key
        }

        return size;
    }
}

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_LIMITS_HPP