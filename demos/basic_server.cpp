/**
 * @file basic_server.cpp
 * @brief Minimal WebSocket server example demonstrating basic setup and operation
 *
 * This example shows the simplest way to create and run a WebSocket server
 * with default configuration and basic message handling.
 */

#include "api/ServerAPI.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

WEBSOCKET_NAMESPACE_BEGIN

int main() {
    std::cout << "=== Basic WebSocket Server Example ===" << std::endl;

    try {
        // Create server instance with default configuration
        auto server = std::make_shared<WebSocketServer>();

        // Set up message handler - echo received messages back to client
        server->setMessageHandler([](ClientID clientId, const Message& message) {
            std::cout << "Received message from client " << clientId
                << " (" << message.size() << " bytes)" << std::endl;

            // Echo the message back to the same client
            server->sendMessage(clientId, message);
            });

        // Set up connection event handlers
        server->setConnectionHandler([](ClientID clientId) {
            std::cout << "Client connected: " << clientId << std::endl;
            });

        server->setDisconnectionHandler([](ClientID clientId) {
            std::cout << "Client disconnected: " << clientId << std::endl;
            });

        // Start the server on default port (8080)
        if (!server->start()) {
            std::cerr << "Failed to start server!" << std::endl;
            return 1;
        }

        std::cout << "Server started on port 8080" << std::endl;
        std::cout << "Press Ctrl+C to stop the server..." << std::endl;

        // Keep the server running until interrupted
        while (server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Display server stats every 10 seconds
            static auto lastStats = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStats).count() >= 10) {
                std::cout << "Active connections: " << server->getConnectionCount() << std::endl;
                lastStats = now;
            }
        }

        // Server will automatically clean up when it goes out of scope
        std::cout << "Server shutdown complete" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

WEBSOCKET_NAMESPACE_END