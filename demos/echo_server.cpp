/**
 * @file echo_server.cpp
 * @brief Echo server example demonstrating message handling and broadcasting
 *
 * This example shows how to create an echo server that can:
 * - Echo messages back to individual clients
 * - Broadcast messages to all connected clients
 * - Handle different message types (text/binary)
 */

#include "api/ServerAPI.hpp"
#include "utils/Logger.hpp"
#include "common/Types.hpp"
#include <iostream>
#include <thread>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

class EchoServer {
private:
    std::shared_ptr<WebSocketServer> server_;
    std::atomic<int> messageCount_{ 0 };

public:
    EchoServer() : server_(std::make_shared<WebSocketServer>()) {
        setupHandlers();
    }

    /**
     * @brief Set up all message and event handlers
     */
    void setupHandlers() {
        // Handle incoming messages
        server_->setMessageHandler([this](ClientID clientId, const Message& message) {
            handleMessage(clientId, message);
            });

        // Handle connection events
        server_->setConnectionHandler([this](ClientID clientId) {
            onClientConnected(clientId);
            });

        // Handle disconnection events
        server_->setDisconnectionHandler([this](ClientID clientId) {
            onClientDisconnected(clientId);
            });
    }

    /**
     * @brief Process incoming messages from clients
     */
    void handleMessage(ClientID clientId, const Message& message) {
        int count = ++messageCount_;

        // Log message details
        std::cout << "Message #" << count << " from client " << clientId
            << " | Type: " << (message.isText ? "TEXT" : "BINARY")
            << " | Size: " << message.size() << " bytes" << std::endl;

        // For text messages, display content (limit to 100 chars for display)
        if (message.isText && message.size() <= 100) {
            std::string text = message.getText();
            std::cout << "Content: \"" << text << "\"" << std::endl;
        }

        // Echo the message back to the same client
        server_->sendMessage(clientId, message);

        // If message contains "broadcast", send to all clients
        if (message.isText) {
            std::string text = message.getText();
            if (text.find("broadcast") != std::string::npos) {
                broadcastMessage(message);
            }
        }
    }

    /**
     * @brief Broadcast a message to all connected clients
     */
    void broadcastMessage(const Message& message) {
        auto connections = server_->getConnectionCount();
        if (connections > 0) {
            server_->broadcast(message);
            std::cout << "Broadcasted message to " << connections << " clients" << std::endl;
        }
    }

    /**
     * @brief Handle new client connections
     */
    void onClientConnected(ClientID clientId) {
        std::cout << "Client " << clientId << " connected" << std::endl;

        // Send welcome message to new client
        Message welcomeMsg("Welcome to Echo Server! Send 'broadcast' to send to all clients.");
        server_->sendMessage(clientId, welcomeMsg);
    }

    /**
     * @brief Handle client disconnections
     */
    void onClientDisconnected(ClientID clientId) {
        std::cout << "Client " << clientId << " disconnected" << std::endl;
    }

    /**
     * @brief Start the echo server
     */
    bool start(int port = 8080) {
        // Configure server settings
        server_->setPort(port);
        server_->setMaxConnections(1000);

        std::cout << "Starting Echo Server on port " << port << "..." << std::endl;
        return server_->start();
    }

    /**
     * @brief Stop the echo server
     */
    void stop() {
        std::cout << "Stopping Echo Server..." << std::endl;
        server_->stop();
    }

    /**
     * @brief Check if server is running
     */
    bool isRunning() const {
        return server_->isRunning();
    }

    /**
     * @brief Get server statistics
     */
    void printStats() const {
        std::cout << "=== Server Statistics ===" << std::endl;
        std::cout << "Active connections: " << server_->getConnectionCount() << std::endl;
        std::cout << "Total messages processed: " << messageCount_.load() << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "=== WebSocket Echo Server Example ===" << std::endl;

    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    try {
        EchoServer echoServer;

        if (!echoServer.start(port)) {
            std::cerr << "Failed to start echo server on port " << port << std::endl;
            return 1;
        }

        std::cout << "Echo server running on port " << port << std::endl;
        std::cout << "Connect using: ws://localhost:" << port << "/" << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        // Main server loop
        while (echoServer.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(5));

            // Print statistics every 30 seconds
            static auto lastStats = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStats).count() >= 30) {
                echoServer.printStats();
                lastStats = now;
            }
        }

        echoServer.printStats();
        std::cout << "Echo server shutdown complete" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

WEBSOCKET_NAMESPACE_END