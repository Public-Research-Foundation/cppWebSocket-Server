/**
 * @file chat_server.cpp
 * @brief Multi-client chat server example demonstrating room-based messaging
 *
 * This example implements a full chat server with:
 * - Multiple chat rooms
 * - User nicknames
 * - Join/leave notifications
 * - Private messaging
 * - User list management
 */

#include "api/ServerAPI.hpp"
#include "utils/Logger.hpp"
#include "common/Types.hpp"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <sstream>

WEBSOCKET_NAMESPACE_BEGIN

// User information structure
struct ChatUser {
    ClientID clientId;
    std::string nickname;
    std::string currentRoom;
};

// Chat room structure
struct ChatRoom {
    std::string name;
    std::unordered_set<ClientID> members;
};

class ChatServer {
private:
    std::shared_ptr<WebSocketServer> server_;
    std::mutex usersMutex_;
    std::mutex roomsMutex_;

    // User management
    std::unordered_map<ClientID, ChatUser> users_;
    std::unordered_map<std::string, ChatRoom> rooms_;

    // Server statistics
    std::atomic<int> totalMessages_{ 0 };

public:
    ChatServer() : server_(std::make_shared<WebSocketServer>()) {
        setupHandlers();
        createDefaultRooms();
    }

    /**
     * @brief Create default chat rooms
     */
    void createDefaultRooms() {
        std::lock_guard lock(roomsMutex_);
        rooms_["general"] = ChatRoom{ "general", {} };
        rooms_["random"] = ChatRoom{ "random", {} };
        rooms_["help"] = ChatRoom{ "help", {} };
    }

    /**
     * @brief Set up server event handlers
     */
    void setupHandlers() {
        server_->setMessageHandler([this](ClientID clientId, const Message& message) {
            handleMessage(clientId, message);
            });

        server_->setConnectionHandler([this](ClientID clientId) {
            onClientConnected(clientId);
            });

        server_->setDisconnectionHandler([this](ClientID clientId) {
            onClientDisconnected(clientId);
            });
    }

    /**
     * @brief Handle new client connections
     */
    void onClientConnected(ClientID clientId) {
        std::lock_guard lock(usersMutex_);

        // Create default user with anonymous nickname
        std::string nickname = "User" + std::to_string(clientId);
        users_[clientId] = ChatUser{ clientId, nickname, "general" };

        // Add to default room
        {
            std::lock_guard roomLock(roomsMutex_);
            rooms_["general"].members.insert(clientId);
        }

        // Send welcome message and room list
        sendWelcomeMessage(clientId);
        sendRoomList(clientId);

        std::cout << "User " << nickname << " (" << clientId << ") connected" << std::endl;
    }

    /**
     * @brief Handle client disconnections
     */
    void onClientDisconnected(ClientID clientId) {
        std::lock_guard lock(usersMutex_);

        auto userIt = users_.find(clientId);
        if (userIt != users_.end()) {
            std::string nickname = userIt->second.nickname;
            std::string room = userIt->second.currentRoom;

            // Remove from current room
            {
                std::lock_guard roomLock(roomsMutex_);
                auto roomIt = rooms_.find(room);
                if (roomIt != rooms_.end()) {
                    roomIt->second.members.erase(clientId);

                    // Notify room members about user leaving
                    broadcastToRoom(room, nickname + " has left the room", true);
                }
            }

            // Remove user
            users_.erase(userIt);

            std::cout << "User " << nickname << " (" << clientId << ") disconnected" << std::endl;
        }
    }

    /**
     * @brief Process incoming chat messages and commands
     */
    void handleMessage(ClientID clientId, const Message& message) {
        if (!message.isText) {
            // Ignore binary messages in chat
            return;
        }

        std::string text = message.getText();
        totalMessages_++;

        // Handle chat commands (start with '/')
        if (!text.empty() && text[0] == '/') {
            handleCommand(clientId, text);
        }
        else {
            // Handle regular chat message
            handleChatMessage(clientId, text);
        }
    }

    /**
     * @brief Process chat commands
     */
    void handleCommand(ClientID clientId, const std::string& command) {
        std::istringstream iss(command.substr(1)); // Remove leading '/'
        std::string cmd;
        iss >> cmd;

        std::lock_guard lock(usersMutex_);
        auto userIt = users_.find(clientId);
        if (userIt == users_.end()) return;

        ChatUser& user = userIt->second;

        if (cmd == "nick" || cmd == "name") {
            // Change nickname: /nick NewName
            std::string newNick;
            iss >> newNick;

            if (!newNick.empty()) {
                std::string oldNick = user.nickname;
                user.nickname = newNick;

                // Notify room about nickname change
                broadcastToRoom(user.currentRoom,
                    oldNick + " is now known as " + newNick, true);

                sendSystemMessage(clientId, "Nickname changed to: " + newNick);
            }

        }
        else if (cmd == "join") {
            // Join room: /join roomname
            std::string roomName;
            iss >> roomName;

            if (!roomName.empty()) {
                joinRoom(user, roomName);
            }

        }
        else if (cmd == "leave") {
            // Leave current room
            leaveRoom(user);

        }
        else if (cmd == "rooms") {
            // List available rooms
            sendRoomList(clientId);

        }
        else if (cmd == "users" || cmd == "list") {
            // List users in current room
            sendUserList(clientId, user.currentRoom);

        }
        else if (cmd == "msg" || cmd == "whisper") {
            // Private message: /msg username message
            std::string targetNick, message;
            iss >> targetNick;
            std::getline(iss, message);

            if (!targetNick.empty() && !message.empty()) {
                sendPrivateMessage(user, targetNick, message);
            }

        }
        else if (cmd == "help") {
            // Show help
            sendHelp(clientId);

        }
        else {
            sendSystemMessage(clientId, "Unknown command: /" + cmd);
        }
    }

    /**
     * @brief Handle regular chat messages
     */
    void handleChatMessage(ClientID clientId, const std::string& text) {
        std::lock_guard lock(usersMutex_);
        auto userIt = users_.find(clientId);
        if (userIt == users_.end()) return;

        ChatUser& user = userIt->second;

        // Format message: "Nickname: message"
        std::string formattedMessage = user.nickname + ": " + text;

        // Broadcast to current room
        broadcastToRoom(user.currentRoom, formattedMessage, false);

        std::cout << "[" << user.currentRoom << "] " << formattedMessage << std::endl;
    }

    /**
     * @brief Join a chat room
     */
    void joinRoom(ChatUser& user, const std::string& roomName) {
        std::lock_guard roomLock(roomsMutex_);

        // Find or create room
        auto roomIt = rooms_.find(roomName);
        if (roomIt == rooms_.end()) {
            // Create new room
            rooms_[roomName] = ChatRoom{ roomName, {} };
            roomIt = rooms_.find(roomName);
        }

        ChatRoom& newRoom = roomIt->second;
        ChatRoom& oldRoom = rooms_[user.currentRoom];

        // Leave old room if different
        if (user.currentRoom != roomName) {
            oldRoom.members.erase(user.clientId);
            broadcastToRoom(user.currentRoom,
                user.nickname + " has left the room", true);
        }

        // Join new room
        user.currentRoom = roomName;
        newRoom.members.insert(user.clientId);

        // Notify new room and user
        broadcastToRoom(roomName, user.nickname + " has joined the room", true);
        sendSystemMessage(user.clientId, "Joined room: " + roomName);
        sendUserList(user.clientId, roomName);
    }

    /**
     * @brief Leave current room and join general
     */
    void leaveRoom(ChatUser& user) {
        if (user.currentRoom == "general") {
            sendSystemMessage(user.clientId, "You cannot leave the general room");
            return;
        }

        joinRoom(user, "general");
    }

    /**
     * @brief Send welcome message to new user
     */
    void sendWelcomeMessage(ClientID clientId) {
        std::string welcome = R"(
Welcome to the Chat Server!

Available commands:
/nick <name>    - Change your nickname
/join <room>    - Join a chat room
/leave          - Leave current room
/rooms          - List available rooms
/users          - List users in current room
/msg <user> <msg> - Send private message
/help           - Show this help

You are in the 'general' room. Type /join roomname to switch rooms.
)";

        sendSystemMessage(clientId, welcome);
    }

    /**
     * @brief Send help information
     */
    void sendHelp(ClientID clientId) {
        std::string help = R"(
Chat Server Commands:
/nick <name>    - Change your nickname
/join <room>    - Join a chat room (creates if new)
/leave          - Leave current room (returns to general)
/rooms          - List all available rooms
/users          - List users in your current room
/msg <user> <msg> - Send private message to user
/help           - Show this help message
)";

        sendSystemMessage(clientId, help);
    }

    /**
     * @brief Send list of available rooms
     */
    void sendRoomList(ClientID clientId) {
        std::lock_guard lock(roomsMutex_);

        std::stringstream ss;
        ss << "Available rooms (" << rooms_.size() << "):\n";

        for (const auto& [name, room] : rooms_) {
            ss << "  " << name << " (" << room.members.size() << " users)\n";
        }

        ss << "Use /join <roomname> to join a room";
        sendSystemMessage(clientId, ss.str());
    }

    /**
     * @brief Send list of users in a room
     */
    void sendUserList(ClientID clientId, const std::string& roomName) {
        std::lock_guard lock(roomsMutex_);
        std::lock_guard userLock(usersMutex_);

        auto roomIt = rooms_.find(roomName);
        if (roomIt == rooms_.end()) {
            sendSystemMessage(clientId, "Room not found: " + roomName);
            return;
        }

        const ChatRoom& room = roomIt->second;
        std::stringstream ss;
        ss << "Users in " << roomName << " (" << room.members.size() << "):\n";

        for (ClientID memberId : room.members) {
            auto userIt = users_.find(memberId);
            if (userIt != users_.end()) {
                ss << "  " << userIt->second.nickname;
                if (memberId == clientId) {
                    ss << " (you)";
                }
                ss << "\n";
            }
        }

        sendSystemMessage(clientId, ss.str());
    }

    /**
     * @brief Send private message to specific user
     */
    void sendPrivateMessage(const ChatUser& fromUser, const std::string& targetNick,
        const std::string& message) {
        std::lock_guard lock(usersMutex_);

        // Find target user by nickname
        ClientID targetId = 0;
        for (const auto& [id, user] : users_) {
            if (user.nickname == targetNick) {
                targetId = id;
                break;
            }
        }

        if (targetId == 0) {
            sendSystemMessage(fromUser.clientId, "User not found: " + targetNick);
            return;
        }

        if (targetId == fromUser.clientId) {
            sendSystemMessage(fromUser.clientId, "You cannot message yourself");
            return;
        }

        // Send private message
        std::string privateMsg = "[PM from " + fromUser.nickname + "] " + message;
        sendSystemMessage(targetId, privateMsg);
        sendSystemMessage(fromUser.clientId, "[PM to " + targetNick + "] " + message);
    }

    /**
     * @brief Broadcast message to all users in a room
     */
    void broadcastToRoom(const std::string& roomName, const std::string& message, bool isSystem) {
        std::lock_guard lock(roomsMutex_);

        auto roomIt = rooms_.find(roomName);
        if (roomIt == rooms_.end()) return;

        const ChatRoom& room = roomIt->second;
        Message msg(message);

        for (ClientID memberId : room.members) {
            server_->sendMessage(memberId, msg);
        }
    }

    /**
     * @brief Send system message to specific client
     */
    void sendSystemMessage(ClientID clientId, const std::string& message) {
        Message msg("[System] " + message);
        server_->sendMessage(clientId, msg);
    }

    /**
     * @brief Start the chat server
     */
    bool start(int port = 8080) {
        server_->setPort(port);
        server_->setMaxConnections(500);

        std::cout << "Starting Chat Server on port " << port << "..." << std::endl;
        return server_->start();
    }

    /**
     * @brief Stop the chat server
     */
    void stop() {
        std::cout << "Stopping Chat Server..." << std::endl;

        // Notify all users
        broadcastToRoom("general", "Server is shutting down. Goodbye!", true);

        server_->stop();
    }

    /**
     * @brief Print server statistics
     */
    void printStats() const {
        std::lock_guard lock(usersMutex_);
        std::lock_guard roomLock(roomsMutex_);

        std::cout << "=== Chat Server Statistics ===" << std::endl;
        std::cout << "Active users: " << users_.size() << std::endl;
        std::cout << "Active rooms: " << rooms_.size() << std::endl;
        std::cout << "Total messages: " << totalMessages_.load() << std::endl;

        for (const auto& [name, room] : rooms_) {
            std::cout << "  Room " << name << ": " << room.members.size() << " users" << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    std::cout << "=== WebSocket Chat Server Example ===" << std::endl;

    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    try {
        ChatServer chatServer;

        if (!chatServer.start(port)) {
            std::cerr << "Failed to start chat server on port " << port << std::endl;
            return 1;
        }

        std::cout << "Chat server running on port " << port << std::endl;
        std::cout << "Connect using: ws://localhost:" << port << "/" << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        // Main server loop
        while (chatServer.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            chatServer.printStats();
        }

        std::cout << "Chat server shutdown complete" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

WEBSOCKET_NAMESPACE_END