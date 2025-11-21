#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include "interfaces/IEngine.hpp"
#include "interfaces/IConfigurable.hpp"
#include "ServiceLocator.hpp"
#include <atomic>
#include <memory>
#include <unordered_map>

WEBSOCKET_NAMESPACE_BEGIN

class Engine : public IEngine, public IConfigurable {
public:
    Engine();
    ~Engine() override;

    // IEngine implementation
    Result initialize() override;
    Result start() override;
    Result stop() override;
    Result shutdown() override;

    bool isRunning() const override;
    std::string getName() const override;
    std::string getStatus() const override;

    Result processMessage(const ByteBuffer& message) override;
    Result broadcastMessage(const ByteBuffer& message) override;

    size_t getConnectionCount() const override;
    size_t getMessageCount() const override;

    void setMaxConnections(size_t max) override;
    void setMaxMessageSize(size_t max) override;

    // IConfigurable implementation
    void configure(const std::unordered_map<std::string, std::any>& config) override;
    std::unordered_map<std::string, std::any> getCurrentConfig() const override;
    bool validateConfig(const std::unordered_map<std::string, std::any>& config) const override;
    void onConfigChanged(const std::string& key, const std::any& value) override;

private:
    WEBSOCKET_DISABLE_COPY(Engine)

        std::atomic<bool> isRunning_{ false };
    std::atomic<bool> isInitialized_{ false };
    std::atomic<size_t> connectionCount_{ 0 };
    std::atomic<size_t> messageCount_{ 0 };
    std::atomic<size_t> maxConnections_{ 1000 };
    std::atomic<size_t> maxMessageSize_{ 1048576 };

    SharedPtr<ServiceLocator> serviceLocator_;

    void initializeComponents();
    void shutdownComponents();
    void handleError(const Error& error);
};

WEBSOCKET_NAMESPACE_END