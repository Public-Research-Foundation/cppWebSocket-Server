#pragma once
#ifndef WEBSOCKET_COMMAND_LINE_PARSER_HPP
#define WEBSOCKET_COMMAND_LINE_PARSER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

WEBSOCKET_NAMESPACE_BEGIN

    /**
     * @struct CommandLineOptions
     * @brief Container for all command line options and configuration overrides
     */
    struct CommandLineOptions {
        // Configuration file options
        std::string config_file;                 ///< Path to configuration file
        std::string config_format = "auto";      ///< Config format: json, yaml, auto

        // Server options
        uint16_t port = 0;                       ///< Server port (0 = use config)
        std::string address;                     ///< Bind address (empty = use config)
        size_t max_connections = 0;              ///< Max connections (0 = use config)

        // Operation mode
        bool daemon = false;                     ///< Run as daemon process
        bool foreground = false;                 ///< Run in foreground (overrides daemon)
        bool verbose = false;                    ///< Enable verbose logging

        // Logging
        std::string log_level = "";              ///< Log level: trace, debug, info, warn, error
        std::string log_file = "";               ///< Log file path (empty = stdout)

        // Runtime configuration overrides
        std::unordered_map<std::string, std::string> config_overrides;

        /**
         * @brief Validate all options
         * @return true if options are valid, false otherwise
         */
        bool validate() const;
    };

    /**
     * @class CommandLineParser
     * @brief Parses and validates command line arguments
     *
     * Supports:
     * - Short options (-p 8080)
     * - Long options (--port 8080)
     * - Configuration overrides (--config.port=8080)
     * - Help and version information
     */
    class CommandLineParser {
    public:
        CommandLineParser() = default;
        ~CommandLineParser() = default;

        // Delete copy constructor and assignment operator
        CommandLineParser(const CommandLineParser&) = delete;
        CommandLineParser& operator=(const CommandLineParser&) = delete;

        /**
         * @brief Parse command line arguments
         * @param argc Argument count
         * @param argv Argument values
         * @return Parsed command line options
         * @throws std::invalid_argument on parsing errors
         */
        CommandLineOptions parse(int argc, char* argv[]);

        /**
         * @brief Display help information to stdout
         */
        static void showHelp();

        /**
         * @brief Display version information to stdout
         */
        static void showVersion();

        /**
         * @brief Check if help was requested
         * @param argc Argument count
         * @param argv Argument values
         * @return true if help was requested
         */
        static bool isHelpRequested(int argc, char* argv[]);

        /**
         * @brief Check if version was requested
         * @param argc Argument count
         * @param argv Argument values
         * @return true if version was requested
         */
        static bool isVersionRequested(int argc, char* argv[]);

    private:
        /**
         * @brief Validate parsed options
         * @param options Options to validate
         * @return true if options are valid, false otherwise
         */
        bool validateOptions(const CommandLineOptions& options) const;

        /**
         * @brief Parse individual argument
         * @param arg Current argument
         * @param next_arg Next argument (for values)
         * @param options Options structure to update
         * @return Number of arguments consumed (1 or 2)
         */
        int parseArgument(const std::string& arg, const char* next_arg, CommandLineOptions& options);

        /**
         * @brief Parse configuration override
         * @param arg Argument containing override (format: --key=value)
         * @param options Options structure to update
         */
        void parseConfigOverride(const std::string& arg, CommandLineOptions& options);
    };

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_COMMAND_LINE_PARSER_HPP