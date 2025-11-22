#include "config/ConfigValidator.hpp"
#include "utils/StringUtils.hpp"

WEBSOCKET_NAMESPACE_BEGIN

ConfigValidator::ConfigValidator() {
    initializeDefaultRules();
}

void ConfigValidator::initializeDefaultRules() {
    // Server configuration rules
    rules_.push_back({
        "server.port", "int",
        [](const std::any& value) {
            try {
                int port = std::any_cast<int>(value);
                return port > 0 && port <= 65535;
            }
 catch (const std::bad_any_cast&) {
  return false;
}
},
"Port must be between 1 and 65535"
        });

    rules_.push_back({
        "server.threads", "int",
        [](const std::any& value) {
            try {
                int threads = std::any_cast<int>(value);
                return threads > 0 && threads <= 256;
            }
 catch (const std::bad_any_cast&) {
  return false;
}
},
"Thread count must be between 1 and 256"
        });

    rules_.push_back({
        "server.max_connections", "int",
        [](const std::any& value) {
            try {
                int max = std::any_cast<int>(value);
                return max > 0 && max <= 1000000;
            }
 catch (const std::bad_any_cast&) {
  return false;
}
},
"Max connections must be between 1 and 1000000"
        });

    rules_.push_back({
        "logging.level", "string",
        [](const std::any& value) {
            try {
                std::string level = std::any_cast<std::string>(value);
                return level == "TRACE" || level == "DEBUG" || level == "INFO" ||
                       level == "WARN" || level == "ERROR" || level == "FATAL";
            }
 catch (const std::bad_any_cast&) {
  return false;
}
},
"Log level must be TRACE, DEBUG, INFO, WARN, ERROR, or FATAL"
        });

    // Set required fields
    requiredFields_ = { "server.port", "server.threads" };
}

bool ConfigValidator::validate(const std::unordered_map<std::string, std::any>& config) const {
    validationErrors_.clear();

    // Check required fields
    for (const auto& field : requiredFields_) {
        if (config.find(field) == config.end()) {
            validationErrors_.push_back("Required field missing: " + field);
        }
    }

    // Validate each rule
    for (const auto& rule : rules_) {
        auto it = config.find(rule.key);
        if (it != config.end()) {
            if (!rule.validator(it->second)) {
                validationErrors_.push_back(rule.errorMessage + " (key: " + rule.key + ")");
            }
        }
    }

    return validationErrors_.empty();
}

std::vector<std::string> ConfigValidator::getErrors() const {
    return validationErrors_;
}

void ConfigValidator::addCustomRule(const ValidationRule& rule) {
    rules_.push_back(rule);
}

void ConfigValidator::setRequiredFields(const std::vector<std::string>& fields) {
    requiredFields_ = fields;
}

bool ConfigValidator::validateType(const std::string& key, const std::any& value, const std::string& expectedType) const {
    if (expectedType == "int") {
        return value.type() == typeid(int);
    }
    else if (expectedType == "bool") {
        return value.type() == typeid(bool);
    }
    else if (expectedType == "string") {
        return value.type() == typeid(std::string);
    }
    else if (expectedType == "double") {
        return value.type() == typeid(double);
    }
    return false;
}

bool ConfigValidator::validateRange(const std::string& key, const std::any& value, double min, double max) const {
    try {
        if (value.type() == typeid(int)) {
            int val = std::any_cast<int>(value);
            return val >= min && val <= max;
        }
        else if (value.type() == typeid(double)) {
            double val = std::any_cast<double>(value);
            return val >= min && val <= max;
        }
    }
    catch (const std::bad_any_cast&) {
        return false;
    }
    return false;
}

bool ConfigValidator::validateFileExists(const std::string& key, const std::any& value) const {
    try {
        if (value.type() == typeid(std::string)) {
            std::string path = std::any_cast<std::string>(value);
            // In production, use FileUtils::exists(path)
            return !path.empty(); // Simplified check
        }
    }
    catch (const std::bad_any_cast&) {
        return false;
    }
    return false;
}

WEBSOCKET_NAMESPACE_END