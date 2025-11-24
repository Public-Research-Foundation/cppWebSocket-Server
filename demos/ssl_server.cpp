/**
 * @file ssl_server.cpp
 * @brief SSL/TLS secured WebSocket server example
 *
 * This example demonstrates how to set up a secure WebSocket server (wss://)
 * with SSL/TLS encryption, certificate handling, and secure configuration.
 */

#include "api/ServerAPI.hpp"
#include "config/ConfigManager.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <thread>

WEBSOCKET_NAMESPACE_BEGIN

class SecureWebSocketServer {
private:
    std::shared_ptr<WebSocketServer> server_;

public:
    SecureWebSocketServer() : server_(std::make_shared<WebSocketServer>()) {
        setupHandlers();
    }

    /**
     * @brief Set up server event handlers
     */
    void setupHandlers() {
        server_->setMessageHandler([this](ClientID clientId, const Message& message) {
            handleSecureMessage(clientId, message);
            });

        server_->setConnectionHandler([this](ClientID clientId) {
            onSecureClientConnected(clientId);
            });

        server_->setDisconnectionHandler([this](ClientID clientId) {
            onSecureClientDisconnected(clientId);
            });
    }

    /**
     * @brief Handle messages from secure connections
     */
    void handleSecureMessage(ClientID clientId, const Message& message) {
        if (message.isText) {
            std::string text = message.getText();
            std::cout << "[Secure] Message from client " << clientId << ": " << text << std::endl;

            // Echo back with secure prefix
            Message response("[Secure Echo] " + text);
            server_->sendMessage(clientId, response);

            // If message is "status", send connection info
            if (text == "status") {
                sendConnectionInfo(clientId);
            }
        }
        else {
            // Handle binary data
            std::cout << "[Secure] Binary data from client " << clientId
                << " (" << message.size() << " bytes)" << std::endl;

            // Echo binary data back
            server_->sendMessage(clientId, message);
        }
    }

    /**
     * @brief Handle new secure connections
     */
    void onSecureClientConnected(ClientID clientId) {
        std::cout << "[Secure] Client connected: " << clientId << std::endl;

        // Send welcome message
        Message welcome(R"({
            "type": "welcome",
            "message": "Connected to secure WebSocket server",
            "clientId": )" + std::to_string(clientId) + R"(,
            "protocol": "wss",
            "timestamp": ")" + getCurrentTimestamp() + R"("
        })");

        server_->sendMessage(clientId, welcome);
    }

    /**
     * @brief Handle secure client disconnections
     */
    void onSecureClientDis