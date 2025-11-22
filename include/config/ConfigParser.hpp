#pragma once
#ifndef WEBSOCKET_CONFIG_PARSER_HPP
#define WEBSOCKET_CONFIG_PARSER_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <unordered_map>
#include <any>
#include <memory>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class ConfigParser
 * @brief Abstract base class for configuration format parsers
 *
 * Supports multiple configuration formats through plugin architecture.
 * Implementations handle specific format parsing and serialization.
 */
    class ConfigParser {
    public:
        virtual ~ConfigParser() = default;

        /**
         * @brief Parse configuration data into key-value map
         * @param data Configuration data as string
         * @param output Output map to populate with parsed values
         * @return true if parsing successful, false otherwise
         */
        virtual bool parse(const std::string& data,
            std::unordered_map<std::string, std::any>& output) = 0;

        /**
         * @brief Serialize configuration map to string
         * @param config Configuration map to serialize
         * @return Serialized configuration data
         */
        virtual std::string serialize(const std::unordered_map<std::string, std::any>& config) = 0;

        /**
         * @brief Get parser format name
         * @return Format identifier (e.g., "json", "yaml")
         */
        virtual std::string getFormatName() const = 0;

        /**
         * @brief Check if parser supports given data
         * @param data Configuration data to check
         * @return true if data appears to be in supported format
         */
        virtual bool supports(const std::string& data) const = 0;
};

/**
 * @class JsonConfigParser
 * @brief JSON format configuration parser
 *
 * Parses JSON configuration files with support for:
 * - Basic types (string, number, boolean, null)
 * - Nested objects (converted to dot notation)
 * - Arrays (supported as vector<any>)
 */
class JsonConfigParser : public ConfigParser {
public:
    /**
     * @brief Parse JSON configuration data
     * @param data JSON configuration string
     * @param output Map to populate with parsed values
     * @return true if parsing successful
     */
    bool parse(const std::string& data,
        std::unordered_map<std::string, std::any>& output) override;

    /**
     * @brief Serialize configuration to JSON string
     * @param config Configuration map to serialize
     * @return JSON string representation
     */
    std::string serialize(const std::unordered_map<std::string, std::any>& config) override;

    /**
     * @brief Get format name
     * @return "json"
     */
    std::string getFormatName() const override { return "json"; }

    /**
     * @brief Check if data appears to be JSON
     * @param data Data to check
     * @return true if data starts with { or [
     */
    bool supports(const std::string& data) const override;

private:
    /**
     * @brief Flatten nested JSON object into dot notation
     * @param currentKey Current key prefix
     * @param value Current JSON value
     * @param output Output map to populate
     */
    void flattenJsonObject(const std::string& currentKey,
        const std::any& value,
        std::unordered_map<std::string, std::any>& output);

    /**
     * @brief Convert JSON value to appropriate C++ type
     * @param jsonValue JSON value to convert
     * @return C++ any value with appropriate type
     */
    std::any convertJsonValue(const std::any& jsonValue);
};

/**
 * @class YamlConfigParser
 * @brief YAML format configuration parser
 *
 * Parses YAML configuration files with support for:
 * - Basic YAML types and structures
 * - Nested mappings and sequences
 * - YAML anchors and references
 */
class YamlConfigParser : public ConfigParser {
public:
    /**
     * @brief Parse YAML configuration data
     * @param data YAML configuration string
     * @param output Map to populate with parsed values
     * @return true if parsing successful
     */
    bool parse(const std::string& data,
        std::unordered_map<std::string, std::any>& output) override;

    /**
     * @brief Serialize configuration to YAML string
     * @param config Configuration map to serialize
     * @return YAML string representation
     */
    std::string serialize(const std::unordered_map<std::string, std::any>& config) override;

    /**
     * @brief Get format name
     * @return "yaml"
     */
    std::string getFormatName() const override { return "yaml"; }

    /**
     * @brief Check if data appears to be YAML
     * @param data Data to check
     * @return true if data contains YAML indicators
     */
    bool supports(const std::string& data) const override;

private:
    /**
     * @brief Flatten nested YAML structure into dot notation
     * @param currentKey Current key prefix
     * @param value Current YAML value
     * @param output Output map to populate
     */
    void flattenYamlStructure(const std::string& currentKey,
        const std::any& value,
        std::unordered_map<std::string, std::any>& output);
};

/**
 * @class ConfigParserFactory
 * @brief Factory for creating configuration parsers
 */
class ConfigParserFactory {
public:
    /**
     * @brief Create parser for specific format
     * @param format Format name ("json", "yaml")
     * @return Unique pointer to parser instance
     */
    static std::unique_ptr<ConfigParser> createParser(const std::string& format);

    /**
     * @brief Create parser that auto-detects format from data
     * @param data Configuration data to analyze
     * @return Unique pointer to appropriate parser
     */
    static std::unique_ptr<ConfigParser> createAutoParser(const std::string& data);

    /**
     * @brief Get list of supported formats
     * @return Vector of supported format names
     */
    static std::vector<std::string> getSupportedFormats();
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_CONFIG_PARSER_HPP