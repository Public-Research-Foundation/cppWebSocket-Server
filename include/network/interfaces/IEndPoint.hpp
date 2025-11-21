#pragma once

#include "../../common/Types.hpp"
#include "../../common/Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IEndpoint {
    WEBSOCKET_INTERFACE(IEndpoint)

public:
    virtual std::string getAddress() const = 0;
    virtual uint16_t getPort() const = 0;
    virtual std::string getProtocol() const = 0;
    virtual bool isSecure() const = 0;

    virtual Result bind() = 0;
    virtual Result unbind() = 0;
    virtual bool isBound() const = 0;

    virtual Result listen(int backlog = SOMAXCONN) = 0;
    virtual bool isListening() const = 0;

    virtual SharedPtr<IConnection> accept() = 0;
    virtual Result asyncAccept(EventCallback<SharedPtr<IConnection>> callback) = 0;

    virtual void setReuseAddress(bool reuse) = 0;
    virtual void setReusePort(bool reuse) = 0;

    virtual std::string toString() const = 0;
};

WEBSOCKET_NAMESPACE_END