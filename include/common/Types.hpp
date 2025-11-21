#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace CppWebSocket {
    // Basic types
    using Byte = uint8_t;
    using ByteBuffer = std::vector<Byte>;
    using String = std::string;

    // Smart pointers
    template<typename T>
    using UniquePtr = std::unique_ptr<T>;

    template<typename T>
    using SharedPtr = std::shared_ptr<T>;

    template<typename T>
    using WeakPtr = std::weak_ptr<T>;

    // Functional types
    using Callback = std::function<void()>;
    template<typename T>
    using EventCallback = std::function<void(const T&)>;

    // Result types
    enum class Result {
        SUCCESS,
        ERROR,
        TIMEOUT,
        INVALID_STATE,
        CONNECTION_CLOSED
    };

    struct Error {
        int code;
        std::string message;
    };
}