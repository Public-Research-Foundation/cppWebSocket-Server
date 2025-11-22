CppWebSocket-Server/
├── 📁 include/
│   ├── 📁 common/                    # Shared utilities & base types
│   │   ├── Types.hpp                 # Fundamental types (Buffer, Result, etc.)
│   │   ├── ErrorCodes.hpp            # Error codes & exceptions
│   │   ├── NonCopyable.hpp           # Base class for non-copyable objects
│   │   └── Version.hpp               # Version information
│   │
│   ├── 📁 config/                    # Configuration system
│   │   ├── ConfigManager.hpp         # Central configuration management
│   │   ├── ServerConfig.hpp          # Server-specific settings
│   │   ├── SecurityConfig.hpp        # SSL, authentication settings
│   │   └── RuntimeConfig.hpp         # Hot-reloadable config
│   │
│   ├── 📁 constants/                 # Protocol constants
│   │   ├── WebSocketConstants.hpp    # RFC 6455 constants
│   │   ├── FrameOpcodes.hpp          # Frame types & opcodes
│   │   ├── StatusCodes.hpp           # Close status codes
│   │   └── Limits.hpp                # Size limits & timeouts
│   │
│   ├── 📁 core/                      # Core engine
│   │   ├── WebSocketServer.hpp       # Main server class
│   │   ├── Engine.hpp                # Core coordination engine
│   │   ├── ServiceLocator.hpp        # Dependency injection
│   │   └── LifecycleManager.hpp      # Startup/shutdown sequencing
│   │
│   ├── 📁 network/                   # Networking layer
│   │   ├── WebSocketSession.hpp      # Client session management
│   │   ├── ConnectionPool.hpp        # Connection resource pooling
│   │   ├── IOThreadPool.hpp          # Async I/O management
│   │   └── Endpoint.hpp              # Network address handling
│   │
│   ├── 📁 protocol/                  # WebSocket protocol
│   │   ├── WebSocketMessage.hpp      # Message handling
│   │   ├── WebSocketFrame.hpp        # Frame parsing/construction
│   │   ├── WebSocketHandshake.hpp    # HTTP upgrade handshake
│   │   └── ProtocolHandler.hpp       # Protocol state machine
│   │
│   ├── 📁 utils/                     # Utilities
│   │   ├── Logger.hpp                # Logging system
│   │   ├── Crypto.hpp                # SHA1, Base64, masking
│   │   ├── BufferPool.hpp            # Zero-copy buffer management
│   │   └── StringUtils.hpp           # String manipulation
│   │
│   ├── 📁 api/                       # Public API
│   │   ├── ServerAPI.hpp             # Main public interface
│   │   ├── MessageAPI.hpp            # Message sending interface
│   │   └── StatsAPI.hpp              # Statistics & monitoring
│   │
│   └── 📁 main/                      # Application entry & CLI handling  ← NEW
│       ├── Application.hpp            # Main application class
│       ├── CommandLineParser.hpp      # CLI argument parsing
│       ├── ConfigOverrides.hpp        # Configuration override system
│       ├── Daemonizer.hpp             # Daemon process management
│       └── SignalHandler.hpp          # Graceful shutdown handling
│
├── 📁 src/                           # Implementation
│   ├── 📁 common/
│   ├── 📁 config/
│   ├── 📁 core/
│   ├── 📁 network/
│   ├── 📁 protocol/
│   ├── 📁 utils/
│   ├── 📁 api/
│   └── 📁 main/                      # Application implementation  ← NEW
│
├── 📁 examples/                      # Usage examples
├── 📁 tests/                         # Unit & integration tests
├── 📁 third_party/                   # Dependencies
├── CMakeLists.txt
└── README.md