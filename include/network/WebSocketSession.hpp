#pragma once
#ifndef WEBSOCKET_SESSION_HPP
#define WEBSOCKET_SESSION_HPP

#include "../common/Types.hpp"
#include "../protocol/WebSocketFrame.hpp"
#include "../protocol/WebSocketMessage.hpp"
#include <memory>
#include <atomic>
#include <string>

WEBSOCKET_NAMESPACE_BEGIN

// Forward declarations
class WebSocketConnection;
class SessionManager;

/**
 * @class WebSocketSession
 * @brief Represents a client WebSocket session with state management
 *
 * Handles:
 * - Session lifecycle (connecting, connected, closing, closed)
 * - Message fragmentation and reassembly
 * - Ping/Pong heartbeat mechanism
 * - Session-specific data and metadata
 */
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    using Pointer = std::shared_ptr<WebSocketSession>;

    /**
     * @brief Session states following RFC 6455
     */
    enum class State {
        CONNECTING,     ///< Handshake in progress
        CONNECTED,      ///< Active WebSocket connection
        CLOSING,        ///< Close handshake in progress
        CLOSED          ///< Connection fully closed
    };

    /**
     * @brief Construct a new WebSocket session
     * @param session_id Unique session identifier
     * @param connection Underlying network connection
     */
    WebSocketSession(ClientID session_id, std::shared_ptr<WebSocketConnection> connection);
    ~WebSocketSession();

    // Delete copy constructor and assignment operator
    WebSocketSession(const WebSocketSession&) = delete;
    WebSocketSession& operator=(const WebSocketSession&) = delete;

    /**
     * @brief Start the session (called after successful handshake)
     */
    void start();

    /**
     * @brief Gracefully close the session
     * @param code Close status code
     * @param reason Close reason message
     */
    void close(uint16_t code = 1000, const std::string& reason = "");

    /**
     * @brief Send a text message to the client
     * @param message Text message to send
     * @return true if message queued successfully
     */
    bool sendText(const std::string& message);

    /**
     * @brief Send a binary message to the client
     * @param data Binary data to send
     * @return true if message queued successfully
     */
    bool sendBinary(const Buffer& data);

    /**
     * @brief Send a ping frame to the client
     * @param data Optional ping data
     * @return true if ping sent successfully
     */
    bool sendPing(const Buffer& data = {});

    /**
     * @brief Handle incoming WebSocket frame
     * @param frame Parsed WebSocket frame
     */
    void handleFrame(const WebSocketFrame& frame);

    /**
     * @brief Get session state
     * @return Current session state
     */
    State getState() const;

    /**
     * @brief Get session ID
     * @return Unique client identifier
     */
    ClientID getId() const;

    /**
     * @brief Check if session is active
     * @return true if session is connected and active
     */
    bool isActive() const;

    /**
     * @brief Get session statistics
     * @return Session statistics structure
     */
    SessionStats getStats() const;

    /**
     * @brief Set custom session data
     * @param key Data key
     * @param value Data value
     */
    void setUserData(const std::string& key, const std::string& value);

    /**
     * @brief Get custom session data
     * @param key Data key
     * @return Data value or empty string if not found
     */
    std::string getUserData(const std::string& key) const;

private:
    /**
     * @brief Handle data frame (TEXT or BINARY)
     * @param frame Received data frame
     */
    void handleDataFrame(const WebSocketFrame& frame);

    /**
     * @brief Handle close frame
     * @param frame Received close frame
     */
    void handleCloseFrame(const WebSocketFrame& frame);

    /**
     * @brief Handle ping frame
     * @param frame Received ping frame
     */
    void handlePingFrame(const WebSocketFrame& frame);

    /**
     * @brief Handle pong frame
     * @param frame Received pong frame
     */
    void handlePongFrame(const WebSocketFrame& frame);

    /**
     * @brief Send a WebSocket frame
     * @param frame Frame to send
     * @return true if frame queued successfully
     */
    bool sendFrame(const WebSocketFrame& frame);

    /**
     * @brief Start ping timer for heartbeat
     */
    void startPingTimer();

    /**
     * @brief Handle ping timeout
     */
    void onPingTimeout();

    // Member variables
    ClientID session_id_;
    std::shared_ptr<WebSocketConnection> connection_;
    std::atomic<State> state_{ State::CONNECTING };

    // Message reassembly
    Buffer fragmented_payload_;
    bool is_fragmented_{ false };
    Opcode fragmentation_opcode_{ Opcode::CONTINUATION };

    // Session data
    std::unordered_map<std::string, std::string> user_data_;
    SessionStats stats_;

    // Timers and timeouts
    std::chrono::steady_clock::time_point last_activity_;
    std::unique_ptr<asio::steady_timer> ping_timer_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_SESSION_HPP