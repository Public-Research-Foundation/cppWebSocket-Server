#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IService {
    WEBSOCKET_INTERFACE(IService)

public:
    virtual Result initialize() = 0;
    virtual Result start() = 0;
    virtual Result stop() = 0;
    virtual Result shutdown() = 0;

    virtual bool isRunning() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getStatus() const = 0;
};

WEBSOCKET_NAMESPACE_END