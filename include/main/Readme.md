# Application Main Layer

## 📋 Overview

The `main/` directory contains the application entry points and command-line interface components that orchestrate the WebSocket server lifecycle. This layer bridges the gap between the core WebSocket library and the operating system, providing configuration management, signal handling, daemonization, and graceful shutdown capabilities.

## 🎯 Main Purpose

The main layer serves as the **application orchestrator** that provides:

- **Application Lifecycle**: Server startup, runtime management, and graceful shutdown
- **Configuration Management**: Command-line parsing and configuration overrides
- **Process Management**: Daemonization and signal handling
- **Error Handling**: Global exception handling and error reporting
- **Integration**: Glue between library components and operating system

## 🏗️ Architecture

```
main/
├── Application.hpp       ───┐
├── CommandLineParser.hpp ──┤→ Application Core
├── ConfigOverrides.hpp   ──┤
├── Daemonizer.hpp        ──┤→ System Integration
└── SignalHandler.hpp     ──┘→ Process Control
```

### **Execution Flow**:
```
Command Line → Application → WebSocketServer → Signal Handling
     ↓              ↓              ↓               ↓
   Parsing     Initialization   Runtime        Shutdown
```

## 📁 Component Details

### **Application.hpp**
**Main application class orchestrating server lifecycle**

**Key Responsibilities**:
- Application initialization and shutdown sequencing
- Configuration loading and validation
- Component dependency resolution
- Graceful shutdown coordination
- Error handling and recovery

**Key Features**:
- ✅ **Lifecycle Management** - Coordinated startup/shutdown
- ✅ **Dependency Injection** - Component initialization order
- ✅ **Error Recovery** - Graceful degradation on failures
- ✅ **Resource Cleanup** - Proper resource release

**Class Diagram**:
```cpp
class Application {
    enum class State { UNINITIALIZED, INITIALIZING, RUNNING, STOPPING, STOPPED };
    
    // Core operations
    int run(int argc, char* argv[]);
    bool initialize();
    void shutdown();
    void waitForShutdown();
    
    // Configuration
    bool parseCommandLine(int argc, char* argv[]);
    bool loadConfiguration();
    bool applyConfigOverrides();
};
```

**Usage**:
```cpp
#include "main/Application.hpp"

int main(int argc, char* argv[]) {
    CppWebSocket::Application app;
    return app.run(argc, argv);
}
```

### **CommandLineParser.hpp**
**Command-line argument parsing and validation**

**Key Responsibilities**:
- Parse command-line arguments and options
- Validate argument combinations and values
- Generate help and version information
- Support configuration overrides via command line

**Key Features**:
- ✅ **Flexible Parsing** - Support for short/long options
- ✅ **Validation** - Argument value validation
- ✅ **Help System** - Automatic help generation
- ✅ **Override Support** - Runtime configuration changes

**Command Line Structure**:
```bash
./websocket-server --port 8080 --config server.json --daemon
./websocket-server --help
./websocket-server --version
./websocket-server --config.worker-threads=8 --config.max-connections=10000
```

**Usage**:
```cpp
CommandLineParser parser;
auto options = parser.parse(argc, argv);

if (options.help_requested) {
    parser.showHelp();
    return 0;
}

if (options.version_requested) {
    parser.showVersion();
    return 0;
}
```

### **ConfigOverrides.hpp**
**Runtime configuration override system**

**Key Responsibilities**:
- Apply command-line overrides to configuration
- Validate override keys and values
- Support nested configuration keys
- Type conversion and validation

**Key Features**:
- ✅ **Dot Notation** - Support for nested keys (`server.port`)
- ✅ **Type Safety** - Runtime type validation
- ✅ **Validation** - Value range and format checking
- ✅ **Fallback** - Default values for missing keys

**Supported Override Formats**:
```cpp
--port=8080                          // Simple value
--config.port=8080                   // Nested configuration
--server.max-connections=10000       // Kebab-case support
--log.level=debug                    // String values
--enable-compression=true            // Boolean values
```

**Usage**:
```cpp
ConfigOverrides overrides;
ServerConfig config;

std::unordered_map<std::string, std::string> cli_overrides = {
    {"server.port", "8080"},
    {"server.max-connections", "10000"},
    {"log.level", "debug"}
};

overrides.applyOverrides(config, cli_overrides);
```

### **Daemonizer.hpp**
**Daemon process management and supervision**

**Key Responsibilities**:
- Convert process to daemon (background) mode
- PID file management for process tracking
- Standard stream redirection
- Single instance enforcement

**Key Features**:
- ✅ **Daemon Mode** - Background process operation
- ✅ **PID Management** - Process tracking and control
- ✅ **Stream Redirection** - Log file output
- ✅ **Single Instance** - Prevent multiple server instances

**Daemonization Process**:
1. **Fork Process** - Create background process
2. **Create Session** - Detach from terminal
3. **Redirect Streams** - Point to /dev/null or log files
4. **Create PID File** - Track process ID
5. **Change Directory** - Set working directory

**Usage**:
```cpp
if (options.daemon_mode) {
    if (!Daemonizer::daemonize("/var/run/websocket-server")) {
        std::cerr << "Failed to daemonize process" << std::endl;
        return 1;
    }
    
    if (!Daemonizer::createPidFile()) {
        std::cerr << "Failed to create PID file" << std::endl;
        return 1;
    }
}
```

### **SignalHandler.hpp**
**System signal handling for graceful shutdown**

**Key Responsibilities**:
- Capture and process system signals
- Coordinate graceful shutdown sequence
- Handle emergency shutdown scenarios
- Support for custom signal handlers

**Key Features**:
- ✅ **Graceful Shutdown** - SIGTERM, SIGINT handling
- ✅ **Configuration Reload** - SIGHUP support
- ✅ **Custom Signals** - SIGUSR1, SIGUSR2 for custom actions
- ✅ **Thread Safety** - Safe signal handling in multi-threaded environment

**Supported Signals**:
- **SIGINT** (Ctrl+C) - Immediate graceful shutdown
- **SIGTERM** - Graceful shutdown request
- **SIGHUP** - Configuration reload
- **SIGUSR1/SIGUSR2** - Custom application signals

**Usage**:
```cpp
SignalHandler signal_handler;
signal_handler.setShutdownCallback([&app]() {
    app.requestShutdown();
});

signal_handler.setReloadCallback([&app]() {
    app.reloadConfiguration();
});

signal_handler.initialize();
```

## 🔄 Application Lifecycle

### **Startup Sequence**:
```
1. main() → Application::run()
2. CommandLineParser → Parse arguments
3. ConfigOverrides → Apply CLI overrides
4. Daemonizer → Convert to daemon (if requested)
5. SignalHandler → Register signal handlers
6. Application::initialize() → Setup components
7. WebSocketServer::start() → Begin serving
8. Application::waitForShutdown() → Main loop
```

### **Runtime Operation**:
```
Main Thread → SignalHandler → Event Loop
     ↓              ↓             ↓
  Wait for      Handle        Process
  Shutdown      Signals       Events
```

### **Shutdown Sequence**:
```
1. Signal Received → SignalHandler triggered
2. Application::requestShutdown() → Set shutdown flag
3. WebSocketServer::stop() → Stop accepting connections
4. Connection Cleanup → Close active connections
5. Resource Release → Free allocated resources
6. Process Exit → Return exit code
```

## 🛠️ Key Features and Capabilities

### **1. Production-Ready Operation**
- **Daemon Mode**: Background operation for servers
- **PID Files**: Process management and supervision
- **Log Rotation**: Integrated with logging system
- **Resource Limits**: Configurable memory and connection limits

### **2. Configuration Management**
```cpp
// Multiple configuration sources with precedence:
// 1. Command-line overrides (highest priority)
// 2. Configuration file values
// 3. Environment variables
// 4. Default values (lowest priority)
```

### **3. Graceful Shutdown**
```cpp
// Shutdown process:
// 1. Stop accepting new connections
// 2. Wait for active requests to complete (with timeout)
// 3. Close remaining connections gracefully
// 4. Release system resources
// 5. Exit with appropriate status code
```

### **4. Error Handling and Reporting**
```cpp
// Comprehensive error handling:
// - Configuration errors (invalid files, syntax errors)
// - Runtime errors (port in use, permission denied)
// - Resource errors (memory exhaustion, file descriptors)
// - Signal errors (handling failures)
```

## 🚀 Usage Examples

### **Complete Application Setup**:
```cpp
#include "main/Application.hpp"
#include "main/SignalHandler.hpp"

int main(int argc, char* argv[]) {
    try {
        CppWebSocket::Application app;
        
        // Run the application (blocks until shutdown)
        int exit_code = app.run(argc, argv);
        
        // Perform any final cleanup
        app.cleanup();
        
        return exit_code;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error" << std::endl;
        return 1;
    }
}
```

### **Custom Signal Handling**:
```cpp
class CustomApplication : public CppWebSocket::Application {
protected:
    void setupSignalHandlers() override {
        SignalHandler::getInstance().setShutdownCallback([this]() {
            this->onShutdownRequest();
        });
        
        SignalHandler::getInstance().setReloadCallback([this]() {
            this->onConfigReload();
        });
        
        SignalHandler::getInstance().setCustomCallback([this](int signal) {
            this->onCustomSignal(signal);
        });
    }
    
    void onShutdownRequest() {
        // Custom shutdown logic
        saveSessionData();
        Application::onShutdownRequest();
    }
};
```

### **Configuration Override Examples**:
```bash
# Development mode with verbose logging
./websocket-server --port 8080 --log-level debug --log-file console

# Production daemon mode
./websocket-server --config production.json --daemon --pid-file /var/run/websocket.pid

# Custom configuration with overrides
./websocket-server --config base.json --server.port=9090 --server.max-connections=5000
```

## 🔧 Integration with Other Layers

### **Core Layer Integration**:
```cpp
class Application {
    std::unique_ptr<WebSocketServer> server_;
    std::unique_ptr<Engine> engine_;
    
    bool initializeComponents() {
        // Initialize core engine
        if (!engine_->initialize()) {
            return false;
        }
        
        // Create and configure server
        server_ = std::make_unique<WebSocketServer>(getServerConfig());
        
        // Register event handlers
        setupEventHandlers();
        
        return true;
    }
};
```

### **Configuration Layer Integration**:
```cpp
void Application::loadConfiguration() {
    auto& global_config = GlobalConfig::getInstance();
    
    // Load from command-line specified file or default
    if (!cmd_options_.config_file.empty()) {
        global_config.initialize(cmd_options_.config_file);
    } else {
        global_config.initialize();
    }
    
    // Apply command-line overrides
    ConfigOverrides overrides;
    overrides.applyOverrides(global_config, cmd_options_.config_overrides);
}
```

### **Network Layer Integration**:
```cpp
void Application::setupServerConfiguration() {
    auto config = getConfigManager()->getServerConfig();
    
    // Configure network settings
    config.max_connections = getConfigValue<size_t>("server.max_connections", 10000);
    config.io_threads = getConfigValue<size_t>("server.io_threads", 0); // auto-detect
    config.worker_threads = getConfigValue<size_t>("server.worker_threads", 0);
    
    server_->setConfig(config);
}
```

## 📊 Operational Characteristics

### **Startup Performance**:
- **Configuration Loading**: < 100ms (file I/O dependent)
- **Component Initialization**: < 500ms
- **Signal Handler Setup**: < 10ms
- **Total Startup**: < 1 second typically

### **Resource Usage**:
- **Memory Overhead**: ~1MB for application layer
- **Thread Count**: 1 main thread + signal handler thread
- **File Descriptors**: Minimal (configuration files, PID file)

### **Error Scenarios**:
```cpp
// Common error handling:
switch (exit_code) {
    case 0:  // Success
        break;
    case 1:  // Configuration error
        logger->error("Invalid configuration");
        break;
    case 2:  // Network error
        logger->error("Network initialization failed");
        break;
    case 3:  // Resource error
        logger->error("Insufficient system resources");
        break;
    default: // Unknown error
        logger->error("Unexpected error occurred");
}
```

## 🔒 Security Considerations

- **Configuration Security**: Validate configuration file permissions
- **PID File Security**: Prevent PID file hijacking
- **Signal Security**: Only handle expected signals
- **Daemon Security**: Proper privilege dropping in daemon mode

## 🎯 Deployment Scenarios

### **Development Environment**:
```bash
./websocket-server --port 8080 --log-level debug --foreground
```

### **Production Environment**:
```bash
./websocket-server --config /etc/websocket/server.conf --daemon
```

### **Containerized Deployment**:
```bash
./websocket-server --config /app/config.json --foreground
```

### **Systemd Service**:
```ini
[Unit]
Description=WebSocket Server
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/websocket-server --config /etc/websocket/server.conf
Restart=always

[Install]
WantedBy=multi-user.target
```

---

The main application layer provides a robust, production-ready foundation for deploying the WebSocket server in various environments. Its comprehensive lifecycle management, configuration flexibility, and graceful operation make it suitable for everything from development testing to enterprise production deployments.