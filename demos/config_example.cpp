/**
 * @file config_example.cpp
 * @brief Configuration usage example demonstrating various configuration methods
 *
 * This example shows different ways to configure the WebSocket server:
 * - Programmatic configuration
 * - JSON configuration files
 * - Runtime configuration overrides
 * - Environment variable support
 */

#include "api/ServerAPI.hpp"
#include "config/ConfigManager.hpp"
#include "config/RuntimeConfig.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <cstdlib>

WEBSOCKET_NAMESPACE_BEGIN

int main(int argc, char* argv[]) {
    std::cout << "=== WebSocket Server Configuration Example ===" << std::endl;

    try {
        // Method 1: Programmatic configuration
        std::cout << "\n1. Programmatic Configuration:" << std::endl;
        auto server1 = std::make_shared<WebSocketServer>();

        // Set configuration directly
        server1->setPort(8081);
        server1->setMaxConnections(1000);
        server1->setMaxMessageSize(16 * 1024 * 1024); // 16MB
        server1->enableCompression(true);

        std::cout << "   Port: " << 8081 << std::endl;
        std::cout << "   Max connections: " << 1000 << std::endl;
        std::cout << "   Max message size: 16MB" << std::endl;
        std::cout << "   Compression: enabled" << std::endl;

        // Method 2: JSON configuration file
        std::cout << "\n2. JSON Configuration File:" << std::endl;
        auto configManager = std::make_shared<ConfigManager>();

        // Example JSON configuration
        std::string jsonConfig = R"({
            "server": {
                "port": 8082,
                "thread_pool_size": 8,
                "max_connections": 2000
            },
            "performance": {
                "max_message_size": 16777216,
                "buffer_size": 8192,
                "compression_enabled": true
            },
            "security": {
                "ssl_enabled": false
            },
            "logging": {
                "level": "INFO",
                "file": "server.log"
            }
        })";

        if (configManager->loadFromString(jsonConfig, "json")) {
            std::cout << "   JSON configuration loaded successfully" << std::endl;

            // Create server with JSON configuration
            auto server2 = std::make_shared<WebSocketServer>();
            // server2 would use the loaded configuration

            // Access configuration values
            int port = configManager->get<int>("server.port", 8080);
            int maxConnections = configManager->get<int>("server.max_connections", 1000);
            std::string logLevel = configManager->get<std::string>("logging.level", "INFO");

            std::cout << "   Port: " << port << std::endl;
            std::cout << "   Max connections: " << maxConnections << std::endl;
            std::cout << "   Log level: " << logLevel << std::endl;
        }

        // Method 3: Runtime configuration overrides
        std::cout << "\n3. Runtime Configuration Overrides:" << std::endl;
        auto& runtimeConfig = RuntimeConfig::getInstance();

        // Set runtime overrides
        runtimeConfig.setPort(8083);
        runtimeConfig.setMaxConnections(1500);
        runtimeConfig.setLogLevel("DEBUG");

        std::cout << "   Runtime port: " << runtimeConfig.getPort() << std::endl;
        std::cout << "   Runtime max connections: " << runtimeConfig.getMaxConnections() << std::endl;
        std::cout << "   Runtime log level: " << runtimeConfig.getLogLevel() << std::endl;

        // Method 4: Environment variables
        std::cout << "\n4. Environment Variables:" << std::endl;

        // Set environment variables (in real usage, these would be set externally)
        setenv("WEBSOCKET_PORT", "8084", 1);
        setenv("WEBSOCKET_MAX_CONNECTIONS", "3000", 1);
        setenv("WEBSOCKET_LOG_LEVEL", "WARN", 1);

        // Read environment variables with fallbacks
        const char* envPort = std::getenv("WEBSOCKET_PORT");
        const char* envMaxConn = std::getenv("WEBSOCKET_MAX_CONNECTIONS");
        const char* envLogLevel = std::getenv("WEBSOCKET_LOG_LEVEL");

        int envPortValue = envPort ? std::stoi(envPort) : 8080;
        int envMaxConnValue = envMaxConn ? std::stoi(envMaxConn) : 1000;
        std::string envLogLevelValue = envLogLevel ? envLogLevel : "INFO";

        std::cout << "   Environment port: " << envPortValue << std::endl;
        std::cout << "   Environment max connections: " << envMaxConnValue << std::endl;
        std::cout << "   Environment log level: " << envLogLevelValue << std::endl;

        // Method 5: Command line arguments simulation
        std::cout << "\n5. Command Line Configuration:" << std::endl;

        // Simulate command line parsing
        std::unordered_map<std::string, std::any> cmdLineOverrides;
        cmdLineOverrides["server.port"] = 8085;
        cmdLineOverrides["server.max_connections"] = 2500;
        cmdLineOverrides["logging.level"] = std::string("ERROR");

        // Apply command line overrides to configuration
        for (const auto& [key, value] : cmdLineOverrides) {
            configManager->setRuntimeOverride(key, value);
        }

        std::cout << "   Command line port: " << configManager->get<int>("server.port") << std::endl;
        std::cout << "   Command line max connections: "
            << configManager->get<int>("server.max_connections") << std::endl;
        std::cout << "   Command line log level: "
            << configManager->get<std::string>("logging.level") << std::endl;

        // Method 6: Configuration validation
        std::cout << "\n6. Configuration Validation:" << std::endl;

        if (configManager->validate()) {
            std::cout << "   Configuration validation: PASSED" << std::endl;
        }
        else {
            auto errors = configManager->getValidationErrors();
            std::cout << "   Configuration validation: FAILED" << std::endl;
            for (const auto& error : errors) {
                std::cout << "     - " << error << std::endl;
            }
        }

        // Method 7: Hot-reload demonstration
        std::cout << "\n7. Configuration Hot-Reload:" << std::endl;

        std::cout << "   Initial configuration loaded" << std::endl;

        // Simulate configuration change
        configManager->set("server.port", 9090);
        configManager->set("logging.level", std::string("DEBUG"));

        std::cout << "   Hot-reloaded port: " << configManager->get<int>("server.port") << std::endl;
        std::cout << "   Hot-reloaded log level: " << configManager->get<std::string>("logging.level") << std::endl;

        // Export current configuration
        std::string exportedConfig = configManager->exportToString("json");
        std::cout << "   Exported configuration size: " << exportedConfig.size() << " bytes" << std::endl;

        std::cout << "\nConfiguration example completed successfully!" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

WEBSOCKET_NAMESPACE_END