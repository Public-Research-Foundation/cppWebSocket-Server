# WebSocket Protocol Implementation

## 📋 Overview

This directory contains the complete WebSocket protocol implementation according to **RFC 6455**. The protocol layer handles all aspects of WebSocket communication including handshake negotiation, frame parsing, message reassembly, and protocol state management.

## 🏗️ Architecture

```
protocol/
├── WebSocketFrame.hpp      # Frame parsing and serialization
├── WebSocketMessage.hpp    # Message fragmentation/defragmentation  
├── WebSocketHandshake.hpp  # HTTP upgrade handshake handling
└── ProtocolHandler.hpp     # Main protocol state machine
```

## 📁 File Descriptions

### **WebSocketFrame.hpp**
**Purpose**: Low-level WebSocket frame handling according to RFC 6455 frame format.

**Key Features**:
- Frame parsing from raw bytes
- Frame serialization to bytes
- Masking/unmasking of payload data
- Frame validation and error checking
- Support for all frame types (data and control)

**Frame Structure**:
```
 0                   1                   2                   3
 0 1 2 3 4 5 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
:                     Payload Data continued ...                :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
```

### **WebSocketMessage.hpp**
**Purpose**: High-level message handling with support for fragmentation across multiple frames.

**Key Features**:
- Message reassembly from multiple frames
- Message fragmentation for large payloads
- UTF-8 validation for text messages
- Support for both text and binary messages
- Message size tracking and validation

**Fragmentation Example**:
```
Message: "Hello World" (split into 3 frames)
Frame 1: FIN=0, Opcode=TEXT, Payload="Hello"
Frame 2: FIN=0, Opcode=CONTINUATION, Payload=" Wor"
Frame 3: FIN=1, Opcode=CONTINUATION, Payload="ld"
```

### **WebSocketHandshake.hpp**
**Purpose**: HTTP upgrade handshake processing for WebSocket protocol negotiation.

**Key Features**:
- HTTP request parsing and validation
- WebSocket key generation and verification
- Subprotocol negotiation support
- Origin validation
- RFC 6455 compliance checking

**Handshake Process**:
1. **Client Request**:
   ```
   GET /chat HTTP/1.1
   Host: server.example.com
   Upgrade: websocket
   Connection: Upgrade
   Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
   Sec-WebSocket-Version: 13
   ```

2. **Server Response**:
   ```
   HTTP/1.1 101 Switching Protocols
   Upgrade: websocket
   Connection: Upgrade
   Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
   ```

### **ProtocolHandler.hpp**
**Purpose**: Main protocol state machine that orchestrates all WebSocket communication.

**Key Features**:
- Complete protocol state management (CONNECTING, OPEN, CLOSING, CLOSED)
- Frame processing and routing
- Control frame handling (PING/PONG/CLOSE)
- Error handling and connection closure
- Event callback system for application integration

**State Machine**:
```
CONNECTING → OPEN → CLOSING → CLOSED
    ↓          ↓        ↓        ↓
 Handshake   Data    Close    Terminal
  Phase      Exchange Handshake State
```

## 🔧 Usage Examples

### Basic Protocol Usage
```cpp
#include "protocol/ProtocolHandler.hpp"

// Create protocol handler
websocket::ProtocolHandler handler;

// Set up callbacks
websocket::ProtocolHandler::Callbacks callbacks;
callbacks.on_message = [](const auto& message) {
    std::cout << "Received: " << message.getText() << std::endl;
};
callbacks.on_close = [](auto code, auto reason) {
    std::cout << "Connection closed: " << reason << std::endl;
};
handler.setCallbacks(callbacks);

// Process handshake
std::string http_request = "GET /chat HTTP/1.1\r\n...";
auto result = handler.processHandshake(http_request);
if (result == websocket::WebSocketHandshake::Result::SUCCESS) {
    std::string response = handler.getHandshakeResponse();
    // Send response to client
}

// Process incoming data
std::vector<uint8_t> data = receiveFromNetwork();
size_t consumed = handler.processData(data);

// Send messages
auto frame = handler.createTextFrame("Hello World");
sendToNetwork(frame);
```

### Frame-Level Usage
```cpp
#include "protocol/WebSocketFrame.hpp"

// Parse incoming frame
websocket::WebSocketFrame frame;
std::vector<uint8_t> raw_data = /* from network */;
size_t bytes_used = websocket::WebSocketFrame::parse(raw_data, frame);

if (bytes_used > 0) {
    if (frame.validate()) {
        // Process valid frame
        if (frame.isControlFrame()) {
            // Handle control frame (PING, PONG, CLOSE)
        } else {
            // Handle data frame
        }
    }
}

// Create outgoing frame
websocket::WebSocketFrame response(
    websocket::Opcode::TEXT, 
    {"Hello"}, 
    true,  // FIN
    false  // masked
);
auto serialized = response.serialize();
```

## 🛠️ API Reference

### WebSocketFrame
```cpp
// Parsing
static size_t parse(const Buffer& data, WebSocketFrame& frame);

// Serialization
Buffer serialize() const;

// Validation
bool validate() const;

// Properties
bool getFin() const;
Opcode getOpcode() const;
const Buffer& getPayload() const;
```

### WebSocketMessage
```cpp
// Construction
WebSocketMessage(const Buffer& data, Type type = Type::TEXT);
explicit WebSocketMessage(const std::string& text);

// Frame management
bool addFrame(const WebSocketFrame& frame);
bool isComplete() const;
std::vector<WebSocketFrame> toFrames(size_t max_frame_size) const;

// Data access
const Buffer& getData() const;
std::string getText() const;
```

### WebSocketHandshake
```cpp
// Handshake processing
Result parseRequest(const std::string& request);
std::string createResponse();
Result validate() const;

// Subprotocol negotiation
std::vector<std::string> getRequestedSubprotocols() const;
void setAcceptedSubprotocol(const std::string& protocol);
```

### ProtocolHandler
```cpp
// Lifecycle
size_t processData(const Buffer& data);
WebSocketHandshake::Result processHandshake(const std::string& request);
std::string getHandshakeResponse();

// Message creation
Buffer createTextFrame(const std::string& text);
Buffer createBinaryFrame(const Buffer& data);
Buffer createCloseFrame(uint16_t code, const std::string& reason);

// State management
State getState() const;
bool isOpen() const;
void initiateClose(uint16_t code, const std::string& reason);
```

## 🔒 Protocol Compliance

This implementation is fully compliant with **RFC 6455** and includes:

- ✅ **Proper handshake** with key verification
- ✅ **Frame masking** for client-to-server messages
- ✅ **Fragmentation support** for large messages
- ✅ **Control frames** (PING, PONG, CLOSE)
- ✅ **Status codes** for connection closure
- ✅ **UTF-8 validation** for text messages
- ✅ **Protocol version** negotiation
- ✅ **Error handling** and connection cleanup

## 🚀 Performance Features

- **Zero-copy parsing** where possible
- **Buffer reuse** for frame serialization
- **Efficient masking** operations
- **Minimal memory allocations**
- **Batch processing** of multiple frames

## 🧪 Testing

The protocol implementation includes comprehensive unit tests covering:

- Frame parsing edge cases
- Message fragmentation scenarios
- Handshake validation
- Protocol state transitions
- Error conditions and recovery
- Performance benchmarks

See the `tests/` directory for detailed test cases.

## 📚 References

- [RFC 6455 - The WebSocket Protocol](https://tools.ietf.org/html/rfc6455)
- [WebSocket API - W3C Specification](https://www.w3.org/TR/websockets/)
- [Mozilla WebSocket Documentation](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket)

---

**Next Steps**: See the [Network Layer](../network/README.md) for how the protocol integrates with the networking stack.