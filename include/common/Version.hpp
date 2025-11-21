#pragma once

#include "Macros.hpp"

WEBSOCKET_NAMESPACE_BEGIN

struct Version {
    static const int MAJOR = 1;
    static const int MINOR = 0;
    static const int PATCH = 0;

    static std::string toString() {
        return std::to_string(MAJOR) + "." +
            std::to_string(MINOR) + "." +
            std::to_string(PATCH);
    }

    static std::string getFullVersion() {
        return "CppWebSocket-Server v" + toString();
    }
};

WEBSOCKET_NAMESPACE_END