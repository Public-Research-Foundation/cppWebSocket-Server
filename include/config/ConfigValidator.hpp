#pragma once
#ifndef WEBSOCKET_CONFIG_VALIDATOR_HPP
#define WEBSOCKET_CONFIG_VALIDATOR_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <vector>
#include <functional>
#include <any>
#include <unordered_map>
#include <memory>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @struct ValidationRule
 * @brief Individual configuration validation rule
 */
    struct ValidationRule {
    std::string key;                            ///< Configuration key to validate
    std::string type;                           ///< Expected type ("int", "string", "bool", etc.)
    std::function<bool(const std::any&)> validator; ///< Custom validation function
    std::string errorMessage;                   ///< Error message if validation fails
    bool required{ true };                        ///< Whether key is required
    std::any defaultValue;                      ///< Default value if key is missing
};

/**
 * @class ConfigValidator
 * @brief Configuration validation system with rule-based validation
 *
 * Provides comprehensive configuration validation including:
 * - Type checking and conversion
 * - Value range validation
 * - Custom validation rules
 * - Required field checking
 * - File existence validation
 */
class ConfigValidator {
public:
    /**
     * @brief Constructor with default validation rules
     */
    ConfigValidator();

    /**
     * @brief Destructor
     */
    ~ConfigValidator() = default;

    // Delete copy and move operations
    WEBSOCKET_DISABLE_COPY(ConfigValidator)
        WEBSOCKET_DISABLE_MOVE(ConfigValidator)

        /**
         * @brief Validate configuration against all rules
         * @param config Configuration map to validate
         * @return true if configuration is valid, false otherwise
         */
        bool validate(const std::unordered_map<std::string, std::any>& config);

    /**
     * @brief Get validation errors from last validation
     * @return Vector of error messages
     */
    std::vector<std::string> getErrors() const;

    /**
     * @brief Check if last validation was successful
     * @return true if no validation errors
     */
    bool isValid() const;

    /**
     * @brief Add custom validation rule
     * @param rule Validation rule to add
     */
    void addCustomRule(const ValidationRule& rule);

    /**
     * @brief Add multiple validation rules
     * @param rules Vector of validation rules
     */
    void addCustomRules(const std::vector<ValidationRule>& rules);

    /**
     * @brief Set required configuration fields
     * @param fields Vector of required field names
     */
    void setRequiredFields(const std::vector<std::string>& fields);

    /**
     * @brief Clear all validation rules
     */
    void clearRules();

    /**
     * @brief Get number of validation rules
     * @return Rule count
     */
    size_t getRuleCount() const;

    /**
     * @brief Apply default values to configuration for missing optional fields
     * @param config Configuration to apply defaults to (modified in-place)
     */
    void applyDefaults(std::unordered_map<std::string, std::any>& config);

    /**
     * @brief Create validation rule for numeric range
     * @param key Configuration key
     * @param type Value type ("int", "double", etc.)
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param required Whether field is required
     * @return Configured validation rule
     */
    static ValidationRule createRangeRule(const std::string& key,
        const std::string& type,
        double min, double max,
        bool required = true);

    /**
     * @brief Create validation rule for string pattern
     * @param key Configuration key
     * @param pattern Regex pattern to match
     * @param required Whether field is required
     * @return Configured validation rule
     */
    static ValidationRule createPatternRule(const std::string& key,
        const std::string& pattern,
        bool required = true);

    /**
     * @brief Create validation rule for file existence
     * @param key Configuration key
     * @param required Whether field is required
     * @return Configured validation rule
     */
    static ValidationRule createFileExistsRule(const std::string& key,
        bool required = true);

    /**
     * @brief Create validation rule for directory existence
     * @param key Configuration key
     * @param required Whether field is required
     * @return Configured validation rule
     */
    static ValidationRule createDirectoryExistsRule(const std::string& key,
        bool required = true);

private:
    /**
     * @brief Validate individual configuration value against rule
     * @param rule Validation rule to apply
     * @param value Value to validate
     * @return true if validation successful
     */
    bool validateRule(const ValidationRule& rule, const std::any& value);

    /**
     * @brief Validate value type
     * @param key Configuration key
     * @param value Value to check
     * @param expectedType Expected type name
     * @return true if type matches
     */
    bool validateType(const std::string& key, const std::any& value, const std::string& expectedType) const;

    /**
     * @brief Validate numeric range
     * @param key Configuration key
     * @param value Value to check
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @return true if value within range
     */
    bool validateRange(const std::string& key, const std::any& value, double min, double max) const;

    /**
     * @brief Validate file existence
     * @param key Configuration key
     * @param value File path value
     * @return true if file exists
     */
    bool validateFileExists(const std::string& key, const std::any& value) const;

    /**
     * @brief Validate directory existence
     * @param key Configuration key
     * @param value Directory path value
     * @return true if directory exists
     */
    bool validateDirectoryExists(const std::string& key, const std::any& value) const;

    /**
     * @brief Validate string against regex pattern
     * @param key Configuration key
     * @param value String value to check
     * @param pattern Regex pattern
     * @return true if pattern matches
     */
    bool validatePattern(const std::string& key, const std::any& value, const std::string& pattern) const;

    /**
     * @brief Add validation error
     * @param error Error message to add
     */
    void addError(const std::string& error);

    /**
     * @brief Clear all validation errors
     */
    void clearErrors();

    // Member variables
    std::vector<ValidationRule> rules_;
    std::vector<std::string> requiredFields_;
    std::vector<std::string> validationErrors_;
    bool lastValidationResult_{ false };
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_CONFIG_VALIDATOR_HPP