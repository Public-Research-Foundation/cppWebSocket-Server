#include "constants/WebSocketConstants.hpp"

WEBSOCKET_NAMESPACE_BEGIN

namespace ErrorCodes {

    // WebSocket-specific error codes
    constexpr int HANDSHAKE_FAILED = 1000;
    constexpr int PROTOCOL_VIOLATION = 1001;
    constexpr int UNSUPPORTED_VERSION = 1002;
    constexpr int INVALID_FRAME = 1003;
    constexpr int MESSAGE_TOO_LARGE = 1004;
    constexpr int TIMEOUT = 1005;
    constexpr int CONNECTION_RESET = 1006;
    constexpr int SSL_ERROR = 1007;

    // System error codes
    constexpr int SYSTEM_ERROR = 2000;
    constexpr int MEMORY_ALLOCATION = 2001;
    constexpr int FILE_IO = 2002;
    constexpr int NETWORK_ERROR = 2003;

    // Configuration error codes
    constexpr int CONFIG_ERROR = 3000;
    constexpr int INVALID_CONFIG = 3001;
    constexpr int MISSING_CONFIG = 3002;

    /**
     * Get error category from error code
     */
    std::string getErrorCategory(int errorCode) {
        if (errorCode >= 1000 && errorCode < 2000) {
            return "WebSocket";
        }
        else if (errorCode >= 2000 && errorCode < 3000) {
            return "System";
        }
        else if (errorCode >= 3000 && errorCode < 4000) {
            return "Configuration";
        }
        else {
            return "Unknown";
        }
    }

    /**
     * Check if error code is recoverable
     */
    bool isRecoverableError(int errorCode) {
        switch (errorCode) {
        case TIMEOUT:
        case CONNECTION_RESET:
        case NETWORK_ERROR:
            return true;
        default:
            return false;
        }
    }

} // namespace ErrorCodes

WEBSOCKET_NAMESPACE_END