#include "main/Application.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <cstdlib>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * WebSocket Server Main Entry Point
 * Handles application lifecycle and exception safety
 */
    int main(int argc, char* argv[]) {
    try {
        // Get application instance
        auto& app = Application::getInstance();

        // Initialize application
        Result result = app.initialize(argc, argv);
        if (result != Result::SUCCESS) {
            std::cerr << "Failed to initialize application" << std::endl;
            return EXIT_FAILURE;
        }

        // Run application
        result = app.run();
        if (result != Result::SUCCESS) {
            std::cerr << "Application run failed" << std::endl;
            return EXIT_FAILURE;
        }

        LOG_INFO("WebSocket Server terminated successfully");
        return EXIT_SUCCESS;

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        LOG_FATAL("Unhandled exception: {}", e.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Fatal error: Unknown exception" << std::endl;
        LOG_FATAL("Unknown exception occurred");
        return EXIT_FAILURE;
    }
}

WEBSOCKET_NAMESPACE_END