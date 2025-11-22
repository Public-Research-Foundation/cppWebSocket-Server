#include "constants/StatusCodes.hpp"
#include <unordered_map>

WEBSOCKET_NAMESPACE_BEGIN

namespace StatusCodes {

    // Status code descriptions (RFC 6455 Section 7.4.1)
    const std::string& getDescription(uint16_t code) {
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

} // namespace StatusCodes

WEBSOCKET_NAMESPACE_END