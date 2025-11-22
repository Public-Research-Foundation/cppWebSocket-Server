#pragma once
#ifndef WEBSOCKET_TYPES_HPP
#define WEBSOCKET_TYPES_HPP

#include "Macros.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <system_error>

WEBSOCKET_NAMESPACE_BEGIN

// ============================================================================
// FUNDAMENTAL TYPES
// ============================================================================

/**
 * @brief Byte type (8-bit unsigned integer)
 */
    using Byte = uint8_t;

/**
 * @brief Buffer type for binary data
 */
using Buffer = std::vector<Byte>;

/**
 * @brief String type (UTF-8 encoded)
 */
using String = std::string;

/**
 * @brief Size type for buffer and container sizes
 */
using Size = size_t;

/**
 * @brief Client identifier type
 */
using ClientID = uint64_t;

/**
 * @brief Timestamp type (steady clock for performance measurements)
 */
using Timestamp = std::chrono::steady_clock::time_point;

/**
 * @brief Duration type for time intervals
 */
using Duration = std::chrono::steady_clock::duration;

// ============================================================================
// SMART POINTER ALIASES
// ============================================================================

/**
 * @brief Unique pointer with custom deleter support
 */
template<typename T>
using UniquePtr = std::unique_ptr<T>;

/**
 * @brief Shared pointer for shared ownership
 */
template<typename T>
using SharedPtr = std::shared_ptr<T>;

/**
 * @brief Weak pointer for non-owning references
 */
template<typename T>
using WeakPtr = std::weak_ptr<T>;

// ============================================================================
// FUNCTIONAL TYPES
// ============================================================================

/**
 * @brief Simple callback without parameters
 */
using Callback = std::function<void()>;

/**
 * @brief Callback with error parameter
 */
using ErrorCallback = std::function<void(const std::error_code&)>;

/**
 * @brief Generic event callback with data
 */
template<typename T>
using EventCallback = std::function<void(const T&)>;

/**
 * @brief Message handler callback
 */
using MessageHandler = std::function<void(ClientID, const Buffer&)>;

/**
 * @brief Connection event callback
 */
using ConnectionHandler = std::function<void(ClientID)>;

// ============================================================================
// RESULT AND ERROR TYPES
// ============================================================================

/**
 * @brief Operation result codes
 */
enum class Result {
    SUCCESS,                ///< Operation completed successfully
    ERROR,                  ///< General operation error
    TIMEOUT,                ///< Operation timed out
    INVALID_STATE,          ///< Invalid state for operation
    CONNECTION_CLOSED,      ///< Connection was closed
    PROTOCOL_ERROR,         ///< Protocol violation detected
    BUFFER_OVERFLOW,        ///< Buffer capacity exceeded
    INVALID_ARGUMENT,       ///< Invalid function argument
    RESOURCE_EXHAUSTED,     ///< System resource limit reached
    NOT_IMPLEMENTED         ///< Feature not implemented
};

/**
 * @brief Extended error information
 */
struct Error {
    int code;                           ///< Error code
    String message;                     ///< Human-readable error message
    String details;                     ///< Additional error details
    Timestamp timestamp;               ///< When error occurred

    /**
     * @brief Default constructor
     */
    Error() : code(0), timestamp(std::chrono::steady_clock::now()) {}

    /**
     * @brief Constructor with code and message
     */
    Error(int errorCode, const String& errorMessage)
        : code(errorCode), message(errorMessage), timestamp(std::chrono::steady_clock::now()) {
    }

    /**
     * @brief Check if error is valid (non-zero code)
     */
    explicit operator bool() const { return code != 0; }

    /**
     * @brief Get string representation
     */
    String toString() const {
        return "Error " + std::to_string(code) + ": " + message;
    }
};

/**
 * @brief Template result type that can hold value or error
 */
template<typename T>
class ResultValue {
public:
    /**
     * @brief Construct with success value
     */
    ResultValue(T&& value) : value_(std::move(value)), success_(true) {}

    /**
     * @brief Construct with error
     */
    ResultValue(const Error& error) : error_(error), success_(false) {}

    /**
     * @brief Check if result is successful
     */
    bool isSuccess() const { return success_; }

    /**
     * @brief Check if result is error
     */
    bool isError() const { return !success_; }

    /**
     * @brief Get value (throws if error)
     */
    T& value() {
        if (!success_) {
            throw std::runtime_error("Attempt to access value of failed result: " + error_.message);
        }
        return value_;
    }

    /**
     * @brief Get value (const version)
     */
    const T& value() const {
        if (!success_) {
            throw std::runtime_error("Attempt to access value of failed result: " + error_.message);
        }
        return value_;
    }

    /**
     * @brief Get error
     */
    const Error& error() const { return error_; }

    /**
     * @brief Get value or default if error
     */
    T valueOr(T defaultValue) const {
        return success_ ? value_ : defaultValue;
    }

private:
    T value_;
    Error error_;
    bool success_;
};

// ============================================================================
// WEB SOCKET SPECIFIC TYPES
// ============================================================================

/**
 * @brief WebSocket frame opcodes
 */
enum class Opcode : uint8_t {
    CONTINUATION = 0x0,     ///< Continuation frame
    TEXT = 0x1,             ///< Text frame
    BINARY = 0x2,           ///< Binary frame
    CLOSE = 0x8,            ///< Connection close frame
    PING = 0x9,             ///< Ping frame
    PONG = 0xA              ///< Pong frame
};

/**
 * @brief WebSocket close status codes
 */
enum class CloseCode : uint16_t {
    NORMAL_CLOSURE = 1000,          ///< Normal closure
    GOING_AWAY = 1001,              ///< Endpoint going away
    PROTOCOL_ERROR = 1002,          ///< Protocol error
    UNSUPPORTED_DATA = 1003,        ///< Unsupported data
    NO_STATUS_RCVD = 1005,          ///< No status received
    ABNORMAL_CLOSURE = 1006,        ///< Abnormal closure
    INVALID_PAYLOAD = 1007,         ///< Invalid payload data
    POLICY_VIOLATION = 1008,        ///< Policy violation
    MESSAGE_TOO_BIG = 1009,         ///< Message too big
    MANDATORY_EXTENSION = 1010,     ///< Mandatory extension missing
    INTERNAL_ERROR = 1011,          ///< Internal server error
    SERVICE_RESTART = 1012,         ///< Service restart
    TRY_AGAIN_LATER = 1013,         ///< Try again later
    TLS_HANDSHAKE_FAILED = 1015     ///< TLS handshake failed
};

/**
 * @brief WebSocket message structure
 */
struct Message {
    Buffer data;                    ///< Message payload data
    bool isText{ false };             ///< true for TEXT, false for BINARY
    Opcode opcode{ Opcode::TEXT };    ///< Original opcode
    Timestamp timestamp;           ///< When message was created/received

    /**
     * @brief Default constructor
     */
    Message() : timestamp(std::chrono::steady_clock::now()) {}

    /**
     * @brief Constructor with data and type
     */
    Message(Buffer&& msgData, bool text = true)
        : data(std::move(msgData)), isText(text), timestamp(std::chrono::steady_clock::now()) {
    }

    /**
     * @brief Constructor from string (text message)
     */
    Message(const String& text)
        : isText(true), timestamp(std::chrono::steady_clock::now()) {
        data.assign(text.begin(), text.end());
    }

    /**
     * @brief Get message as string (for text messages)
     */
    String getText() const {
        return String(data.begin(), data.end());
    }

    /**
     * @brief Get message size in bytes
     */
    Size size() const {
        return data.size();
    }

    /**
     * @brief Check if message is empty
     */
    bool empty() const {
        return data.empty();
    }
};

// ============================================================================
// CONTAINER ALIASES
// ============================================================================

/**
 * @brief Map from client ID to session data
 */
template<typename T>
using ClientMap = std::unordered_map<ClientID, T>;

/**
 * @brief String map for configuration and headers
 */
using StringMap = std::unordered_map<String, String>;

/**
 * @brief String list for collections
 */
using StringList = std::vector<String>;

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_TYPES_HPP