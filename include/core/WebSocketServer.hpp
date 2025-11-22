#pragma once
#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include "../common/Types.hpp"
#include "../common/NonCopyable.hpp"
#include "../config/ServerConfig.hpp"
#include "Engine.hpp"
#include "ServiceLocator.hpp"
#include <memory>
#include <functional>
#include <atomic>
#include <unordered_map>

WEBSOCKET_NAMESPACE_BEGIN

// Forward declarations
class WebSocketSession;
class ProtocolHandler;
class IOThreadPool;

/**
 * @class WebSocketServer
 * @brief Main WebSocket server class providing high-level API for applications
 *
 * This class serves as the primary interface for users to create and manage
 * WebSocket servers. It orchestrates all underlying components including
 * networking, protocol handling, session management, and configuration.
 *
 * Features:
 * - High-performance async I/O
 * - Configurable connection limits
 * - Session lifecycle management
 * - Event-driven architecture
 * - Graceful shutdown support
 */
class WebSocketServer : public NonCopyable {
public:
    /**
     * @brief Event callback types
     */
    using MessageHandler = std::function<void(ClientID, const Message&)>;
    using ConnectionHandler = std::function<void(ClientID)>;
    using ErrorHandler = std::function<void(ClientID, const std::string&)>;

    /**
     * @brief Server statistics
     */
    struct Statistics {
        size_t active_connections{ 0 };        ///< Currently active connections
        size_t total_connections{ 0 };         ///< Total connections since start
        size_t messages_received{ 0 };         ///< Total messages received
        size_t messages_sent{ 0 };             ///< Total messages sent
        size_t bytes_received{ 0 };            ///< Total bytes received
        size_t bytes_sent{ 0 };                ///< Total bytes sent
        size_t connection_errors{ 0 };         ///< Total connection errors
    };

    /**
     * @brief Default constructor with default configuration
     */
    WebSocketServer();

    /**
     * @brief Constructor with custom configuration
     * @param config Server configuration parameters
     */
    explicit WebSocketServer(const ServerConfig& config);

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~WebSocketServer();

    /**
     * @brief Start the WebSocket server
     * @return true if server started successfully, false otherwise
     *
     * @note Starts listening on configured port and begins accepting connections
     */
    bool start();

    /**
     * @brief Start server on specific port (overrides config)
     * @param port Port number to listen on
     * @return true if server started successfully
     */
    bool start(uint16_t port);

    /**
     * @brief Stop the server gracefully
     *
     * @note Waits for active connections to complete and stops accepting new ones
     */
    void stop();

    /**
     * @brief Stop server immediately (forceful shutdown)
     *
     * @note Drops all active connections without waiting
     */
    void stopNow();

    /**
     * @brief Check if server is running
     * @return true if server is active and accepting connections
     */
    bool isRunning() const;

    /**
     * @brief Send message to specific client
     * @param client_id Target client identifier
     * @param message Message to send
     * @return true if message queued successfully, false if client not found
     */
    bool send(ClientID client_id, const Message& message);

    /**
     * @brief Send text message to specific client
     * @param client_id Target client identifier
     * @param text Text message to send
     * @return true if message queued successfully
     */
    bool sendText(ClientID client_id, const std::string& text);

    /**
     * @brief Send binary data to specific client
     * @param client_id Target client identifier
     * @param data Binary data to send
     * @return true if message queued successfully
     */
    bool sendBinary(ClientID client_id, const Buffer& data);

    /**
     * @brief Broadcast message to all connected clients
     * @param message Message to broadcast
     * @return Number of clients the message was sent to
     */
    size_t broadcast(const Message& message);

    /**
     * @brief Broadcast text message to all clients
     * @param text Text message to broadcast
     * @return Number of clients the message was sent to
     */
    size_t broadcastText(const std::string& text);

    /**
     * @brief Broadcast binary data to all clients
     * @param data Binary data to broadcast
     * @return Number of clients the message was sent to
     */
    size_t broadcastBinary(const Buffer& data);

    /**
     * @brief Close connection with specific client
     * @param client_id Client identifier to close
     * @param code WebSocket close status code
     * @param reason Close reason message
     * @return true if close initiated successfully
     */
    bool close(ClientID client_id, uint16_t code = 1000, const std::string& reason = "");

    /**
     * @brief Close all connections gracefully
     * @param code WebSocket close status code
     * @param reason Close reason message
     */
    void closeAll(uint16_t code = 1000, const std::string& reason = "");

    /**
     * @brief Set message received event handler
     * @param handler Callback function for incoming messages
     */
    void onMessage(MessageHandler handler);

    /**
     * @brief Set client connection event handler
     * @param handler Callback function for new connections
     */
    void onConnect(ConnectionHandler handler);

    /**
     * @brief Set client disconnection event handler
     * @param handler Callback function for disconnections
     */
    void onDisconnect(ConnectionHandler handler);

    /**
     * @brief Set error event handler
     * @param handler Callback function for errors
     */
    void onError(ErrorHandler handler);

    /**
     * @brief Get server statistics
     * @return Current server statistics
     */
    Statistics getStatistics() const;

    /**
     * @brief Get current server configuration
     * @return Active server configuration
     */
    ServerConfig getConfig() const;

    /**
     * @brief Update server configuration (may require restart)
     * @param config New server configuration
     * @return true if configuration applied successfully
     */
    bool setConfig(const ServerConfig& config);

    /**
     * @brief Get list of currently connected client IDs
     * @return Vector of active client identifiers
     */
    std::vector<ClientID> getConnectedClients() const;

    /**
     * @brief Check if specific client is connected
     * @param client_id Client identifier to check
     * @return true if client is currently connected
     */
    bool isClientConnected(ClientID client_id) const;

    /**
     * @brief Wait for server to stop (blocking call)
     *
     * @note Useful for main thread in server applications
     */
    void waitForStop();

private:
    /**
     * @brief Initialize server components
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Cleanup server resources
     */
    void cleanup();

    /**
     * @brief Handle new client connection
     * @param session New client session
     */
    void handleNewConnection(std::shared_ptr<WebSocketSession> session);

    /**
     * @brief Handle client disconnection
     * @param client_id Disconnected client identifier
     */
    void handleClientDisconnect(ClientID client_id);

    /**
     * @brief Handle incoming message from client
     * @param client_id Source client identifier
     * @param message Received message
     */
    void handleClientMessage(ClientID client_id, const Message& message);

    /**
     * @brief Handle client error
     * @param client_id Client identifier with error
     * @param error Error description
     */
    void handleClientError(ClientID client_id, const std::string& error);

    // Member variables
    std::unique_ptr<class WebSocketServerImpl> impl_;  ///< Pimpl pattern implementation
    std::atomic<bool> running_{ false };                 ///< Server running state

    // Event handlers
    MessageHandler message_handler_;
    ConnectionHandler connect_handler_;
    ConnectionHandler disconnect_handler_;
    ErrorHandler error_handler_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_SERVER_HPP