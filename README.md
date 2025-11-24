## 📋 RFC 6455 Compliance

### Immutable Protocol Constants

The server strictly adheres to RFC 6455 standards with separate header files for different protocol aspects:

#### Core Protocol Constants
```cpp
namespace RFC {

    constexpr uint8_t FIN_BIT = 0x80;
    constexpr uint8_t RSV1_BIT = 0x40;

}
```

#### Frame Type Definitions
```cpp
namespace FrameOpcode {
    constexpr uint8_t CONTINUATION = 0x00;
    constexpr uint8_t PING = 0x09;
    constexpr uint8_t PONG = 0x0A;
    
    // Validation utilities
    constexpr bool isControlFrame(uint8_t opcode) {
        return opcode >= CLOSE;
    }
    
    constexpr bool isDataFrame(uint8_t opcode) {
        return opcode == TEXT || opcode == BINARY || opcode == CONTINUATION;
    }
}
```

#### Standard Close Status Codes
```cpp
namespace Code {
    constexpr uint16_t NORMAL_CLOSURE = 1000;
    constexpr uint16_t GOING_AWAY = 1001;
    constexpr uint16_t PROTOCOL_ERROR = 1002;
    
    constexpr bool isValidCloseCode(uint16_t code) {
        return (code >= 1000 && code <= 1011 && code != 1004 && code != 1005 && code != 1006) ||
               (code >= 3000 && code <= 4999) ||
               code == 1015;
    }
}
```

#### Maximum Frame Sizes and Timeouts
```cpp
namespace Limits {
    // Frame size limits
    constexpr size_t MAX_FRAME_HEADER_SIZE = 14; // bytes
    constexpr size_t MAX_CONTROL_FRAME_PAYLOAD = 125; // bytes

    
    // Protocol timeouts
    constexpr uint32_t HANDSHAKE_TIMEOUT_MS = 5000;
    constexpr uint32_t PONG_TIMEOUT_MS = 30000;
    
    // Connection limits
    constexpr uint32_t MAX_HEADER_SIZE = 8192; // bytes
    constexpr uint32_t MAX_HEADER_FIELD_SIZE = 4096; // bytes
}
```

## ⚙️ System

### Three-Tier Configuration Hierarchy

```
RFC 6455 Standards (Immutable)
        ↓
Compile-time Defaults (Build-time)
        ↓
Runtime Configuration (JSON/XML/Env)
```

### Quick Configuration Examples

**Compile-time Configuration** (`include/config/NetworkDefaults.hpp`):
```cpp
namespace NetworkDefaults {
    constexpr uint16_t PORT = 8080;
    constexpr uint32_t MAX_CONNECTIONS = 10000;
    constexpr size_t BUFFER_SIZE = 8192;
    constexpr uint32_t HANDSHAKE_TIMEOUT_MS = 5000;
}
```

**Runtime Configuration** (`config/server.json`):
```json
{
  "server": {
    "port": 9000,
    "io_threads": 4,
    "max_connections": 5000
  },
  "session": {
    "timeout_seconds": 3600,
    "heartbeat_interval": 30
  },
  "protocol": {
    "max_frame_size": 16777216,
    "enable_compression": true,
    "validate_utf8": true
  },
  "security": {
    "enable_tls": true,
    "cert_file": "/path/to/cert.pem"
  }
}
```

**Environment Overrides**:
```bash
export CWS_PORT=8080
export CWS_THREADS=8
export CWS_MAX_CONN=10000
export CWS_MAX_FRAME_SIZE=16777216
```

## 🚀 Quick Start

### Building from Source

```bash
# Clone and build
git clone https://github.com/your-org/CppWebSocket-Server.git
cd CppWebSocket-Server

# Configure build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run with default configuration
./build/bin/websocket-server

# Run with custom config
./build/bin/websocket-server --config config/production.json
```

### Basic Usage Example

```cpp
#include <server/WebSocketServer.h>
#include <server/MessageAPI.h>
#include <server/constants/FrameOpcode.hpp>
#include <server/constants/CloseCode.hpp>

// Create server instance
auto server = WebSocketServer::create();

// Configure runtime settings
server->configure()
    .port(8080)
    .ioThreads(4)
    .maxConnections(10000)
    .maxFrameSize(ProtocolLimits::MAX_FRAME_PAYLOAD_16BIT);

// Set message handlers
server->onMessage([](auto session, auto message) {
    if (message.opcode == FrameOpcode::TEXT) {
        session->send("Echo: " + message.data);
    }
});

server->onClose([](auto session, auto code) {
    if (code != CloseCode::NORMAL_CLOSURE) {
        std::cout << "Abnormal closure with code: " << code << std::endl;
    }
});

// Start the server
server->start();
```

## 📊 Performance Benchmarks

| Metric | Value |
|--------|-------|
| **Max Concurrent Connections** | 10,000+ |
| **Message Throughput** | 100,000+ msg/sec |
| **Memory Footprint** | < 50MB base |
| **Connection Handshake** | < 5ms |
| **Startup Time** | < 100ms |
| **Frame Processing** | < 1μs per frame |

## 🔧 Core Components

### Protocol Layer
- **WebSocketFrame** - RFC-compliant frame parsing/construction using `FrameOpcode`
- **WebSocketHandshake** - HTTP upgrade handshake processing
- **ProtocolHandler** - WebSocket state machine with `ProtocolLimits` enforcement
- **MessageRouter** - Efficient message distribution

### Network Layer
- **SessionManager** - Client session lifecycle management
- **ConnectionManager** - Resource pooling and connection reuse
- **IOThreadPool** - Async I/O with configurable threading
- **NetworkAcceptor** - TCP/SSL connection acceptance

### Configuration Management
- **ConfigManager** - Centralized configuration handling
- **RuntimeConfig** - Hot-reloadable settings
- **ConfigValidator** - Configuration integrity checking

## 🛠️ Advanced Configuration

### Custom Protocol Extensions

```cpp
#include <server/constants/FrameOpcode.hpp>
#include <server/constants/CloseCode.hpp>

// Custom message handler with protocol constants
class CustomMessageHandler : public MessageHandler {
public:
    void onTextMessage(SessionPtr session, const std::string& message) override {
        if (message.size() > ProtocolLimits::MAX_FRAME_PAYLOAD_16BIT) {
            session->close(CloseCode::MESSAGE_TOO_BIG);
            return;
        }
        
        auto response = processBusinessLogic(message);
        session->send(response, FrameOpcode::TEXT);
    }
};

// Register custom handler
server->registerMessageHandler(std::make_shared<CustomMessageHandler>());
```

### SSL/TLS Configuration

```json
{
  "security": {
    "enable_tls": true,
    "certificate": {
      "cert_file": "/path/to/cert.pem",
      "key_file": "/path/to/key.pem",
      "ca_file": "/path/to/ca.pem"
    },
    "ciphers": "ECDHE-RSA-AES128-GCM-SHA256",
    "protocols": "TLSv1.2,TLSv1.3"
  }
}
```

## 🧪 Testing & Validation

### RFC Compliance Testing

```bash
# Run comprehensive test suite
./tests/compliance/rfc6455_compliance_test

# Test specific protocol components
./tests/protocol/frame_opcode_test
./tests/protocol/close_code_validation_test
./tests/protocol/protocol_limits_test

# Performance benchmarking
./tests/performance/benchmark --connections=1000 --duration=60

# Configuration validation
./scripts/validate_config.py config/production.json
```

### Custom Test Scenarios

```bash
# Stress testing with protocol validation
./tests/load/stress_test --clients=5000 --message-rate=10000 --validate-frames

# Protocol fuzzing
./tests/security/protocol_fuzzer --iterations=100000

# Frame size limit testing
./tests/protocol/frame_size_test --max-size=16777216
```

## 🔒 Security Features

- **Frame Validation** - Strict RFC 6455 frame processing using `FrameOpcode` and `ProtocolLimits`
- **Size Limits** - Configurable message and frame size limits
- **TLS 1.2/1.3** - Modern encryption protocols
- **Input Sanitization** - Comprehensive message validation
- **Rate Limiting** - Configurable connection and message rate limits
- **Close Code Validation** - Proper connection termination using `CloseCode`

## 📈 Monitoring & Metrics

```cpp
#include <server/constants/FrameOpcode.hpp>

// Access real-time statistics with protocol insights
auto stats = server->getStatistics();

std::cout << "Active connections: " << stats.active_connections << std::endl;
std::cout << "Messages processed: " << stats.messages_processed << std::endl;
std::cout << "Text frames: " << stats.frames_by_opcode[FrameOpcode::TEXT] << std::endl;
std::cout << "Binary frames: " << stats.frames_by_opcode[FrameOpcode::BINARY] << std::endl;
std::cout << "Protocol errors: " << stats.close_codes[CloseCode::PROTOCOL_ERROR] << std::endl;
std::cout << "Memory usage: " << stats.memory_usage_mb << " MB" << std::endl;
```

## 🔄 API Reference

### Server Management
```cpp
// Server lifecycle
server->start();
server->stop();
server->restart();

// Configuration
server->reloadConfig("new_config.json");

// Session management
server->getActiveSessions();
server->closeSession(session_id, CloseCode::GOING_AWAY);
```

### Message Handling
```cpp
#include <server/constants/FrameOpcode.hpp>

// Message broadcasting with different opcodes
server->broadcast("Hello all!", FrameOpcode::TEXT);
server->broadcast(binary_data, data_size, FrameOpcode::BINARY);

// Targeted messages
server->sendToSession(session_id, "Private message", FrameOpcode::TEXT);
server->sendToGroup(group_id, binary_data, data_size, FrameOpcode::BINARY);

// Control frames
server->sendPing(session_id, "ping data");
```

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

```bash
# Setup development environment
./scripts/setup_dev.sh

# Build with tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run protocol compliance tests
./build/tests/protocol/compliance_test
./build/tests/protocol/frame_validation_test
```

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

---

**CppWebSocket-Server** - Enterprise-grade WebSocket server for high-performance real-time applications. Built with modern C++ and strict RFC 6455 compliance using properly separated protocol constants.
```