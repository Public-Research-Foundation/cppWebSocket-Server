#pragma once
#ifndef WEBSOCKET_MACROS_HPP
#define WEBSOCKET_MACROS_HPP

/**
 * @file Macros.hpp
 * @brief Project-wide macros and compiler-specific definitions
 *
 * This header provides:
 * - Namespace helpers
 * - Class behavior control macros
 * - Compiler-specific attributes
 * - Debug and assertion macros
 * - Platform abstraction macros
 */

 // ============================================================================
 // NAMESPACE MACROS
 // ============================================================================

 /**
  * @brief Begin the WebSocket namespace
  */
#define WEBSOCKET_NAMESPACE_BEGIN namespace CppWebSocket {

  /**
   * @brief End the WebSocket namespace
   */
#define WEBSOCKET_NAMESPACE_END }

   // ============================================================================
   // CLASS BEHAVIOR CONTROL
   // ============================================================================

   /**
    * @brief Disable copy constructor and assignment operator
    * @param ClassName Name of the class to modify
    */
#define WEBSOCKET_DISABLE_COPY(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete;

    /**
     * @brief Disable move constructor and assignment operator
     * @param ClassName Name of the class to modify
     */
#define WEBSOCKET_DISABLE_MOVE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;

     /**
      * @brief Enable default copy constructor and assignment operator
      * @param ClassName Name of the class to modify
      */
#define WEBSOCKET_DEFAULT_COPY(ClassName) \
    ClassName(const ClassName&) = default; \
    ClassName& operator=(const ClassName&) = default;

      /**
       * @brief Enable default move constructor and assignment operator
       * @param ClassName Name of the class to modify
       */
#define WEBSOCKET_DEFAULT_MOVE(ClassName) \
    ClassName(ClassName&&) = default; \
    ClassName& operator=(ClassName&&) = default;

       /**
        * @brief Make class non-copyable and non-movable
        * @param ClassName Name of the class to modify
        */
#define WEBSOCKET_NON_COPYABLE(ClassName) \
    WEBSOCKET_DISABLE_COPY(ClassName) \
    WEBSOCKET_DISABLE_MOVE(ClassName)

        /**
         * @brief Define interface class (abstract base class)
         * @param ClassName Name of the interface class
         */
#define WEBSOCKET_INTERFACE(ClassName) \
    public: \
        virtual ~ClassName() = default; \
    protected: \
        ClassName() = default; \
    private: \
        WEBSOCKET_DISABLE_COPY(ClassName) \
        WEBSOCKET_DISABLE_MOVE(ClassName)

         // ============================================================================
         // DLL EXPORT/IMPORT (WINDOWS)
         // ============================================================================

#ifdef _WIN32
#ifdef WEBSOCKET_BUILD_DLL
#define WEBSOCKET_EXPORT __declspec(dllexport)
#else
#define WEBSOCKET_EXPORT __declspec(dllimport)
#endif
#define WEBSOCKET_IMPORT __declspec(dllimport)
#else
#define WEBSOCKET_EXPORT __attribute__((visibility("default")))
#define WEBSOCKET_IMPORT
#endif

// ============================================================================
// COMPILER-SPECIFIC ATTRIBUTES
// ============================================================================

#ifdef __GNUC__
#define WEBSOCKET_LIKELY(x)   __builtin_expect(!!(x), 1)
#define WEBSOCKET_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define WEBSOCKET_NOINLINE    __attribute__((noinline))
#define WEBSOCKET_ALWAYS_INLINE __attribute__((always_inline))
#define WEBSOCKET_NORETURN    __attribute__((noreturn))
#else
#define WEBSOCKET_LIKELY(x)   (x)
#define WEBSOCKET_UNLIKELY(x) (x)
#define WEBSOCKET_NOINLINE
#define WEBSOCKET_ALWAYS_INLINE
#define WEBSOCKET_NORETURN
#endif

// ============================================================================
// DEBUG AND ASSERTION MACROS
// ============================================================================

#ifdef NDEBUG
#define WEBSOCKET_ASSERT(condition, message) ((void)0)
#define WEBSOCKET_DEBUG_LOG(message) ((void)0)
#else
#include <cassert>
#include <iostream>
#define WEBSOCKET_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "Assertion failed: " << #condition << " - " << message << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
                assert(condition); \
            } \
        } while (false)

#define WEBSOCKET_DEBUG_LOG(message) \
        std::cout << "[DEBUG] " << message << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl
#endif

// ============================================================================
// DEPRECATION WARNINGS
// ============================================================================

#ifdef __GNUC__
#define WEBSOCKET_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define WEBSOCKET_DEPRECATED __declspec(deprecated)
#else
#define WEBSOCKET_DEPRECATED
#endif

// ============================================================================
// PLATFORM DETECTION
// ============================================================================

#if defined(_WIN32) || defined(_WIN64)
#define WEBSOCKET_PLATFORM_WINDOWS 1
#define WEBSOCKET_PLATFORM_POSIX 0
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#define WEBSOCKET_PLATFORM_WINDOWS 0
#define WEBSOCKET_PLATFORM_POSIX 1
#else
#define WEBSOCKET_PLATFORM_WINDOWS 0
#define WEBSOCKET_PLATFORM_POSIX 0
#endif

// ============================================================================
// COMPILER DETECTION
// ============================================================================

#if defined(__clang__)
#define WEBSOCKET_COMPILER_CLANG 1
#define WEBSOCKET_COMPILER_GCC 0
#define WEBSOCKET_COMPILER_MSVC 0
#elif defined(__GNUC__) || defined(__GNUG__)
#define WEBSOCKET_COMPILER_CLANG 0
#define WEBSOCKET_COMPILER_GCC 1
#define WEBSOCKET_COMPILER_MSVC 0
#elif defined(_MSC_VER)
#define WEBSOCKET_COMPILER_CLANG 0
#define WEBSOCKET_COMPILER_GCC 0
#define WEBSOCKET_COMPILER_MSVC 1
#else
#define WEBSOCKET_COMPILER_CLANG 0
#define WEBSOCKET_COMPILER_GCC 0
#define WEBSOCKET_COMPILER_MSVC 0
#endif

#endif // WEBSOCKET_MACROS_HPP