#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

namespace Limits {
    // RFC 6455 Section 1.4 - Design Philosophy
    // Protocol should impose minimal limitations

    // Frame size limits (RFC 6455 Section 5.2)
    constexpr size_t MAX_FRAME_HEADER_SIZE = 14; // 2 + 8 + 4 bytes (basic + extended + mask)
    constexpr size_t MAX_CONTROL_FRAME_PAYLOAD = 125; // Section 5.5
    constexpr size_t MIN_FRAGMENT_SIZE = 1;

    // Payload length encoding (RFC 6455 Section 5.2)
    constexpr uint64_t PAYLOAD_LEN_7BIT_MAX = 125;
    constexpr uint64_t PAYLOAD_LEN_16BIT_MAX = 65535;
    constexpr uint64_t PAYLOAD_LEN_16BIT = 126;
    constexpr uint64_t PAYLOAD_LEN_64BIT = 127;

    // Message size limits
    constexpr uint64_t MAX_64BIT_PAYLOAD = 0x7FFFFFFFFFFFFFFF; // 2^63-1 (theoretical max)
    constexpr uint64_t PRACTICAL_MAX_MESSAGE_SIZE = 0x4000000000000000; // 2^62 (practical limit)

    // Close frame limits (RFC 6455 Section 5.5.1)
    constexpr size_t MAX_CLOSE_REASON_BYTES = 123;

    // Handshake limits
    constexpr size_t MAX_HANDSHAKE_SIZE = 8192; // 8KB for initial handshake
    constexpr size_t MAX_HEADER_SIZE = 4096; // 4KB for individual headers
    constexpr size_t MAX_HEADER_COUNT = 100; // Maximum number of headers

    // Operational limits (RFC compliant but configurable)
    constexpr size_t DEFAULT_MAX_FRAME_SIZE = 1048576; // 1MB
    constexpr size_t DEFAULT_MAX_MESSAGE_SIZE = 8388608; // 8MB
    constexpr size_t DEFAULT_MAX_CONNECTIONS = 65536; // 64K connections
    constexpr size_t MIN_PING_INTERVAL = 1000; // 1 second minimum

    // Buffer sizes
    constexpr size_t DEFAULT_BUFFER_SIZE = 8192; // 8KB
    constexpr size_t MAX_BUFFER_POOL_SIZE = 10000; // 10K buffers

    // Timeouts (milliseconds)
    constexpr uint32_t DEFAULT_HANDSHAKE_TIMEOUT = 30000; // 30 seconds
    constexpr uint32_t DEFAULT_PING_INTERVAL = 30000; // 30 seconds
    constexpr uint32_t DEFAULT_PONG_TIMEOUT = 10000; // 10 seconds
    constexpr uint32_t DEFAULT_CLOSE_TIMEOUT = 5000; // 5 seconds

    // Validation functions
    constexpr bool isValidFrameSize(uint64_t size) {
        return size <= MAX_64BIT_PAYLOAD;
    }

    constexpr bool isValidControlFrameSize(uint64_t size) {
        return size <= MAX_CONTROL_FRAME_PAYLOAD;
    }

    constexpr bool isValidCloseReasonSize(size_t size) {
        return size <= MAX_CLOSE_REASON_BYTES;
    }

    constexpr bool requiresExtendedLength(uint64_t payloadLength) {
        return payloadLength > PAYLOAD_LEN_7BIT_MAX;
    }

    constexpr bool requires64BitLength(uint64_t payloadLength) {
        return payloadLength > PAYLOAD_LEN_16BIT_MAX;
    }
}

WEBSOCKET_NAMESPACE_END