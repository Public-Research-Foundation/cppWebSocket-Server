#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <unordered_map>

WEBSOCKET_NAMESPACE_BEGIN

namespace StatusCodes {
    // RFC 6455 Section 7.4.1 - Defined Status Codes
    enum Code : uint16_t {
        // 1000-1999: Standard status codes
        NORMAL_CLOSURE = 1000,
        GOING_AWAY = 1001,
        PROTOCOL_ERROR = 1002,
        UNSUPPORTED_DATA = 1003,
        NO_STATUS_RCVD = 1005, // Must not be set in close frame
        ABNORMAL_CLOSURE = 1006, // Must not be set in close frame
        INVALID_FRAME_PAYLOAD_DATA = 1007,
        POLICY_VIOLATION = 1008,
        MESSAGE_TOO_BIG = 1009,
        MANDATORY_EXTENSION = 1010,
        INTERNAL_SERVER_ERROR = 1011,
        TLS_HANDSHAKE_FAILED = 1015, // Must not be set in close frame

        // 3000-3999: Registered status codes (IANA)
        // 4000-4999: Private use
        SERVICE_RESTART = 4000,
        TRY_AGAIN_LATER = 4001
    };

    // Validation functions
    constexpr bool isValidStatusCode(uint16_t code) {
        // RFC 6455 Section 7.4.2 - Status Code Ranges
        if (code >= 1000 && code <= 1999) {
            // Standard status codes
            return code != NO_STATUS_RCVD && code != ABNORMAL_CLOSURE && code != TLS_HANDSHAKE_FAILED;
        }
        if (code >= 2000 && code <= 2999) {
            // Reserved range
            return false;
        }
        if (code >= 3000 && code <= 4999) {
            // Registered and private use
            return true;
        }
        return false;
    }

    constexpr bool isProtocolErrorCode(uint16_t code) {
        return (code >= PROTOCOL_ERROR && code <= POLICY_VIOLATION) ||
            code == INTERNAL_SERVER_ERROR;
    }

    // Status code descriptions (RFC 6455 Section 7.4.1)
    inline const std::string& getDescription(uint16_t code) {
        static const std::unordered_map<uint16_t, std::string> descriptions = {
            {NORMAL_CLOSURE,              "Normal closure"},
            {GOING_AWAY,                  "Endpoint is going away"},
            {PROTOCOL_ERROR,              "Protocol error"},
            {UNSUPPORTED_DATA,            "Received unsupported data"},
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

    // Check if status code can be sent in close frame
    constexpr bool canSendInCloseFrame(uint16_t code) {
        return code != NO_STATUS_RCVD && code != ABNORMAL_CLOSURE && code != TLS_HANDSHAKE_FAILED;
    }
}

WEBSOCKET_NAMESPACE_END