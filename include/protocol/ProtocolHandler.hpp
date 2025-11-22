#pragma once
#ifndef WEBSOCKET_PROTOCOL_HANDLER_HPP
#define WEBSOCKET_PROTOCOL_HANDLER_HPP

#include "../common/Types.hpp"
#include "WebSocketFrame.hpp"
#include "WebSocketMessage.hpp"
#include "WebSocketHandshake.hpp"
#include <memory>
#include <functional>
#include <queue>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class ProtocolHandler
 * @brief Main WebSocket protocol state machine and message processor
 *
 * This class implements the complete WebSocket protocol state machine
 * as defined in RFC 6455, handling:
 * - Handshake negotiation
 * - Frame parsing and validation
 * - Message reassembly
 * - Control frame processing
 * - Protocol compliance enforcement
 * - Error handling and connection closure
 */
    class ProtocolHandler {
    public:
        /**
         * @brief Protocol states
         */
        enum class State {
            CONNECTING,         ///< Waiting for handshake
            OPEN,               ///< WebSocket connection open
            CLOSING,            ///< Close handshake in progress
            CLOSED              ///< Connection closed
        };

        /**
         * @brief Event callbacks
         */
        struct Callbacks {
            std::function<void(const WebSocketMessage&)> on_message;
            std::function<void(uint16_t code, const std::string& reason)> on_close;
            std::function<void(const Buffer& data)> on_ping;
            std::function<void(const Buffer& data)> on_pong;
            std::function<void(const std::string& error)> on_error;
        };

        /**
         * @brief Constructor
         */
        ProtocolHandler();

        /**
         * @brief Destructor
         */
        ~ProtocolHandler() = default;

        // Delete copy operations
        ProtocolHandler(const ProtocolHandler&) = delete;
        ProtocolHandler& operator=(const ProtocolHandler&) = delete;

        /**
         * @brief Process incoming data (handshake or frames)
         * @param data Raw incoming data
         * @return Number of bytes consumed
         */
        size_t processData(const Buffer& data);

        /**
         * @brief Process HTTP handshake request
         * @param request HTTP request data
         * @return Handshake result
         */
        WebSocketHandshake::Result processHandshake(const std::string& request);

        /**
         * @brief Get handshake response
         * @return HTTP response for successful handshake
         * @throws std::runtime_error if handshake not completed
         */
        std::string getHandshakeResponse();

        /**
         * @brief Create a WebSocket frame
         * @param opcode Frame opcode
         * @param payload Frame payload
         * @param fin FIN flag
         * @return Serialized frame data
         */
        Buffer createFrame(Opcode opcode, const Buffer& payload, bool fin = true);

        /**
         * @brief Create a text message frame
         * @param text Text message
         * @return Serialized frame data
         */
        Buffer createTextFrame(const std::string& text);

        /**
         * @brief Create a binary message frame
         * @param data Binary data
         * @return Serialized frame data
         */
        Buffer createBinaryFrame(const Buffer& data);

        /**
         * @brief Create a close frame
         * @param code Close status code
         * @param reason Close reason
         * @return Serialized close frame
         */
        Buffer createCloseFrame(uint16_t code = 1000, const std::string& reason = "");

        /**
         * @brief Create a ping frame
         * @param data Ping data
         * @return Serialized ping frame
         */
        Buffer createPingFrame(const Buffer& data = {});

        /**
         * @brief Create a pong frame
         * @param data Pong data
         * @return Serialized pong frame
         */
        Buffer createPongFrame(const Buffer& data = {});

        /**
         * @brief Initiate close handshake
         * @param code Close status code
         * @param reason Close reason
         */
        void initiateClose(uint16_t code = 1000, const std::string& reason = "");

        /**
         * @brief Get current protocol state
         * @return Current state
         */
        State getState() const;

        /**
         * @brief Check if connection is open
         * @return true if in OPEN state
         */
        bool isOpen() const;

        /**
         * @brief Set protocol event callbacks
         * @param callbacks Callback functions
         */
        void setCallbacks(const Callbacks& callbacks);

        /**
         * @brief Get close status code
         * @return Close status code if closed, 0 otherwise
         */
        uint16_t getCloseCode() const;

        /**
         * @brief Get close reason
         * @return Close reason message
         */
        std::string getCloseReason() const;

        /**
         * @brief Reset handler for new connection
         */
        void reset();

    private:
        /**
         * @brief Process a complete WebSocket frame
         * @param frame Parsed WebSocket frame
         */
        void processFrame(const WebSocketFrame& frame);

        /**
         * @brief Handle data frame (TEXT or BINARY)
         * @param frame Data frame
         */
        void handleDataFrame(const WebSocketFrame& frame);

        /**
         * @brief Handle continuation frame
         * @param frame Continuation frame
         */
        void handleContinuationFrame(const WebSocketFrame& frame);

        /**
         * @brief Handle close frame
         * @param frame Close frame
         */
        void handleCloseFrame(const WebSocketFrame& frame);

        /**
         * @brief Handle ping frame
         * @param frame Ping frame
         */
        void handlePingFrame(const WebSocketFrame& frame);

        /**
         * @brief Handle pong frame
         * @param frame Pong frame
         */
        void handlePongFrame(const WebSocketFrame& frame);

        /**
         * @brief Validate frame according to current state
         * @param frame Frame to validate
         * @return true if frame is valid in current state
         */
        bool validateFrame(const WebSocketFrame& frame);

        /**
         * @brief Send close frame and transition to CLOSED state
         * @param code Close status code
         * @param reason Close reason
         */
        void sendClose(uint16_t code, const std::string& reason);

        /**
         * @brief Handle protocol error
         * @param error Error description
         * @param close_code Close code to use
         */
        void handleError(const std::string& error, uint16_t close_code = 1002);

        // Member variables
        State state_{ State::CONNECTING };            ///< Current protocol state
        Callbacks callbacks_;                       ///< Event callbacks
        WebSocketHandshake handshake_;              ///< Handshake processor
        WebSocketMessage current_message_;          ///< Current message being assembled
        uint16_t close_code_{ 0 };                    ///< Close status code
        std::string close_reason_;                  ///< Close reason
        Buffer read_buffer_;                        ///< Buffer for incomplete reads
        bool expecting_continuation_{ false };        ///< Waiting for continuation frame
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_PROTOCOL_HANDLER_HPP