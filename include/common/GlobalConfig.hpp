#pragma once

#include <string>
#include <memory>
#include "config/ConfigManager.hpp"

namespace CppWebSocket {
    class GlobalConfig {
    public:
        static GlobalConfig& getInstance();

        void initialize(const std::string& configPath = "");
        void reload();
        void setRuntimeOverride(const std::string& key, const std::string& value);

        std::shared_ptr<ConfigManager> getConfigManager();

    private:
        GlobalConfig() = default;
        ~GlobalConfig() = default;

        std::shared_ptr<ConfigManager> configManager_;
        std::string configPath_;
    };
}