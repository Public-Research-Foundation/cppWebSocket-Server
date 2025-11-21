#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "interfaces/IProtocol.hpp"
#include <unordered_map>
#include <memory>
#include <shared_mutex>

WEBSOCKET_NAMESPACE_BEGIN

class ProtocolRegistry {
public:
    static ProtocolRegistry& getInstance();

    // Protocol registration
    Result registerProtocol(const std::string& name, SharedPtr<IProtocol> protocol);
    Result unregisterProtocol(const std::string& name);
    bool isProtocolRegistered(const std::string& name) const;

    // Protocol access
    SharedPtr<IProtocol> getProtocol(const std::string& name) const;
    std::vector<std::string> getRegisteredProtocols() const;
    std::vector<SharedPtr<IProtocol>> getAllProtocols() const;

    // Protocol discovery
    SharedPtr<IProtocol> findProtocolForHandshake(const ByteBuffer& handshakeData) const;
    SharedPtr<IProtocol> findProtocolByName(const std::string& name) const;
    std::vector<std::string> findProtocolsByVersion(const std::string& version) const;

    // Protocol negotiation
    std::string negotiateProtocol(const std::vector<std::string>& clientProtocols) const;
    bool supportsProtocol(const std::string& protocol) const;

    // Default protocol management
    void setDefaultProtocol(const std::string& name);
    SharedPtr<IProtocol> getDefaultProtocol() const;
    std::string getDefaultProtocolName() const;

    // Protocol lifecycle
    Result initializeAllProtocols();
    Result shutdownAllProtocols();
    void configureAllProtocols(const std::unordered_map<std::string, std::any>& config);

    // Statistics
    struct RegistryStats {
        size_t totalProtocols;
        size_t activeProtocols;
        size_t initializedProtocols;
        std::vector<std::string> protocolNames;
    };

    RegistryStats getStats() const;

    // Protocol validation
    bool validateProtocol(SharedPtr<IProtocol> protocol) const;
    std::vector<std::string> getProtocolValidationErrors(SharedPtr<IProtocol> protocol) const;

private:
    ProtocolRegistry();
    ~ProtocolRegistry() = default;

    mutable std::shared_mutex mutex_;

    std::unordered_map<std::string, SharedPtr<IProtocol>> protocols_;
    std::string defaultProtocol_;

    std::atomic<size_t> totalProtocols_{ 0 };
    std::atomic<size_t> activeProtocols_{ 0 };
    std::atomic<size_t> initializedProtocols_{ 0 };

    bool isValidProtocolName(const std::string& name) const;
    void updateProtocolStats();
};

WEBSOCKET_NAMESPACE_END