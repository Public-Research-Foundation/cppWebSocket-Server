#pragma once
#ifndef WEBSOCKET_STRING_UTILS_HPP
#define WEBSOCKET_STRING_UTILS_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <random>
#include <functional>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class StringUtils
 * @brief Comprehensive string manipulation utilities
 *
 * Collection of commonly needed string operations.
 * All methods are static and thread-safe.
 *
 * Features:
 * - String splitting and joining
 * - Case conversion and trimming
 * - Search and replace operations
 * - Encoding/decoding utilities
 * - Validation and formatting
 * - Type conversion with error handling
 */
    class StringUtils {
    public:
        // ===== STRING SPLITTING AND JOINING =====

        /**
         * @brief Split string by single character delimiter
         * @param str String to split
         * @param delimiter Character to split on
         * @param skipEmpty Whether to skip empty substrings (default: false)
         * @return Vector of substrings
         */
        static std::vector<std::string> split(const std::string& str, char delimiter, bool skipEmpty = false);

        /**
         * @brief Split string by string delimiter
         * @param str String to split
         * @param delimiter String to split on
         * @param skipEmpty Whether to skip empty substrings (default: false)
         * @return Vector of substrings
         */
        static std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool skipEmpty = false);

        /**
         * @brief Split string by whitespace
         * @param str String to split
         * @return Vector of non-whitespace substrings
         */
        static std::vector<std::string> splitWhitespace(const std::string& str);

        /**
         * @brief Join vector of strings with delimiter
         * @param strings Strings to join
         * @param delimiter Delimiter to insert between strings
         * @return Joined string
         */
        static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

        /**
         * @brief Join range of strings with delimiter
         * @tparam InputIterator Iterator type
         * @param begin Start iterator
         * @param end End iterator
         * @param delimiter Delimiter string
         * @return Joined string
         */
        template<typename InputIterator>
        static std::string join(InputIterator begin, InputIterator end, const std::string& delimiter);

        // ===== CASE CONVERSION =====

        /**
         * @brief Convert string to lowercase
         * @param str String to convert
         * @return Lowercase string
         */
        static std::string toLower(const std::string& str);

        /**
         * @brief Convert string to uppercase
         * @param str String to convert
         * @return Uppercase string
         */
        static std::string toUpper(const std::string& str);

        /**
         * @brief Convert string to camelCase
         * @param str String to convert
         * @return camelCase string
         */
        static std::string toCamelCase(const std::string& str);

        /**
         * @brief Convert string to snake_case
         * @param str String to convert
         * @return snake_case string
         */
        static std::string toSnakeCase(const std::string& str);

        /**
         * @brief Convert string to kebab-case
         * @param str String to convert
         * @return kebab-case string
         */
        static std::string toKebabCase(const std::string& str);

        /**
         * @brief Convert string to PascalCase
         * @param str String to convert
         * @return PascalCase string
         */
        static std::string toPascalCase(const std::string& str);

        // ===== TRIMMING OPERATIONS =====

        /**
         * @brief Trim whitespace from both ends of string
         * @param str String to trim
         * @return Trimmed string
         */
        static std::string trim(const std::string& str);

        /**
         * @brief Trim whitespace from left end of string
         * @param str String to trim
         * @return Left-trimmed string
         */
        static std::string trimLeft(const std::string& str);

        /**
         * @brief Trim whitespace from right end of string
         * @param str String to trim
         * @return Right-trimmed string
         */
        static std::string trimRight(const std::string& str);

        /**
         * @brief Trim specific characters from both ends
         * @param str String to trim
         * @param chars Characters to remove
         * @return Trimmed string
         */
        static std::string trim(const std::string& str, const std::string& chars);

        // ===== SEARCHING OPERATIONS =====

        /**
         * @brief Check if string starts with prefix
         * @param str String to check
         * @param prefix Prefix to look for
         * @return true if string starts with prefix
         */
        static bool startsWith(const std::string& str, const std::string& prefix);

        /**
         * @brief Check if string ends with suffix
         * @param str String to check
         * @param suffix Suffix to look for
         * @return true if string ends with suffix
         */
        static bool endsWith(const std::string& str, const std::string& suffix);

        /**
         * @brief Check if string contains substring
         * @param str String to search
         * @param substring Substring to find
         * @return true if substring found
         */
        static bool contains(const std::string& str, const std::string& substring);

        /**
         * @brief Check if string contains substring (case-insensitive)
         * @param str String to search
         * @param substring Substring to find
         * @return true if substring found (case-insensitive)
         */
        static bool containsIgnoreCase(const std::string& str, const std::string& substring);

        /**
         * @brief Find all occurrences of substring in string
         * @param str String to search
         * @param substring Substring to find
         * @return Vector of positions where substring found
         */
        static std::vector<size_t> findAll(const std::string& str, const std::string& substring);

        // ===== REPLACEMENT OPERATIONS =====

        /**
         * @brief Replace first occurrence of substring
         * @param str String to modify
         * @param from Substring to replace
         * @param to Replacement substring
         * @return Modified string
         */
        static std::string replace(const std::string& str, const std::string& from, const std::string& to);

        /**
         * @brief Replace all occurrences of substring
         * @param str String to modify
         * @param from Substring to replace
         * @param to Replacement substring
         * @return Modified string
         */
        static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

        /**
         * @brief Replace all occurrences using predicate function
         * @param str String to modify
         * @param predicate Function that returns replacement for each character
         * @return Modified string
         */
        static std::string replaceAll(const std::string& str, std::function<char(char)> predicate);

        // ===== ENCODING AND DECODING =====

        /**
         * @brief URL-encode string
         * @param str String to encode
         * @return URL-encoded string
         */
        static std::string urlEncode(const std::string& str);

        /**
         * @brief URL-decode string
         * @param str String to decode
         * @return URL-decoded string
         * @throws std::invalid_argument if encoding is invalid
         */
        static std::string urlDecode(const std::string& str);

        /**
         * @brief HTML-escape string
         * @param str String to escape
         * @return HTML-escaped string
         */
        static std::string htmlEscape(const std::string& str);

        /**
         * @brief HTML-unescape string
         * @param str String to unescape
         * @return HTML-unescaped string
         */
        static std::string htmlUnescape(const std::string& str);

        /**
         * @brief Base64-encode string
         * @param str String to encode
         * @return Base64-encoded string
         */
        static std::string base64Encode(const std::string& str);

        /**
         * @brief Base64-decode string
         * @param str String to decode
         * @return Base64-decoded string
         * @throws std::invalid_argument if encoding is invalid
         */
        static std::string base64Decode(const std::string& str);

        // ===== VALIDATION OPERATIONS =====

        /**
         * @brief Check if string represents a number
         * @param str String to check
         * @return true if string is a valid number
         */
        static bool isNumber(const std::string& str);

        /**
         * @brief Check if string represents an integer
         * @param str String to check
         * @return true if string is a valid integer
         */
        static bool isInteger(const std::string& str);

        /**
         * @brief Check if string represents a floating-point number
         * @param str String to check
         * @return true if string is a valid float
         */
        static bool isFloat(const std::string& str);

        /**
         * @brief Check if string contains only alphabetic characters
         * @param str String to check
         * @return true if string is alphabetic
         */
        static bool isAlpha(const std::string& str);

        /**
         * @brief Check if string contains only alphanumeric characters
         * @param str String to check
         * @return true if string is alphanumeric
         */
        static bool isAlphaNumeric(const std::string& str);

        /**
         * @brief Check if string contains only whitespace
         * @param str String to check
         * @return true if string is whitespace-only
         */
        static bool isWhitespace(const std::string& str);

        /**
         * @brief Check if string is a valid email address
         * @param str String to check
         * @return true if string is a valid email
         */
        static bool isEmail(const std::string& str);

        /**
         * @brief Check if string is a valid IP address
         * @param str String to check
         * @return true if string is a valid IP address
         */
        static bool isIPAddress(const std::string& str);

        // ===== FORMATTING OPERATIONS =====

        /**
         * @brief printf-style string formatting
         * @param format Format string with placeholders
         * @param args Arguments for placeholders
         * @return Formatted string
         */
        template<typename... Args>
        static std::string format(const std::string& format, Args... args);

        /**
         * @brief Pad string on left side
         * @param str String to pad
         * @param length Target length
         * @param padChar Padding character (default: space)
         * @return Padded string
         */
        static std::string padLeft(const std::string& str, size_t length, char padChar = ' ');

        /**
         * @brief Pad string on right side
         * @param str String to pad
         * @param length Target length
         * @param padChar Padding character (default: space)
         * @return Padded string
         */
        static std::string padRight(const std::string& str, size_t length, char padChar = ' ');

        /**
         * @brief Pad string on both sides (center)
         * @param str String to pad
         * @param length Target length
         * @param padChar Padding character (default: space)
         * @return Centered string
         */
        static std::string padCenter(const std::string& str, size_t length, char padChar = ' ');

        // ===== TYPE CONVERSION =====

        /**
         * @brief Convert string to integer with error handling
         * @param str String to convert
         * @param defaultValue Value to return if conversion fails
         * @return Converted integer or default value
         */
        static int toInt(const std::string& str, int defaultValue = 0);

        /**
         * @brief Convert string to long with error handling
         * @param str String to convert
         * @param defaultValue Value to return if conversion fails
         * @return Converted long or default value
         */
        static long toLong(const std::string& str, long defaultValue = 0);

        /**
         * @brief Convert string to double with error handling
         * @param str String to convert
         * @param defaultValue Value to return if conversion fails
         * @return Converted double or default value
         */
        static double toDouble(const std::string& str, double defaultValue = 0.0);

        /**
         * @brief Convert string to boolean with error handling
         * @param str String to convert
         * @param defaultValue Value to return if conversion fails
         * @return Converted boolean or default value
         */
        static bool toBool(const std::string& str, bool defaultValue = false);

        // ===== STRING GENERATION =====

        /**
         * @brief Generate random UUID (version 4)
         * @return UUID string in format "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
         */
        static std::string generateUUID();

        /**
         * @brief Generate random string
         * @param length Desired string length
         * @param charset Character set to use for generation
         * @return Random string
         */
        static std::string randomString(size_t length, const std::string& charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

        /**
         * @brief Repeat string multiple times
         * @param str String to repeat
         * @param count Number of repetitions
         * @return Repeated string
         */
        static std::string repeat(const std::string& str, size_t count);

        // ===== UTILITY FUNCTIONS =====

        /**
         * @brief Count occurrences of character in string
         * @param str String to search
         * @param ch Character to count
         * @return Number of occurrences
         */
        static size_t count(const std::string& str, char ch);

        /**
         * @brief Count occurrences of substring in string
         * @param str String to search
         * @param substring Substring to count
         * @return Number of occurrences
         */
        static size_t count(const std::string& str, const std::string& substring);

        /**
         * @brief Reverse string
         * @param str String to reverse
         * @return Reversed string
         */
        static std::string reverse(const std::string& str);

    private:
        // Random number generator for string generation
        static std::mt19937& getRandomGenerator();
};

// Template implementation
template<typename InputIterator>
std::string StringUtils::join(InputIterator begin, InputIterator end, const std::string& delimiter) {
    if (begin == end) {
        return "";
    }

    std::ostringstream oss;
    oss << *begin;
    ++begin;

    for (; begin != end; ++begin) {
        oss << delimiter << *begin;
    }

    return oss.str();
}

template<typename... Args>
std::string StringUtils::format(const std::string& format, Args... args) {
    // Calculate required buffer size
    int size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size <= 0) {
        return "";
    }

    // Create buffer and format
    std::vector<char> buf(size);
    std::snprintf(buf.data(), size, format.c_str(), args...);
    return std::string(buf.data(), buf.data() + size - 1); // Exclude null terminator
}

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_STRING_UTILS_HPP