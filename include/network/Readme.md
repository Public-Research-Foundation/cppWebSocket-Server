# Network Layer

## 📋 Overview

The `network/` directory contains the core networking components responsible for handling TCP connections, WebSocket sessions, connection pooling, and asynchronous I/O operations. This layer provides the foundation for high-performance, scalable WebSocket communication with efficient resource management.

## 🎯 Main Purpose

The network layer serves as the **communication backbone** that provides:

- **Connection Management**: TCP socket handling and lifecycle management
- **Session Management**: WebSocket client session state and messaging
- **Resource Pooling**: Efficient connection and buffer reuse
- **Async I/O**: Non-blocking operations with thread pools
- **Endpoint Handling**: Network address abstraction and resolution

## 🏗️ Architecture

```
network/
├── WebSocketConnection.hpp  ───┐
├── WebSocketSession.hpp     ──┤→ Connection Management
├── ConnectionPool.hpp       ──┤
├── IOThreadPool.hpp         ──┤→ Resource Management  
└── Endpoint.hpp             ──┘→ Network Abstraction
```

### **Data Flow**:
```
TCP Accept → WebSocketConnection → WebSocketSession → Application
     ↑               ↑                  ↑
IOThreadPool   ConnectionPool      SessionManager
```

## 📁 Component Details

### **WebSocketConnection.hpp**
**Low-level TCP connection handler with async I/O**

**Key Responsibilities**:
- Raw TCP socket management
- Asynchronous read/write operations
- Connection state tracking
- Error handling and cleanup
- Buffer management

**Key Features**:
- ✅ **Async I/O** - Non-blocking socket operations
- ✅ **State Machine** - Connection lifecycle management
- ✅ **Buffer Management** - Efficient data transfer
- ✅ **Error Resilience** - Graceful connection recovery

**Class Diagram**:
```cpp
class WebSocketConnection {
    enum class State { DISCONNECTED, CONNECTING, CONNECTED, CLOSING, CLOSED };
    
    // Core operations
    void start();
    void close(bool graceful = true);
    bool send(const Buffer& data);
    
    // Event callbacks
    setReceiveCallback()
    setCloseCallback()
};
```

**Usage**:
```cpp
auto connection = std::make_shared<WebSocketConnection>(io_context);
connection->setReceiveCallback([&](const Buffer& data) {
    // Handle incoming data
});
connection->start();
```

### **WebSocketSession.hpp**
**WebSocket client session with protocol state management**

**Key Responsibilities**:
- WebSocket session lifecycle
- Message fragmentation/defragmentation
- Ping/Pong heartbeat mechanism
- Session-specific data storage
- Protocol state validation

**Key Features**:
- ✅ **Session State** - CONNECTING, CONNECTED, CLOSING, CLOSED
- ✅ **Message Handling** - Fragmentation and reassembly
- ✅ **Heartbeat** - Automatic ping/pong for connection health
- ✅ **User Data** - Custom session storage

**Class Diagram**:
```cpp
class WebSocketSession {
    enum class State { CONNECTING, CONNECTED, CLOSING, CLOSED };
    
    // Message operations
    bool sendText(const std::string& message);
    bool sendBinary(const Buffer& data);
    void handleFrame(const WebSocketFrame& frame);
    
    // Session management
    void close(uint16_t code, const std::string& reason);
    void setUserData(const std::string& key, const std::string& value);
};
```

**Usage**:
```cpp
auto session = std::make_shared<WebSocketSession>(client_id, connection);
session->start();
session->sendText("Welcome to WebSocket server!");
session->setUserData("username", "john_doe");
```

### **ConnectionPool.hpp**
**Resource pool for efficient connection reuse**

**Key Responsibilities**:
- Connection object pooling
- Resource reuse and memory management
- Pool sizing and growth strategies
- Connection lifecycle tracking

**Key Features**:
- ✅ **Object Pooling** - Reduce allocation overhead
- ✅ **Configurable Sizing** - Dynamic pool growth
- ✅ **Thread Safety** - Concurrent access support
- ✅ **Statistics** - Pool usage monitoring

**Configuration**:
```cpp
struct Config {
    size_t initial_size{100};     // Initial pool size
    size_t max_size{1000};        // Maximum pool size  
    size_t grow_size{50};         // Growth increment
    bool enable_growth{true};     // Allow dynamic growth
};
```

**Usage**:
```cpp
ConnectionPool::Config pool_config{200, 5000, 100, true};
ConnectionPool pool(pool_config);

auto connection = pool.acquire();
// Use connection...
pool.release(std::move(connection));
```

### **IOThreadPool.hpp**
**Managed thread pool for asynchronous I/O operations**

**Key Responsibilities**:
- Thread pool management for async operations
- Work stealing and load balancing
- Graceful shutdown support
- Thread affinity configuration

**Key Features**:
- ✅ **Fixed-size Pool** - Controlled resource usage
- ✅ **Work Stealing** - Optimal load distribution
- ✅ **Graceful Shutdown** - Complete pending operations
- ✅ **Thread Affinity** - CPU optimization support

**Configuration**:
```cpp
struct Config {
    size_t thread_count{0};           // 0 = auto-detect
    size_t queue_size_per_thread{1024};
    bool enable_affinity{false};
    std::string name{"IOThreadPool"};
};
```

**Usage**:
```cpp
IOThreadPool::Config thread_config{4, 2048, true, "WebSocketIO"};
IOThreadPool pool(thread_config);
pool.start();

pool.post([]() {
    // Async I/O operation
});
```

### **Endpoint.hpp**
**Network endpoint abstraction for address handling**

**Key Responsibilities**:
- IP address and port management
- Hostname resolution
- Address family support (IPv4/IPv6)
- String representation parsing

**Key Features**:
- ✅ **Dual Stack** - IPv4 and IPv6 support
- ✅ **Hostname Resolution** - DNS lookup capabilities
- ✅ **Validation** - Address format checking
- ✅ **Hashing** - Use in containers

**Usage**:
```cpp
Endpoint endpoint("192.168.1.1", 8080);
std::cout << endpoint.toString(); // "192.168.1.1:8080"

auto endpoints = Endpoint::resolve("example.com", "8080");
for (const auto& ep : endpoints) {
    std::cout << ep.getAddress() << ":" << ep.getPort() << std::endl;
}
```

## 🔄 Data Flow and Lifecycle

### **Connection Establishment**:
```
1. TCP Accept → New WebSocketConnection
2. HTTP Upgrade → WebSocket Handshake
3. Handshake Success → Create WebSocketSession
4. Session Start → Begin Message Processing
```

### **Message Processing**:
```
1. Raw Data → WebSocketConnection (async read)
2. Frame Parsing → Protocol Handler
3. Message Assembly → WebSocketSession
4. Application Callback → User Code
```

### **Resource Management**:
```
ConnectionPool → WebSocketConnection → IOThreadPool
     ↑                    ↑                  ↑
  Acquire             Process Data       Post Work
  Release             Handle Events      Execute Async
```

## 🛠️ Key Features and Optimizations

### **1. High Performance Architecture**
- **Async I/O**: Non-blocking operations with completion handlers
- **Zero-Copy**: Buffer sharing where possible
- **Thread Pools**: Optimal CPU utilization
- **Connection Pooling**: Reduced allocation overhead

### **2. Scalability**
- **Configurable Limits**: Max connections, thread counts
- **Dynamic Growth**: Pool expansion under load
- **Load Balancing**: Work stealing across threads
- **Memory Efficiency**: Buffer reuse and pooling

### **3. Reliability**
- **Graceful Degradation**: Handle resource exhaustion
- **Error Recovery**: Automatic reconnection strategies
- **State Validation**: Prevent protocol violations
- **Resource Cleanup**: Proper connection termination

### **4. Monitoring and Statistics**
```cpp
// Connection statistics
struct ConnectionStats {
    size_t bytes_received;
    size_t bytes_sent;
    size_t messages_processed;
    std::chrono::seconds uptime;
};

// Pool statistics  
struct PoolStats {
    size_t total_connections;
    size_t active_connections;
    size_t available_connections;
    size_t allocation_count;
};
```

## 🚀 Usage Examples

### **Complete Server Setup**:
```cpp
#include "network/IOThreadPool.hpp"
#include "network/ConnectionPool.hpp"
#include "network/WebSocketSession.hpp"

using namespace CppWebSocket;

class WebSocketServer {
    IOThreadPool io_pool_;
    ConnectionPool connection_pool_;
    std::unordered_map<ClientID, std::shared_ptr<WebSocketSession>> sessions_;
    
public:
    void start() {
        // Start I/O thread pool
        io_pool_.start();
        
        // Setup TCP acceptor
        setupAcceptor();
    }
    
    void handleNewConnection(std::shared_ptr<WebSocketConnection> connection) {
        auto session = std::make_shared<WebSocketSession>(
            generateClientId(), connection);
            
        session->start();
        sessions_[session->getId()] = session;
        
        // Setup session callbacks
        setupSessionCallbacks(session);
    }
};
```

### **Async Message Broadcasting**:
```cpp
void broadcastMessage(const Message& message) {
    io_pool_.post([this, message]() {
        for (auto& [client_id, session] : sessions_) {
            if (session->isActive()) {
                session->sendBinary(message.data);
            }
        }
    });
}
```

### **Connection Pool Management**:
```cpp
class ConnectionManager {
    ConnectionPool pool_;
    
    void handleClientRequest() {
        auto connection = pool_.acquire();
        if (!connection) {
            // Handle pool exhaustion
            return;
        }
        
        connection->setReceiveCallback([this](const Buffer& data) {
            processClientData(data);
        });
        
        connection->start();
        
        // Later, when connection is done
        pool_.release(std::move(connection));
    }
};
```

## 🔧 Integration with Other Layers

### **Protocol Layer Integration**:
```cpp
class WebSocketSession {
    std::unique_ptr<ProtocolHandler> protocol_handler_;
    
    void handleData(const Buffer& data) {
        size_t consumed = protocol_handler_->processData(data);
        if (consumed == 0) {
            // Protocol error
            close(1002, "Protocol violation");
        }
    }
};
```

### **Core Engine Integration**:
```cpp
class WebSocketServer {
    void onSessionMessage(ClientID client_id, const Message& message) {
        // Forward to application layer
        if (message_handler_) {
            message_handler_(client_id, message);
        }
    }
};
```

### **Configuration Integration**:
```cpp
void initializeFromConfig() {
    auto& config = GlobalConfig::getInstance();
    auto cfg_manager = config.getConfigManager();
    
    size_t max_connections = cfg_manager->get<size_t>("network.max_connections");
    size_t io_threads = cfg_manager->get<size_t>("network.io_threads");
    
    IOThreadPool::Config io_config{io_threads};
    ConnectionPool::Config pool_config{max_connections / 10, max_connections};
}
```

## 📊 Performance Characteristics

### **Connection Scaling**:
- **Memory Usage**: ~2KB per connection (without buffers)
- **Thread Overhead**: Minimal context switching
- **Network Throughput**: Limited by OS socket buffers
- **Connection Rate**: Thousands per second (hardware dependent)

### **Optimization Tips**:
1. **Tune Pool Sizes**: Match expected connection count
2. **Configure Thread Count**: CPU cores × 2 for I/O bound workloads
3. **Buffer Sizing**: Balance memory usage and performance
4. **Connection Timeouts**: Prevent resource leaks

## 🔒 Security Considerations

- **Input Validation**: Sanitize all incoming data
- **Resource Limits**: Prevent DoS through connection exhaustion
- **Buffer Bounds**: Prevent overflow attacks
- **TLS/SSL**: Support for encrypted connections (when configured)

---

The network layer provides a robust, scalable foundation for WebSocket communication with enterprise-grade performance and reliability characteristics. Its modular design allows for easy customization while maintaining high efficiency through resource pooling and async I/O operations.