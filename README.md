# CppWebSocket-Server

A highly configurable, RFC-compliant C++ WebSocket server implementation designed for maximum flexibility and standards compliance. This server provides a generic foundation that can be adapted for various real-time communication scenarios while strictly adhering to WebSocket protocol standards.

## 🏗️ Project Structure

```
CppWebSocket-Server/
├── include/
│   ├── server/
│   │   ├── core/
│   │   │   ├── interfaces/
│   │   │   │   ├── IStartable.hpp
│   │   │   │   ├── IStoppable.hpp
│   │   │   │   └── IConfigurable.hpp
│   │   │   ├── ServerBase.hpp
│   │   │   ├── LifecycleManager.hpp
│   │   │   └── ComponentRegistry.hpp
│   │   ├── network/
│   │   │   ├── Connection.hpp
│   │   │   ├── ProtocolHandler.hpp
│   │   │   ├── FrameBuilder.hpp
│   │   │   └── SecurityManager.hpp
│   │   ├── session/
│   │   │   ├── Session.hpp
│   │   │   ├── SessionController.hpp
│   │   │   └── StorageBackend.hpp
│   │   ├── messaging/
│   │   │   ├── Message.hpp
│   │   │   ├── Router.hpp
│   │   │   └── HandlerRegistry.hpp
│   │   ├── utils/
│   │   │   ├── Logger.hpp
│   │   │   ├── ConfigParser.hpp
│   │   │   └── Metrics.hpp
│   │   └── constants/                   # RFC STANDARDS
│   │       ├── WebSocketRFC.hpp         # RFC 6455 Constants
│   │       ├── FrameOpcode.hpp          # Frame type definitions
│   │       ├── CloseCode.hpp            # Status codes
│   │       ├── HttpStatus.hpp           # HTTP status codes
│   │       └── ProtocolLimits.hpp       # Size limits and timeouts
│   ├── config/
│   │   ├── ServerDefaults.hpp           # Default server settings
│   │   ├── NetworkDefaults.hpp          # Network configuration
│   │   ├── SessionDefaults.hpp          # Session management defaults
│   │   ├── SecurityDefaults.hpp         # Security parameters
│   │   └── RuntimeConfig.hpp            # Modifiable at runtime
│   └── third_party/
├── src/
│   ├── core/
│   ├── network/
│   ├── session/
│   ├── messaging/
│   ├── utils/
│   └── main.cpp
├── config/
│   ├── presets/
│   │   ├── development.json
│   │   ├── production.json
│   │   └── testing.json
│   ├── templates/
│   │   ├── server.conf.template
│   │   ├── network.conf.template
│   │   └── security.conf.template
│   └── overrides/
│       └── custom.conf                   # User modifications
├── resources/
│   ├── rfc/
│   │   └── rfc6455.txt                   # Protocol specification
│   └── docs/
│       └── configuration-guide.md
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── compliance/                       # RFC compliance tests
│   └── performance/
├── scripts/
│   ├── build/
│   ├── deployment/
│   └── validation/                       # Config validation
└── docs/
    ├── api/
    ├── configuration/
    └── compliance/                       # RFC compliance docs
```

## 📋 RFC Standards Compliance

### Strictly Immutable RFC Constants

All WebSocket protocol constants defined in **RFC 6455** are stored in read-only headers:

- **`WebSocketRFC.hpp`** - Core protocol constants
- **`FrameOpcode.hpp`** - Frame type definitions (TEXT, BINARY, CLOSE, etc.)
- **`CloseCode.hpp`** - Standard close status codes
- **`ProtocolLimits.hpp`** - Maximum frame sizes, timeouts

Example usage:
```cpp
#include <server/constants/WebSocketRFC.hpp>
#include <server/constants/FrameOpcode.hpp>

// RFC values are constexpr and immutable
constexpr auto MAGIC_STRING = WebSocketRFC::MAGIC_STRING;
constexpr auto MAX_FRAME_SIZE = ProtocolLimits::MAX_FRAME_SIZE;
```

### Configurable Parameters

Separate headers for modifiable constants:

- **`ServerDefaults.hpp`** - Server behavior defaults
- **`NetworkDefaults.hpp`** - Network stack configuration
- **`SessionDefaults.hpp`** - Session management parameters
- **`SecurityDefaults.hpp`** - Security and TLS settings

## ⚙️ Configuration System

### Three-Tier Configuration

1. **RFC Standards** - Immutable protocol constants
2. **Compile-time Defaults** - Modifiable through header files
3. **Runtime Configuration** - JSON/XML config files

### Configuration Hierarchy

```
RFC Standards (Immutable)
        ↓
Compile-time Defaults (Modifiable)
        ↓
Runtime Configuration (User-modifiable)
        ↓
Environment Variables (Optional overrides)
```

### Key Configurable Areas

#### Network Configuration (`config/NetworkDefaults.hpp`)
```cpp
namespace NetworkDefaults {
    constexpr uint16_t DEFAULT_PORT = 8080;
    constexpr uint32_t MAX_CONNECTIONS = 10000;
    constexpr size_t READ_BUFFER_SIZE = 8192;
    constexpr uint32_t HANDSHAKE_TIMEOUT_MS = 5000;
    // ... more network parameters
}
```

#### Session Configuration (`config/SessionDefaults.hpp`)
```cpp
namespace SessionDefaults {
    constexpr uint32_t SESSION_TIMEOUT_SEC = 3600;
    constexpr uint32_t HEARTBEAT_INTERVAL_SEC = 30;
    constexpr size_t MAX_SESSION_ATTRIBUTES = 50;
    constexpr size_t MAX_MESSAGE_QUEUE_SIZE = 1000;
    // ... more session parameters
}
```

## 🚀 Quick Start

### Building the Server

```bash
# Clone and setup
git clone https://github.com/your-org/CppWebSocket-Server.git
cd CppWebSocket-Server

# Configure with defaults
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run compliance tests
./build/tests/compliance/rfc_compliance_test
```

### Basic Configuration

Create `config/overrides/custom.conf`:
```json
{
  "server": {
    "port": 8080,
    "io_threads": 4
  },
  "session": {
    "timeout_seconds": 1800,
    "storage_backend": "memory"
  },
  "security": {
    "enable_tls": true,
    "cert_file": "/path/to/cert.pem"
  }
}
```

### Running with Custom Config

```bash
# Use development preset
./build/bin/WebSocketServer --config config/presets/development.json

# With custom overrides
./build/bin/WebSocketServer --config config/overrides/custom.conf

# With environment overrides
CWS_PORT=9000 CWS_THREADS=8 ./build/bin/WebSocketServer
```

## 🔧 Configuration Management

### Modifying Constants

**For compile-time changes:** Edit header files in `include/config/`
```cpp
// In include/config/SessionDefaults.hpp
namespace SessionDefaults {
    constexpr uint32_t SESSION_TIMEOUT_SEC = 7200;  // 2 hours
    constexpr uint32_t HEARTBEAT_INTERVAL_SEC = 60; // 1 minute
}
```

**For runtime changes:** Use JSON configuration
```json
{
  "session": {
    "timeout_seconds": 7200,
    "heartbeat_interval": 60
  }
}
```

### Configuration Validation

```bash
# Validate configuration
./scripts/validation/validate_config.py config/overrides/custom.conf

# Check RFC compliance
./scripts/validation/compliance_check.py --strict
```

## 📚 Key Features

### RFC 6455 Compliance
- Full WebSocket protocol implementation
- Proper handshake processing
- Frame masking and unmasking
- Fragmentation support
- Ping/Pong heartbeat mechanism

### Generic Architecture
- Pluggable components
- Interface-based design
- Multiple storage backends
- Custom protocol extensions

### Flexible Configuration
- Compile-time defaults
- Runtime configuration files
- Environment variable overrides
- Configuration validation

## 🧪 Testing & Compliance

### RFC Compliance Testing
```bash
# Run all compliance tests
./tests/compliance/run_compliance_suite.sh

# Test specific RFC sections
./tests/compliance/test_handshake_compliance
./tests/compliance/test_framing_compliance
```

### Custom Test Configurations
```bash
# Test with different configurations
./scripts/testing/run_with_config.sh tests/config/stress_test.json
```

## 🔒 Security & Standards

- **TLS/SSL Support** - Secure WebSocket connections (wss://)
- **Input Validation** - RFC-compliant message processing
- **Size Limits** - Configurable frame and message limits
- **Authentication** - Pluggable auth providers

## 🤝 Contributing

When contributing, please ensure:

1. **RFC Compliance** - All changes must maintain protocol standards
2. **Configuration Separation** - New constants go in appropriate headers
3. **Backward Compatibility** - Don't break existing configurations
4. **Documentation** - Update relevant configuration guides

### Adding New Configuration Parameters

1. Add to appropriate `*Defaults.hpp` file
2. Update configuration parser
3. Add to configuration templates
4. Document in configuration guide

## 📄 License

MIT License - see LICENSE file for details.

---

**CppWebSocket-Server** - A generic, RFC-compliant WebSocket server built for flexibility and standards compliance.