#pragma once

#include "../../common/Types.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IComponent {
    WEBSOCKET_INTERFACE(IComponent)

public:
    virtual Result initialize() = 0;
    virtual Result configure() = 0;
    virtual void update() = 0;

    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    virtual std::string getComponentName() const = 0;
    virtual std::string getComponentVersion() const = 0;
};

WEBSOCKET_NAMESPACE_END