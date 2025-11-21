#pragma once

#include "../../common/Types.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IInitializable {
    WEBSOCKET_INTERFACE(IInitializable)

public:
    virtual Result preInitialize() = 0;
    virtual Result initialize() = 0;
    virtual Result postInitialize() = 0;

    virtual bool isInitialized() const = 0;
    virtual std::vector<std::string> getDependencies() const = 0;
};

WEBSOCKET_NAMESPACE_END