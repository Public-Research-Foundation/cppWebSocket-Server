#pragma once
#ifndef WEBSOCKET_STATUS_CODES_HPP
#define WEBSOCKET_STATUS_CODES_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <unordered_map>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @namespace StatusCodes
 * @brief WebSocket connection close status codes (RFC 6455 Section 7.4)
 *
 * Status codes provide reason for connection closure:
 * - 1000-1999: Standard WebSocket status codes
 * - 3000-3999: Registered by IANA
 * - 4000-4999: Private use for applications
 */
    namespace StatusCodes {
    // ============================================================================
    // STATUS CODE DEFINITIONS (RFC 6455 Section 7.4.1)
    // ============================================================================

    /**
     * @enum Code
     * @brief WebSocket connection close status codes
     */
    enum Code : uint16_t {
        // ==========================================
        // STANDARD STATUS CODES (1000-1999)
        // ==========================================

        /**
         * @brief Normal closure (1000)
         *
         * Connection closed normally without error.
         */
        NORMAL_CLOSURE = 1000,

        /**
         * @brief Endpoint going away (1001)
         *
         * Server or client is shutting down or page navigation.
         */
        GOING_AWAY = 1001,

        /**
         * @brief Protocol error (1002)
         *
         * Endpoint terminated connection due to protocol error.
         */
        PROTOCOL_ERROR = 1002,

        /**
         * @brief Unsupported data (1003)
         *
         * Endpoint received unsupported data type (e.g., binary when expecting text).
         */
        UNSUPPORTED_DATA = 1003,

        /**
         * @brief No status received (1005)
         *
         * Reserved. Must not be sent in close frame.
         */
        NO_STATUS_RCVD = 1005,

        /**
         * @brief Abnormal closure (1006)
         *
         * Reserved. Must not be sent in close frame.
         */
        ABNORMAL_CLOSURE = 1006,

        /**
         * @brief Invalid frame payload data (1007)
         *
         * Endpoint received inconsistent data (e.g., non-UTF-8 in text frame).
         */
        INVALID_FRAME_PAYLOAD_DATA = 1007,

        /**
         * @brief Policy violation (1008)
         *
         * Endpoint terminated connection due to policy violation.
         */
        POLICY_VIOLATION = 1008,

        /**
         * @brief Message too big (1009)
         *
         * Endpoint received message too large to process.
         */
        MESSAGE_TOO_BIG = 1009,

        /**
         * @brief Mandatory extension missing (1010)
         *
         * Client expected server to negotiate one or more extensions.
         */
        MANDATORY_EXTENSION = 1010,

        /**
         * @brief Internal server error (1011)
         *
         * Server encountered unexpected condition.
         */
        INTERNAL_SERVER_ERROR = 1011,

        /**
         * @brief TLS handshake failed (1015)
         *
         * Reserved. Must not be sent in close frame.
         */
        TLS_HANDSHAKE_FAILED = 1015,

        // ==========================================
        // APPLICATION-SPECIFIC STATUS CODES (4000-4999)
        // ==========================================

        /**
         * @brief Service restart (4000)
         *
         * Application-specific: Server is restarting.
         */
        SERVICE_RESTART = 4000,

        /**
         * @brief Try again later (4001)
         *
         * Application-specific: Client should retry connection later.
         */
        TRY_AGAIN_LATER = 4001
    };

    // ============================================================================
    // STATUS CODE VALIDATION FUNCTIONS
    // ============================================================================

    /**
     * @brief Check if status code is valid according to RFC 6455
     * @param code The status code to validate
     * @return true if code is valid and can be used in close frames
     *
     * @note Valid codes are 1000-1011 (excluding 1005, 1006, 1015) and 3000-4999
     */
    constexpr bool isValidStatusCode(uint16_t code) {
        // RFC 6455 Section 7.4.2 - Status Code Ranges
        if (code >= 1000 && code <= 1999) {
            // Standard status codes (exclude reserved codes)
            return code != NO_STATUS_RCVD &&
                code != ABNORMAL_CLOSURE &&
                code != TLS_HANDSHAKE_FAILED;
        }
        if (code >= 2000 && code <= 2999) {
            // Reserved range for WebSocket extensions
            return false;
        }
        if (code >= 3000 && code <= 4999) {
            // Registered (3000-3999) and private use (4000-4999)
            return true;
        }
        return false;
    }

    /**
     * @brief Check if status code indicates a protocol error
     * @param code The status code to check
     * @return true if code represents a protocol-level error
     */
    constexpr bool isProtocolErrorCode(uint16_t code) {
        return (code >= PROTOCOL_ERROR && code <= POLICY_VIOLATION) ||
            code == INTERNAL_SERVER_ERROR;
    }

    /**
     * @brief Check if status code can be sent in close frame
     * @param code The status code to check
     * @return true if code can be included in close frame payload
     */
    constexpr bool canSendInCloseFrame(uint16_t code) {
        return code != NO_STATUS_RCVD &&
            code != ABNORMAL_CLOSURE &&
            code != TLS_HANDSHAKE_FAILED;
    }

    // ============================================================================
    // STATUS CODE DESCRIPTIONS
    // ============================================================================

    /**
     * @brief Get human-readable description for status code
     * @param code The status code
     * @return Description string for the status code
     */
    inline const std::string& getDescription(uint16_t code) {
        static const std::unordered_map<uint16_t, std::string> descriptions = {
            {NORMAL_CLOSURE,              "Normal closure"},
            {GOING_AWAY,                  "Endpoint is going away"},
            {PROTOCOL_ERROR,              "Protocol error"},
            {UNSUPPORTED_DATA,            "Received unsupported data type"},
            {INVALID_FRAME_PAYLOAD_DATA,  "Invalid frame payload data"},
            {POLICY_VIOLATION,            "Policy violation"},
            {MESSAGE_TOO_BIG,             "Message too big"},
            {MANDATORY_EXTENSION,         "Mandatory extension missing"},
            {INTERNAL_SERVER_ERROR,       "Internal server error"},
            {SERVICE_RESTART,             "Service restart"},
            {TRY_AGAIN_LATER,             "Try again later"}
        };

        static const std::string unknown = "Unknown status code";
        auto it = descriptions.find(code);
        return it != descriptions.end() ? it->second : unknown;
    }

    /**
     * @brief Check if status code indicates a clean closure
     * @param code The status code to check
     * @return true if closure was normal and expected
     */
    constexpr bool isCleanClosure(uint16_t code) {
        return code == NORMAL_CLOSURE || code == GOING_AWAY;
    }
}

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_STATUS_CODES_HPP