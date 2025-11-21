#pragma once

#include "../common/Types.hpp"
#include <vector>
#include <functional>

WEBSOCKET_NAMESPACE_BEGIN

struct ValidationRule {
    std::string key;
    std::string type;
    std::function<bool(const std::any&)> validator;
    std::string errorMessage;
};

class ConfigValidator {
public:
    ConfigValidator();
    
    bool validate(const std::unordered_map<std::string, std::any>& config) const;
    std::vector<std::string> getErrors() const;
    
    void addCustomRule(const ValidationRule& rule);
    void setRequiredFields(const std::vector<std::string>& fields);
    
private:
    std::vector<ValidationRule> rules_;
    std::vector<std::string> requiredFields_;
    mutable std::vector<std::string> validationErrors_;
    
    bool validateType(const std::string& key, const std::any& value, const std::string& expectedType) const;
    bool validateRange(const std::string& key, const std::any& value, double min, double max) const;
    bool validateFileExists(const std::string& key, const std::any& value) const;
};

WEBSOCKET_NAMESPACE_END