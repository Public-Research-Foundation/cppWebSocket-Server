#pragma once

#include "../common/Types.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Comprehensive string manipulation utilities
 * Collection of commonly needed string operations
 * All methods are static and thread-safe
 */
    class StringUtils {
    public:
        // ===== STRING SPLITTING/JOINING =====

        /**
         * Split string by single character delimiter
         * @param str String to split
         * @param delimiter Character to split on
         * @return Vector of substrings
         */
        static std::vector<std::string> split(const std::string& str, char delimiter);

        /**
         * Split string by string delimiter
         * @param str String to split
         * @param delimiter String to split on
         * @return Vector of substrings
         */
        static std::vector<std::string> split(const std::string& str, const std::string& delimiter);

        /**
         * Join vector of strings with delimiter
         * @param strings Strings to join
         * @param delimiter Delimiter to insert between strings
         * @return Joined string
         */
        static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

        // ===== CASE CONVERSION =====

        static std::string toLower(const std::string& str);
        static std::string toUpper(const std::string& str);
        static std::string toCamelCase(const std::string& str);
        static std::string toSnakeCase(const std::string& str);

        // ===== TRIMMING =====
        // Remove whitespace from string edges

        static std::string trim(const std::string& str);
        static std::string trimLeft(const std::string& str);
        static std::string trimRight(const std::string& str);

        // ===== SEARCHING =====

        static bool startsWith(const std::string& str, const std::string& prefix);
        static bool endsWith(const std::string& str, const std::string& suffix);
        static bool contains(const std::string& str, const std::string& substring);
        static bool containsIgnoreCase(const std::string& str, const std::string& substring);

        // ===== REPLACEMENT =====

        static std::string replace(const std::string& str, const std::string& from, const std::string& to);
        static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

        // ===== ENCODING/DECODING =====

        static std::string urlEncode(const std::string& str);
        static std::string urlDecode(const std::string& str);
        static std::string htmlEscape(const std::string& str);
        static std::string htmlUnescape(const std::string& str);

        // ===== VALIDATION =====

        static bool isNumber(const std::string& str);
        static bool isInteger(const std::string& str);
        static bool isFloat(const std::string& str);
        static bool isAlpha(const std::string& str);
        static bool isAlphaNumeric(const std::string& str);
        static bool isWhitespace(const std::string& str);

        // ===== FORMATTING =====

        /**
         * printf-style string formatting
         * @param format Format string with placeholders
         * @param ... Arguments for placeholders
         * @return Formatted string
         */
        static std::string format(const std::string& format, ...);

        static std::string padLeft(const std::string& str, size_t length, char padChar = ' ');
        static std::string padRight(const std::string& str, size_t length, char padChar = ' ');
        static std::string padCenter(const std::string& str, size_t length, char padChar = ' ');

        // ===== CONVERSION =====
        // String to primitive type conversions with error handling

        static int toInt(const std::string& str, int defaultValue = 0);
        static long toLong(const std::string& str, long defaultValue = 0);
        static double toDouble(const std::string& str, double defaultValue = 0.0);
        static bool toBool(const std::string& str, bool defaultValue = false);

        // ===== ID GENERATION =====

        /**
         * Generate random UUID (version 4)
         * @return UUID string in format "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
         */
        static std::string generateUUID();

        /**
         * Generate random string
         * @param length Desired string length
         * @param charset Character set to use for generation
         * @return Random string
         */
        static std::string randomString(size_t length, const std::string& charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
};

WEBSOCKET_NAMESPACE_END