#pragma once
#ifndef WEBSOCKET_NONCOPYABLE_HPP
#define WEBSOCKET_NONCOPYABLE_HPP

/**
 * @file NonCopyable.hpp
 * @brief Base class to prevent copying and assignment
 *
 * This class follows the Resource Acquisition Is Initialization (RAII) principle
 * and prevents object copying and assignment to ensure proper resource management
 * in the WebSocket server components.
 */

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class NonCopyable
 * @brief Base class to make derived classes non-copyable
 *
 * Inheriting from this class prevents:
 * - Copy construction
 * - Copy assignment
 *
 * But allows:
 * - Move construction (if explicitly implemented)
 * - Move assignment (if explicitly implemented)
 * - Default construction and destruction
 *
 * @usage
 * class MyClass : public NonCopyable {
 *     // MyClass cannot be copied, but can be moved if implemented
 * };
 */
    class NonCopyable {
    protected:
        /**
         * @brief Default constructor
         *
         * Protected to prevent direct instantiation of NonCopyable.
         * Only derived classes can be instantiated.
         */
        NonCopyable() = default;

        /**
         * @brief Default destructor
         *
         * Virtual to allow proper destruction of derived classes.
         */
        virtual ~NonCopyable() = default;

        /**
         * @brief Deleted copy constructor
         *
         * Prevents copying of any derived class instances.
         * This ensures unique ownership of resources like:
         * - File descriptors
         * - Socket handles
         * - Memory buffers
         * - Thread locks
         */
        NonCopyable(const NonCopyable&) = delete;

        /**
         * @brief Deleted copy assignment operator
         *
         * Prevents assignment of any derived class instances.
         * This maintains resource ownership integrity and
         * prevents accidental resource sharing or double-free.
         */
        NonCopyable& operator=(const NonCopyable&) = delete;

        /**
         * @brief Default move constructor
         *
         * Allows derived classes to implement move semantics if needed.
         * Derived classes should explicitly define move operations
         * if they manage transferable resources.
         */
        NonCopyable(NonCopyable&&) = default;

        /**
         * @brief Default move assignment operator
         *
         * Allows derived classes to implement move semantics if needed.
         * Derived classes should explicitly define move operations
         * if they manage transferable resources.
         */
        NonCopyable& operator=(NonCopyable&&) = default;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_NONCOPYABLE_HPP