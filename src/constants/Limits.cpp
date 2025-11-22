#include "constants/Limits.hpp"

WEBSOCKET_NAMESPACE_BEGIN

namespace Limits {

	// Frame size limits (RFC 6455 Section 5.2)
	constexpr size_t MAX_FRAME_HEADER_SIZE;
	constexpr size_t MAX_CONTROL_FRAME_PAYLOAD;
	constexpr size_t MIN_FRAGMENT_SIZE;

	// Payload length encoding (RFC 6455 Section 5.2)
	constexpr uint64_t PAYLOAD_LEN_7BIT_MAX;
	constexpr uint64_t PAYLOAD_LEN_16BIT_MAX;
	constexpr uint64_t PAYLOAD_LEN_16BIT;
	constexpr uint64_t PAYLOAD_LEN_64BIT;

	// Message size limits
	constexpr uint64_t MAX_64BIT_PAYLOAD;
	constexpr uint64_t PRACTICAL_MAX_MESSAGE_SIZE;

	// Close frame limits (RFC 6455 Section 5.5.1)
	constexpr size_t MAX_CLOSE_REASON_BYTES;

	// Handshake limits
	constexpr size_t MAX_HANDSHAKE_SIZE;
	constexpr size_t MAX_HEADER_SIZE;
	constexpr size_t MAX_HEADER_COUNT;

	// Operational limits (RFC compliant but configurable)
	constexpr size_t DEFAULT_MAX_FRAME_SIZE;
	constexpr size_t DEFAULT_MAX_MESSAGE_SIZE;
	constexpr size_t DEFAULT_MAX_CONNECTIONS;
	constexpr size_t MIN_PING_INTERVAL;

	// Buffer sizes
	constexpr size_t DEFAULT_BUFFER_SIZE;
	constexpr size_t MAX_BUFFER_POOL_SIZE;

	// Timeouts (milliseconds)
	constexpr uint32_t DEFAULT_HANDSHAKE_TIMEOUT;
	constexpr uint32_t DEFAULT_PING_INTERVAL;
	constexpr uint32_t DEFAULT_PONG_TIMEOUT;
	constexpr uint32_t DEFAULT_CLOSE_TIMEOUT;

} // namespace Limits

WEBSOCKET_NAMESPACE_END