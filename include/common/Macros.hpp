#pragma once

#define WEBSOCKET_NAMESPACE_BEGIN namespace CppWebSocket {
#define WEBSOCKET_NAMESPACE_END }

#define WEBSOCKET_DISABLE_COPY(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete;

#define WEBSOCKET_DISABLE_MOVE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;

#define WEBSOCKET_DEFAULT_COPY(ClassName) \
    ClassName(const ClassName&) = default; \
    ClassName& operator=(const ClassName&) = default;

#define WEBSOCKET_DEFAULT_MOVE(ClassName) \
    ClassName(ClassName&&) = default; \
    ClassName& operator=(ClassName&&) = default;

#define WEBSOCKET_INTERFACE(ClassName) \
    public: \
        virtual ~ClassName() = default; \
    protected: \
        ClassName() = default;

#ifdef _WIN32
#define WEBSOCKET_EXPORT __declspec(dllexport)
#define WEBSOCKET_IMPORT __declspec(dllimport)
#else
#define WEBSOCKET_EXPORT __attribute__((visibility("default")))
#define WEBSOCKET_IMPORT
#endif