#include "config/ConfigParser.hpp"
#include "utils/StringUtils.hpp"
#include <sstream>
#include <iostream>

WEBSOCKET_NAMESPACE_BEGIN

// JsonConfigParser implementation
bool JsonConfigParser::parse(const std::string& data,
    std::unordered_map<std::string, std::any>& output) {
    // Simple JSON parser for basic configuration
    // In production, use a library like rapidjson or nlohmann/json

    if (data.empty()) {
        return false;
    }

    // Remove whitespace for simple parsing
    std::string cleanData = StringUtils::trim(data);
    if (cleanData.front() != '{' || cleanData.back() != '}') {
        return false;
    }

    // Simple key-value extraction
    std::string content = cleanData.substr(1, cleanData.length() - 2);
    std::vector<std::string> pairs = StringUtils::split(content, ',');

    for (const auto& pair : pairs) {
        std::vector<std::string> keyValue = StringUtils::split(pair, ':');
        if (keyValue.size() == 2) {
            std::string key = StringUtils::trim(StringUtils::trim(keyValue[0]), "\"");
            std::string value = StringUtils::trim(StringUtils::trim(keyValue[1]), "\"");

            // Simple type detection
            if (value == "true" || value == "false") {
                output[key] = (value == "true");
            }
            else if (StringUtils::isInteger(value)) {
                output[key] = StringUtils::toInt(value);
            }
            else if (StringUtils::isFloat(value)) {
                output[key] = StringUtils::toDouble(value);
            }
            else {
                output[key] = value;
            }
        }
    }

    return true;
}

std::string JsonConfigParser::serialize(const std::unordered_map<std::string, std::any>& config) {
    std::ostringstream oss;
    oss << "{";

    bool first = true;
    for (const auto& [key, value] : config) {
        if (!first) {
            oss << ",";
        }
        first = false;

        oss << "\"" << key << "\":";

        if (value.type() == typeid(bool)) {
            oss << (std::any_cast<bool>(value) ? "true" : "false");
        }
        else if (value.type() == typeid(int)) {
            oss << std::any_cast<int>(value);
        }
        else if (value.type() == typeid(double)) {
            oss << std::any_cast<double>(value);
        }
        else if (value.type() == typeid(std::string)) {
            oss << "\"" << std::any_cast<std::string>(value) << "\"";
        }
    }

    oss << "}";
    return oss.str();
}

// YamlConfigParser implementation  
bool YamlConfigParser::parse(const std::string& data,
    std::unordered_map<std::string, std::any>& output) {
    // Simple YAML parser for basic configuration
    // In production, use a library like yaml-cpp

    if (data.empty()) {
        return false;
    }

    std::vector<std::string> lines = StringUtils::split(data, '\n');
    for (const auto& line : lines) {
        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue; // Skip empty lines and comments
        }

        size_t colonPos = trimmed.find(':');
        if (colonPos != std::string::npos) {
            std::string key = StringUtils::trim(trimmed.substr(0, colonPos));
            std::string value = StringUtils::trim(trimmed.substr(colonPos + 1));

            // Simple type detection
            if (value == "true" || value == "false") {
                output[key] = (value == "true");
            }
            else if (StringUtils::isInteger(value)) {
                output[key] = StringUtils::toInt(value);
            }
            else if (StringUtils::isFloat(value)) {
                output[key] = StringUtils::toDouble(value);
            }
            else {
                output[key] = value;
            }
        }
    }

    return true;
}

std::string YamlConfigParser::serialize(const std::unordered_map<std::string, std::any>& config) {
    std::ostringstream oss;

    for (const auto& [key, value] : config) {
        oss << key << ": ";

        if (value.type() == typeid(bool)) {
            oss << (std::any_cast<bool>(value) ? "true" : "false");
        }
        else if (value.type() == typeid(int)) {
            oss << std::any_cast<int>(value);
        }
        else if (value.type() == typeid(double)) {
            oss << std::any_cast<double>(value);
        }
        else if (value.type() == typeid(std::string)) {
            oss << std::any_cast<std::string>(value);
        }

        oss << "\n";
    }

    return oss.str();
}

WEBSOCKET_NAMESPACE_END