#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <any>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Command Line Option Structure
 * Defines a single command line option with its properties
 */
    struct CommandLineOption {
    std::string longName;        // Long option name (e.g., "help")
    char shortName;              // Short option character (e.g., 'h')
    std::string description;     // Help description
    std::string valueName;       // Name for value in help (e.g., "FILE")
    bool requiresValue;          // Whether option requires a value
    std::any defaultValue;       // Default value if not provided
    bool isSet;                  // Whether option was provided
    std::any value;              // Actual value provided
};

/**
 * Command Line Parser - Comprehensive CLI argument processing
 *
 * PURPOSE:
 * - Parse command line arguments according to defined options
 * - Generate help and usage information
 * - Validate argument combinations and requirements
 * - Convert CLI arguments to configuration values
 *
 * FEATURES:
 * - Support for both long (--help) and short (-h) options
 * - Automatic type conversion and validation
 * - Positional argument support
 * - Help text generation
 * - Configuration export
 */
class CommandLineParser {
public:
    CommandLineParser();

    // ===== OPTION REGISTRATION =====

    /**
     * Add a command line option with full specification
     *
     * @param longName Long option name (e.g., "config")
     * @param shortName Short option character (e.g., 'c')
     * @param description Help text description
     * @param valueName Name for value in help display
     * @param requiresValue Whether option requires a value
     * @param defaultValue Default value if not provided
     */
    void addOption(const std::string& longName, char shortName,
        const std::string& description,
        const std::string& valueName = "",
        bool requiresValue = false,
        const std::any& defaultValue = std::any());

    /**
     * Add a boolean flag option (no value required)
     */
    void addFlag(const std::string& longName, char shortName,
        const std::string& description);

    // ===== PARSING OPERATIONS =====

    /**
     * Parse command line arguments from main() parameters
     *
     * @param argc Argument count from main()
     * @param argv Argument values from main()
     * @return Result indicating parsing success
     */
    Result parse(int argc, char* argv[]);

    /**
     * Parse command line arguments from string vector
     * Alternative interface for testing or programmatic use
     */
    Result parse(const std::vector<std::string>& args);

    // ===== OPTION ACCESS =====

    bool hasOption(const std::string& longName) const;
    bool hasOption(char shortName) const;

    /**
     * Get option value with type safety
     *
     * @tparam T Expected value type
     * @param longName Option long name
     * @param defaultValue Value to return if option not set
     * @return Option value or default
     */
    template<typename T>
    T getOptionValue(const std::string& longName, const T& defaultValue = T()) const;

    template<typename T>
    T getOptionValue(char shortName, const T& defaultValue = T()) const;

    // ===== UTILITY METHODS =====

    std::string getExecutableName() const;
    std::vector<std::string> getPositionalArguments() const;
    std::vector<std::string> getUnknownOptions() const;

    // ===== HELP AND USAGE GENERATION =====

    void showHelp() const;
    void showUsage() const;
    std::string generateHelpText() const;
    std::string generateUsageText() const;

    // ===== VALIDATION =====

    bool validate() const;
    std::vector<std::string> getValidationErrors() const;

    // ===== CONFIGURATION GENERATION =====

    /**
     * Generate configuration map from parsed options
     * Converts command line options to configuration key-value pairs
     */
    std::unordered_map<std::string, std::any> generateConfig() const;

private:
    std::unordered_map<std::string, CommandLineOption> options_;
    std::unordered_map<char, std::string> shortToLongMap_;
    std::vector<std::string> positionalArgs_;
    std::vector<std::string> unknownOptions_;
    std::string executableName_;

    mutable std::vector<std::string> validationErrors_;

    void initializeDefaultOptions();
    Result processOption(const std::string& arg, const std::string& nextArg, size_t& index, const std::vector<std::string>& args);
    Result processLongOption(const std::string& arg, const std::string& nextArg, size_t& index, const std::vector<std::string>& args);
    Result processShortOption(const std::string& arg, const std::string& nextArg, size_t& index, const std::vector<std::string>& args);

    CommandLineOption* findOption(const std::string& longName);
    const CommandLineOption* findOption(const std::string& longName) const;
    CommandLineOption* findOptionByShortName(char shortName);
    const CommandLineOption* findOptionByShortName(char shortName) const;

    std::string extractOptionName(const std::string& arg) const;
    std::string extractOptionValue(const std::string& arg) const;

    void addValidationError(const std::string& error);
};

// Template implementations
template<typename T>
T CommandLineParser::getOptionValue(const std::string& longName, const T& defaultValue) const {
    const CommandLineOption* option = findOption(longName);
    if (!option || !option->isSet) {
        return defaultValue;
    }

    try {
        return std::any_cast<T>(option->value);
    }
    catch (const std::bad_any_cast&) {
        return defaultValue;
    }
}

template<typename T>
T CommandLineParser::getOptionValue(char shortName, const T& defaultValue) const {
    const CommandLineOption* option = findOptionByShortName(shortName);
    if (!option || !option->isSet) {
        return defaultValue;
    }

    try {
        return std::any_cast<T>(option->value);
    }
    catch (const std::bad_any_cast&) {
        return defaultValue;
    }
}

WEBSOCKET_NAMESPACE_END