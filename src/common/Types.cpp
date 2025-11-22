#include "common/Types.hpp"
#include <sstream>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Types - Implementation of common type utilities and helper functions
 * Provides type conversions, string representations, and utility operations
 */

 // Error class implementation

 /**
  * Error constructor
  *
  * @param code: Numeric error code for machine-readable error identification
  * @param message: Human-readable error description
  *
  * Purpose: Creates a standardized error object with code and message
  */
    Error::Error(int code, const std::string& message)
    : code(code), message(message) {
}

/**
 * Convert error to string representation
 *
 * @return: Formatted string containing error code and message
 *
 * Purpose: Provides human-readable error representation for logging and display
 */
std::string Error::toString() const {
    std::ostringstream oss;
    oss << "Error " << code << ": " << message;
    return oss.str();
}

/**
 * Error equality comparison
 *
 * @param other: Error object to compare with
 * @return: true if both code and message are identical
 *
 * Purpose: Allows error objects to be compared for equality
 */
bool Error::operator==(const Error& other) const {
    return code == other.code && message == other.message;
}

// Result enumeration utility functions

/**
 * Convert Result enum to string representation
 *
 * @param result: Result value to convert
 * @return: String name of the result value
 *
 * Purpose: Provides human-readable result names for logging and debugging
 */
std::string resultToString(Result result) {
    switch (result) {
    case Result::SUCCESS: return "SUCCESS";
    case Result::ERROR: return "ERROR";
    case Result::TIMEOUT: return "TIMEOUT";
    case Result::INVALID_STATE: return "INVALID_STATE";
    case Result::CONNECTION_CLOSED: return "CONNECTION_CLOSED";
    default: return "UNKNOWN";
    }
}

/**
 * Check if result represents success
 *
 * @param result: Result value to check
 * @return: true if result is SUCCESS
 *
 * Purpose: Provides clean success/failure checking in conditionals
 */
bool resultIsSuccess(Result result) {
    return result == Result::SUCCESS;
}

/**
 * Check if result represents failure
 *
 * @param result: Result value to check
 * @return: true if result is not SUCCESS
 *
 * Purpose: Inverse of resultIsSuccess for failure checking
 */
bool resultIsFailure(Result result) {
    return result != Result::SUCCESS;
}

// ByteBuffer utility functions

/**
 * Convert ByteBuffer to std::string
 *
 * @param buffer: ByteBuffer containing binary data
 * @return: String representation of the binary data
 *
 * Purpose: Converts binary WebSocket message data to string for text messages
 * Note: Assumes buffer contains valid UTF-8 text data
 */
std::string byteBufferToString(const ByteBuffer& buffer) {
    return std::string(buffer.begin(), buffer.end());
}

/**
 * Convert std::string to ByteBuffer
 *
 * @param str: String to convert to binary data
 * @return: ByteBuffer containing the string data as bytes
 *
 * Purpose: Converts string text to binary format for WebSocket transmission
 */
ByteBuffer stringToByteBuffer(const std::string& str) {
    return ByteBuffer(str.begin(), str.end());
}

/**
 * Convert raw data pointer to ByteBuffer
 *
 * @param data: Pointer to raw binary data
 * @param length: Number of bytes to copy
 * @return: ByteBuffer containing copied data
 *
 * Purpose: Safely converts C-style byte arrays to ByteBuffer with bounds checking
 */
ByteBuffer dataToByteBuffer(const void* data, size_t length) {
    const Byte* byteData = static_cast<const Byte*>(data);
    return ByteBuffer(byteData, byteData + length);
}

// ConnectionState utility functions

/**
 * Convert ConnectionState enum to string representation
 *
 * @param state: ConnectionState value to convert
 * @return: String name of the connection state
 *
 * Purpose: Provides human-readable connection state names for logging and monitoring
 */
std::string connectionStateToString(ConnectionState state) {
    switch (state) {
    case ConnectionState::DISCONNECTED: return "DISCONNECTED";
    case ConnectionState::CONNECTING: return "CONNECTING";
    case ConnectionState::CONNECTED: return "CONNECTED";
    case ConnectionState::DISCONNECTING: return "DISCONNECTING";
    case ConnectionState::ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}

WEBSOCKET_NAMESPACE_END