# Core Engine Architecture

## 📋 Overview

The `core/` directory contains the central coordination and management components of the WebSocket server. This layer provides the foundation for all server operations, including component lifecycle management, dependency injection, service coordination, and the main server API.

## 🏗️ Architecture Overview

```
core/
├── WebSocketServer.hpp   # Main server class (Public API)
├── Engine.hpp           # Central coordination engine  
├── ServiceLocator.hpp   # Dependency injection container
└── LifecycleManager.hpp # Component lifecycle orchestration
```

## 📁 Component Descriptions

### **WebSocketServer.hpp**
**Main Public API** - High-level interface for applications to create and manage WebSocket servers.

**Responsibilities**:
- Server lifecycle management (start/stop)
- Client connection management
- Message broadcasting and targeted sends
- Event handling (connect, message, disconnect, error)
- Statistics collection and monitoring

**Key Features**:
- **Clean Public API** - Simple interface for application developers
- **Pimpl Pattern** - Binary compatibility and implementation hiding
- **Event-Driven** - Asynchronous, non-blocking operations
- **Thread-Safe** - Safe concurrent access from multiple threads
- **Comprehensive Statistics** - Real-time monitoring and metrics

**Usage Example**:
```cpp
#include "core/WebSocketServer.hpp"

websocket::WebSocketServer server;

// Set up event handlers
server.onMessage([](auto client, auto message) {
    std::cout << "Received: " << message.getText() << std::endl;
    server.sendText(client, "Echo: " + message.getText());
});

server.onConnect([](auto client) {
    std::cout << "Client connected: " << client << std::endl;
});

// Start server
server.start(8080);
server.waitForStop();
```

### **Engine.hpp**
**Central Coordination Engine** - Singleton that orchestrates all system components.

**Responsibilities**:
- Component and service registration
- Dependency resolution and injection
- System initialization and shutdown sequencing
- Operational mode management (development/testing/production)
- Error handling and recovery coordination

**Design Patterns**:
- **Singleton** - Global coordination instance
- **Facade** - Simplified interface to complex subsystem
- **Mediator** - Coordinates communication between components

**State Management**:
```
UNINITIALIZED → INITIALIZING → RUNNING → STOPPING → STOPPED
     ↓              ↓           ↓          ↓          ↓
   Created       Starting    Normal     Shutting   Terminal
                              Operation   Down      State
```

**Usage Example**:
```cpp
#include "core/Engine.hpp"

// Get engine instance
auto& engine = websocket::Engine::getInstance();

// Initialize with configuration
engine.initialize("config/server.json");

// Register custom services
engine.registerService<MyCustomService>("MyService");

// Start all components
engine.start();

// Access services
auto service = engine.getService<MyCustomService>("MyService");

// Wait for shutdown
engine.waitForStop();
```

### **ServiceLocator.hpp**
**Dependency Injection Container** - Manages service instances and dependencies.

**Responsibilities**:
- Service registration and resolution
- Lifetime management (singleton, transient, scoped)
- Interface-based service lookup
- Thread-safe service access
- Dependency graph management

**Service Lifetimes**:
- **Singleton** - Single instance shared across entire application
- **Transient** - New instance created for each resolution request  
- **Scoped** - Instance shared within a specific scope/context

**Usage Example**:
```cpp
#include "core/ServiceLocator.hpp"

websocket::ServiceLocator locator;

// Register services
locator.registerService<ILogger, FileLogger>("FileLogger");
locator.registerService<IDatabase, PostgreSQLDatabase>("Database");

// Register with custom factory
locator.registerFactory<ICache>([]() {
    return std::make_shared<RedisCache>("localhost", 6379);
});

// Resolve services
auto logger = locator.resolve<ILogger>("FileLogger");
auto database = locator.resolve<IDatabase>("Database");
auto cache = locator.resolve<ICache>();
```

### **LifecycleManager.hpp**
**Component Lifecycle Orchestrator** - Manages initialization and shutdown sequencing.

**Responsibilities**:
- Dependency-based initialization ordering
- Ordered shutdown (reverse of initialization)
- Lifecycle state tracking
- Error handling during startup/shutdown
- Timeout management for slow operations

**Lifecycle Phases**:
1. **CORE** - Fundamental components (logging, configuration)
2. **INFRASTRUCTURE** - System services (thread pools, networking)
3. **SERVICES** - Business logic services
4. **APPLICATION** - Application-level components

**Component States**:
```
UNINITIALIZED → INITIALIZING → INITIALIZED → STARTING → RUNNING → STOPPING → STOPPED
```

**Usage Example**:
```cpp
#include "core/LifecycleManager.hpp"

websocket::LifecycleManager manager;

// Register components with dependencies
manager.registerComponent(logger, "Logger", Phase::CORE);
manager.registerComponent(thread_pool, "ThreadPool", Phase::INFRASTRUCTURE, {"Logger"});
manager.registerComponent(server, "WebSocketServer", Phase::SERVICES, {"ThreadPool"});

// Initialize in correct order
if (manager.initializeAll()) {
    manager.startAll();
}

// Shutdown gracefully
manager.stopAll(true);
```

## 🔄 Data Flow

### **Server Startup Sequence**:
1. **Application** creates `WebSocketServer` instance
2. **WebSocketServer** initializes `Engine` singleton
3. **Engine** uses `ServiceLocator` to register core services
4. **Engine** uses `LifecycleManager` to initialize components in dependency order
5. **LifecycleManager** ensures proper sequencing:
   - Logger → Configuration → Thread Pool → Network Stack → WebSocket Server
6. **WebSocketServer** begins accepting connections

### **Message Handling Flow**:
1. **Network Layer** receives raw data
2. **Protocol Handler** parses WebSocket frames
3. **Session Manager** routes to appropriate client session
4. **WebSocketServer** invokes application message handler
5. **Application** processes message and optionally sends response

### **Shutdown Sequence**:
1. **Application** calls `server.stop()`
2. **WebSocketServer** notifies `Engine` to begin shutdown
3. **LifecycleManager** stops components in reverse initialization order
4. **ServiceLocator** cleans up service instances
5. **Engine** transitions to STOPPED state

## 🎯 Design Principles

### **Separation of Concerns**
- **WebSocketServer** - Public API and application interface
- **Engine** - System coordination and component management
- **ServiceLocator** - Dependency injection and service lifecycle
- **LifecycleManager** - Initialization and shutdown sequencing

### **Testability**
- Interface-based design enables easy mocking
- Dependency injection supports unit testing
- Service locator allows test-specific service registration
- Lifecycle manager provides controlled component states

### **Extensibility**
- Plugin architecture through service registration
- Custom components can integrate via lifecycle management
- New protocols can be added without core modifications
- Configuration-driven behavior changes

### **Reliability**
- Graceful error handling at all levels
- Proper resource cleanup during shutdown
- Dependency validation prevents misconfiguration
- State machine ensures valid transitions

## 🔧 Configuration

### **Server Configuration**:
```cpp
websocket::ServerConfig config;
config.port = 8080;
config.max_connections = 10000;
config.io_threads = 4;
config.worker_threads = 8;

websocket::WebSocketServer server(config);
```

### **Engine Configuration**:
```json
{
  "mode": "production",
  "components": {
    "logging": {"level": "info"},
    "monitoring": {"enabled": true}
  }
}
```

## 📊 Monitoring & Statistics

The core components provide comprehensive statistics:

- **Connection counts** (active, total, peak)
- **Message rates** (incoming, outgoing, errors)
- **Resource usage** (memory, threads, file descriptors)
- **Performance metrics** (latency, throughput)
- **Component health** (status, errors, warnings)

## 🚀 Performance Features

- **Async-first architecture** - Non-blocking I/O operations
- **Connection pooling** - Efficient resource reuse
- **Thread pooling** - Optimal CPU utilization
- **Zero-copy messaging** - Reduced memory overhead
- **Buffer recycling** - Minimized allocations
- **Lock-free structures** - High-concurrency operations

## 🔒 Error Handling

### **Graceful Degradation**:
- Failed components don't crash the entire system
- Circuit breaker pattern for dependent services
- Fallback mechanisms for critical operations

### **Comprehensive Logging**:
- Structured logging with context
- Different log levels for various environments
- Performance tracing and audit trails

### **Health Monitoring**:
- Component health checks
- Automatic recovery where possible
- Alerting for critical failures

## 📚 Related Documentation

- [Network Layer](../network/README.md) - Networking components and I/O management
- [Protocol Layer](../protocol/README.md) - WebSocket protocol implementation
- [Configuration](../config/README.md) - Server configuration management
- [Utilities](../utils/README.md) - Common utilities and helpers

---

This core architecture provides a robust, scalable foundation for building high-performance WebSocket servers with enterprise-grade reliability and maintainability.