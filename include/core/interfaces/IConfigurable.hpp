#pragma once

#include "../../common/Types.hpp"
#include <unordered_map>
#include <any>

WEBSOCKET_NAMESPACE_BEGIN

class IConfigurable {
    WEBSOCKET_INTERFACE(IConfigurable)

public:
    virtual void configure(const std::unordered_map<std::string, std::any>& config) = 0;
    virtual std::unordered_map<std::string, std::any> getCurrentConfig() const = 0;
    virtual bool validateConfig(const std::unordered_map<std::string, std::any>& config) const = 0;

    virtual void onConfigChanged(const std::string& key, const std::any& value) = 0;
};

WEBSOCKET_NAMESPACE_END