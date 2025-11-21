#pragma once

#include "../common/Types.hpp"
#include <unordered_map>
#include <any>

WEBSOCKET_NAMESPACE_BEGIN

class ConfigParser {
public:
    virtual ~ConfigParser() = default;

    virtual bool parse(const std::string& data,
        std::unordered_map<std::string, std::any>& output) = 0;
    virtual std::string serialize(const std::unordered_map<std::string, std::any>& config) = 0;
};

class JsonConfigParser : public ConfigParser {
public:
    bool parse(const std::string& data,
        std::unordered_map<std::string, std::any>& output) override;
    std::string serialize(const std::unordered_map<std::string, std::any>& config) override;
};

class YamlConfigParser : public ConfigParser {
public:
    bool parse(const std::string& data,
        std::unordered_map<std::string, std::any>& output) override;
    std::string serialize(const std::unordered_map<std::string, std::any>& config) override;
};

WEBSOCKET_NAMESPACE_END