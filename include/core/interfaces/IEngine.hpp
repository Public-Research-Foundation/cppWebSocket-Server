#pragma once

#include "../../common/Types.hpp"
#include "IService.hpp"

WEBSOCKET_NAMESPACE_BEGIN

class IEngine : public IService {
WEBSOCKET_INTERFACE(IEngine)
    
public:
    virtual Result processMessage(const ByteBuffer& message) = 0;
    virtual Result broadcastMessage(const ByteBuffer& message) = 0;
    
    virtual size_t getConnectionCount() const = 0;
    virtual size_t getMessageCount() const = 0;
    
    virtual void setMaxConnections(size_t max) = 0;
    virtual void setMaxMessageSize(size_t max) = 0;
};

WEBSOCKET_NAMESPACE_END