#include "constants/WebSocketConstants.hpp"
#include <unordered_map>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Additional protocol-related constants and utilities
 * Extends the basic RFC 6455 constants with operational values
 */

    namespace ProtocolConstants {

    // WebSocket protocol versions
    const std::vector<std::string> SUPPORTED_VERSIONS = { "13", "8", "7" };

    // Default WebSocket subprotocols
    const std::vector<std::string> SUPPORTED_SUBPROTOCOLS = {
        "chat", "superchat", "soap", "wamp"
    };

    // Supported extensions
    const std::vector<std::string> SUPPORTED_EXTENSIONS = {
        "permessage-deflate",
        "x-webkit-deflate-frame"
    };

    // MIME types for WebSocket-related content
    const std::unordered_map<std::string, std::string> MIME_TYPES = {
        {"html", "text/html"},
        {"json", "application/json"},
        {"xml", "application/xml"},
        {"text", "text/plain"},
        {"binary", "application/octet-stream"}
    };

    // HTTP status codes for WebSocket handshake
    namespace HttpStatus {
        constexpr int SWITCHING_PROTOCOLS = 101;
        constexpr int BAD_REQUEST = 400;
        constexpr int UNAUTHORIZED = 401;
        constexpr int FORBIDDEN = 403;
        constexpr int NOT_FOUND = 404;
        constexpr int UPGRADE_REQUIRED = 426;
        constexpr int INTERNAL_SERVER_ERROR = 500;
        constexpr int SERVICE_UNAVAILABLE = 503;
    }

    // WebSocket error categories
    namespace ErrorCategory {
        constexpr int PROTOCOL = 1;
        constexpr int HANDSHAKE = 2;
        constexpr int FRAME = 3;
        constexpr int MESSAGE = 4;
        constexpr int NETWORK = 5;
        constexpr int SECURITY = 6;
    }

    /**
     * Check if a WebSocket version is supported
     */
    bool isVersionSupported(const std::string& version) {
        for (const auto& supported : SUPPORTED_VERSIONS) {
            if (supported == version) {
                return true;
            }
        }
        return false;
    }

    /**
     * Check if a subprotocol is supported
     */
    bool isSubprotocolSupported(const std::string& protocol) {
        for (const auto& supported : SUPPORTED_SUBPROTOCOLS) {
            if (supported == protocol) {
                return true;
            }
        }
        return false;
    }

    /**
     * Get the highest supported WebSocket version
     */
    std::string getHighestSupportedVersion() {
        if (SUPPORTED_VERSIONS.empty()) {
            return "13"; // Default to RFC 6455
        }
        return SUPPORTED_VERSIONS[0];
    }

    /**
     * Validate a WebSocket key (base64 encoded 16-byte value)
     */
    bool isValidWebSocketKey(const std::string& key) {
        if (key.empty() || key.length() != 24) { // 16 bytes base64 encoded
            return false;
        }

        // Basic base64 validation
        for (char c : key) {
            if (!isalnum(c) && c != '+' && c != '/' && c != '=') {
                return false;
            }
        }

        return true;
    }

} // namespace ProtocolConstants

WEBSOCKET_NAMESPACE_END