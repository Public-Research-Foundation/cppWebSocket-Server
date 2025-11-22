#pragma once
#ifndef WEBSOCKET_HANDSHAKE_HPP
#define WEBSOCKET_HANDSHAKE_HPP

#include "../common/Types.hpp"
#include "../constants/WebSocketConstants.hpp"
#include <string>
#include <unordered_map>
#include <optional>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class WebSocketHandshake
 * @brief Handles WebSocket HTTP upgrade handshake according to RFC 6455
 *
 * This class processes the initial HTTP upgrade request and generates
 * the appropriate response for WebSocket protocol negotiation.
 *
 * Handshake process:
 * 1. Client sends HTTP upgrade request
 * 2. Server validates request and computes accept key
 * 3. Server sends HTTP 101 Switching Protocols response
 * 4. WebSocket connection is established
 */
    class WebSocketHandshake {
    public:
        /**
         * @brief Handshake result codes
         */
        enum class Result {
            SUCCESS,                    ///< Handshake successful
            INVALID_REQUEST,            ///< Malformed HTTP request
            MISSING_HEADERS,            ///< Required headers missing
            UNSUPPORTED_VERSION,        ///< WebSocket version not supported
            INVALID_ORIGIN,             ///< Origin validation failed
            PROTOCOL_ERROR              ///< Other protocol errors
        };

        /**
         * @brief Default constructor
         */
        WebSocketHandshake() = default;

        /**
         * @brief Parse HTTP upgrade request
         * @param request HTTP request data
         * @return Result code indicating success or failure reason
         */
        Result parseRequest(const std::string& request);

        /**
         * @brief Generate HTTP upgrade response
         * @return HTTP response string
         * @throws std::runtime_error if handshake not properly initialized
         */
        std::string createResponse();

        /**
         * @brief Validate handshake request
         * @return Result code indicating validation result
         */
        Result validate() const;

        /**
         * @brief Get requested WebSocket subprotocols
         * @return Vector of requested subprotocols
         */
        std::vector<std::string> getRequestedSubprotocols() const;

        /**
         * @brief Set accepted subprotocol
         * @param protocol Subprotocol to accept (empty for none)
         */
        void setAcceptedSubprotocol(const std::string& protocol);

        /**
         * @brief Get handshake error message
         * @return Error description if handshake failed
         */
        std::string getErrorMessage() const;

        /**
         * @brief Get client's WebSocket key
         * @return Sec-WebSocket-Key value from client
         */
        std::string getClientKey() const;

        /**
         * @brief Get client's origin header
         * @return Origin value from client
         */
        std::string getOrigin() const;

        /**
         * @brief Get client's WebSocket version
         * @return WebSocket version from client
         */
        int getClientVersion() const;

    private:
        /**
         * @brief Parse HTTP request line
         * @param line HTTP request line
         * @return true if parsing successful
         */
        bool parseRequestLine(const std::string& line);

        /**
         * @brief Parse HTTP header line
         * @param line HTTP header line
         * @return true if parsing successful
         */
        bool parseHeaderLine(const std::string& line);

        /**
         * @brief Extract headers from HTTP request
         * @param request Complete HTTP request
         */
        void extractHeaders(const std::string& request);

        /**
         * @brief Generate WebSocket accept key
         * @param client_key Client's WebSocket key
         * @return Base64-encoded accept key
         */
        std::string generateAcceptKey(const std::string& client_key) const;

        /**
         * @brief Validate required headers are present
         * @return true if all required headers present
         */
        bool validateRequiredHeaders() const;

        /**
         * @brief Validate WebSocket version
         * @return true if version is supported
         */
        bool validateVersion() const;

        /**
         * @brief Validate upgrade header
         * @return true if upgrade header is valid
         */
        bool validateUpgradeHeader() const;

        /**
         * @brief Validate connection header
         * @return true if connection header is valid
         */
        bool validateConnectionHeader() const;

        // Handshake data
        std::string method_;                            ///< HTTP method
        std::string path_;                              ///< Request path
        std::string http_version_;                      ///< HTTP version
        std::unordered_map<std::string, std::string> headers_;  ///< HTTP headers
        std::string error_message_;                     ///< Error description
        std::string accepted_subprotocol_;              ///< Accepted subprotocol
        Result result_{ Result::SUCCESS };                ///< Handshake result
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_HANDSHAKE_HPP