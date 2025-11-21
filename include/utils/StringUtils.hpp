#pragma once

#include "../common/Types.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

WEBSOCKET_NAMESPACE_BEGIN

class StringUtils {
public:
    // String splitting/joining
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

    // Case conversion
    static std::string toLower(const std::string& str);
    static std::string toUpper(const std::string& str);
    static std::string toCamelCase(const std::string& str);
    static std::string toSnakeCase(const std::string& str);

    // Trimming
    static std::string trim(const std::string& str);
    static std::string trimLeft(const std::string& str);
    static std::string trimRight(const std::string& str);

    // Searching
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static bool contains(const std::string& str, const std::string& substring);
    static bool containsIgnoreCase(const std::string& str, const std::string& substring);

    // Replacement
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
    static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

    // Encoding/decoding
    static std::string urlEncode(const std::string& str);
    static std::string urlDecode(const std::string& str);
    static std::string htmlEscape(const std::string& str);
    static std::string htmlUnescape(const std::string& str);

    // Validation
    static bool isNumber(const std::string& str);
    static bool isInteger(const std::string& str);
    static bool isFloat(const std::string& str);
    static bool isAlpha(const std::string& str);
    static bool isAlphaNumeric(const std::string& str);
    static bool isWhitespace(const std::string& str);

    // Formatting
    static std::string format(const std::string& format, ...);
    static std::string padLeft(const std::string& str, size_t length, char padChar = ' ');
    static std::string padRight(const std::string& str, size_t length, char padChar = ' ');
    static std::string padCenter(const std::string& str, size_t length, char padChar = ' ');

    // Conversion
    static int toInt(const std::string& str, int defaultValue = 0);
    static long toLong(const std::string& str, long defaultValue = 0);
    static double toDouble(const std::string& str, double defaultValue = 0.0);
    static bool toBool(const std::string& str, bool defaultValue = false);

    // UUID generation
    static std::string generateUUID();

    // Random string
    static std::string randomString(size_t length, const std::string& charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
};

WEBSOCKET_NAMESPACE_END