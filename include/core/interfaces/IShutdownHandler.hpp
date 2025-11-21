#pragma once

#include "../../common/Types.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IShutdownHandler {
    WEBSOCKET_INTERFACE(IShutdownHandler)

public:
    virtual void onShutdown() = 0;
    virtual void onEmergencyShutdown() = 0;
    virtual bool canShutdown() const = 0;
    virtual int getShutdownPriority() const = 0;
};

WEBSOCKET_NAMESPACE_END