#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Memory buffer pool for zero-copy operations and reduced allocations
 * Improves performance by reusing buffers instead of frequent allocations
 * Thread-safe and configurable
 */
    class BufferPool {
    public:
        /**
         * Create buffer pool with specified parameters
         * @param bufferSize Size of each buffer in bytes
         * @param maxPoolSize Maximum number of buffers to keep in pool
         */
        explicit BufferPool(size_t bufferSize = 8192, size_t maxPoolSize = 100);
        ~BufferPool() = default;

        WEBSOCKET_DISABLE_COPY(BufferPool)

            /**
             * Acquire buffer from pool
             * @return Unique pointer to buffer (automatically returns to pool when destroyed)
             */
            UniquePtr<ByteBuffer> acquire();

        /**
         * Return buffer to pool for reuse
         * @param buffer Buffer to return (must have been acquired from this pool)
         */
        void release(UniquePtr<ByteBuffer> buffer);

        /**
         * Clear all buffers from pool
         * Useful for memory cleanup or configuration changes
         */
        void clear();

        /**
         * Resize pool with new parameters
         * @param newBufferSize New buffer size
         * @param newMaxPoolSize New maximum pool size
         */
        void resize(size_t newBufferSize, size_t newMaxPoolSize);

        // ===== STATISTICS =====

        size_t getAvailableCount() const;
        size_t getBufferSize() const;
        size_t getMaxPoolSize() const;
        size_t getTotalAllocations() const;
        size_t getTotalReleases() const;

        struct Stats {
            size_t available;
            size_t bufferSize;
            size_t maxPoolSize;
            size_t totalAllocations;
            size_t totalReleases;
        };

        Stats getStats() const;

    private:
        mutable std::mutex mutex_;
        std::queue<UniquePtr<ByteBuffer>> availableBuffers_;

        size_t bufferSize_;
        size_t maxPoolSize_;

        std::atomic<size_t> totalAllocations_{ 0 };
        std::atomic<size_t> totalReleases_{ 0 };
};

/**
 * RAII wrapper for automatic buffer management
 * Automatically returns buffer to pool when destroyed
 * Provides pointer-like interface to underlying buffer
 */
class ScopedBuffer {
public:
    /**
     * Acquire buffer from pool (constructor)
     * @param pool Buffer pool to acquire from
     */
    explicit ScopedBuffer(BufferPool& pool);

    /**
     * Return buffer to pool (destructor)
     */
    ~ScopedBuffer();

    // Pointer-like interface for easy access
    ByteBuffer* operator->() const { return buffer_.get(); }
    ByteBuffer& operator*() const { return *buffer_; }
    ByteBuffer* get() const { return buffer_.get(); }

    WEBSOCKET_DEFAULT_MOVE(ScopedBuffer)
        WEBSOCKET_DISABLE_COPY(ScopedBuffer)

private:
    BufferPool& pool_;
    UniquePtr<ByteBuffer> buffer_;
};

WEBSOCKET_NAMESPACE_END