#pragma once
#ifndef WEBSOCKET_BUFFER_POOL_HPP
#define WEBSOCKET_BUFFER_POOL_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class BufferPool
 * @brief Memory buffer pool for zero-copy operations and reduced allocations
 *
 * Improves performance by reusing buffers instead of frequent allocations.
 * Thread-safe and configurable with comprehensive statistics tracking.
 *
 * Features:
 * - Zero-copy buffer management
 * - Thread-safe operations
 * - Configurable buffer sizes and pool limits
 * - Comprehensive statistics tracking
 * - Automatic buffer cleanup
 */
    class BufferPool {
    public:
        /**
         * @brief Buffer pool statistics structure
         */
        struct Stats {
            size_t available{ 0 };            ///< Number of available buffers in pool
            size_t bufferSize{ 0 };           ///< Size of each buffer in bytes
            size_t maxPoolSize{ 0 };          ///< Maximum pool size
            size_t totalAllocations{ 0 };     ///< Total buffers allocated
            size_t totalReleases{ 0 };        ///< Total buffers released
            size_t peakUsage{ 0 };            ///< Maximum concurrent buffers in use
        };

        /**
         * @brief Create buffer pool with specified parameters
         * @param bufferSize Size of each buffer in bytes (default: 8KB)
         * @param maxPoolSize Maximum number of buffers to keep in pool (default: 100)
         * @param preallocate Whether to preallocate buffers on construction
         */
        explicit BufferPool(size_t bufferSize = 8192, size_t maxPoolSize = 100, bool preallocate = false);

        /**
         * @brief Destructor - automatically cleans up all buffers
         */
        ~BufferPool();

        // Delete copy and move operations
        WEBSOCKET_DISABLE_COPY(BufferPool)
            WEBSOCKET_DISABLE_MOVE(BufferPool)

            /**
             * @brief Acquire buffer from pool
             * @return Unique pointer to buffer (automatically returns to pool when destroyed)
             *
             * @note If pool is empty, creates a new buffer. Returns nullptr if max pool size exceeded.
             */
            UniquePtr<ByteBuffer> acquire();

        /**
         * @brief Return buffer to pool for reuse
         * @param buffer Buffer to return (must have been acquired from this pool)
         *
         * @note Buffer is cleared before being returned to pool. If pool is full, buffer is destroyed.
         */
        void release(UniquePtr<ByteBuffer> buffer);

        /**
         * @brief Clear all buffers from pool
         *
         * Useful for memory cleanup or configuration changes.
         * All acquired buffers remain valid until released.
         */
        void clear();

        /**
         * @brief Resize pool with new parameters
         * @param newBufferSize New buffer size in bytes
         * @param newMaxPoolSize New maximum pool size
         *
         * @note Existing buffers are cleared. New buffers are created on next acquisition.
         */
        void resize(size_t newBufferSize, size_t newMaxPoolSize);

        /**
         * @brief Preallocate buffers in pool
         * @param count Number of buffers to preallocate
         *
         * Improves performance by allocating buffers upfront rather than on-demand.
         */
        void preallocate(size_t count);

        // ===== STATISTICS ACCESS =====

        /**
         * @brief Get number of available buffers in pool
         * @return Available buffer count
         */
        size_t getAvailableCount() const;

        /**
         * @brief Get buffer size in bytes
         * @return Buffer size
         */
        size_t getBufferSize() const;

        /**
         * @brief Get maximum pool size
         * @return Maximum number of buffers in pool
         */
        size_t getMaxPoolSize() const;

        /**
         * @brief Get total buffers allocated since creation
         * @return Total allocation count
         */
        size_t getTotalAllocations() const;

        /**
         * @brief Get total buffers released since creation
         * @return Total release count
         */
        size_t getTotalReleases() const;

        /**
         * @brief Get current number of buffers in use
         * @return Active buffer count
         */
        size_t getActiveCount() const;

        /**
         * @brief Get comprehensive pool statistics
         * @return Statistics structure
         */
        Stats getStats() const;

    private:
        mutable std::mutex mutex_;                          ///< Thread safety mutex
        std::queue<UniquePtr<ByteBuffer>> availableBuffers_; ///< Available buffers queue

        size_t bufferSize_;                                 ///< Size of each buffer
        size_t maxPoolSize_;                                ///< Maximum pool size
        size_t activeCount_{ 0 };                             ///< Currently active buffers

        std::atomic<size_t> totalAllocations_{ 0 };           ///< Total buffers allocated
        std::atomic<size_t> totalReleases_{ 0 };              ///< Total buffers released
        std::atomic<size_t> peakUsage_{ 0 };                  ///< Peak concurrent usage

        /**
         * @brief Create a new buffer
         * @return New buffer instance
         */
        UniquePtr<ByteBuffer> createBuffer();

        /**
         * @brief Update peak usage statistics
         */
        void updatePeakUsage();
};

/**
 * @class ScopedBuffer
 * @brief RAII wrapper for automatic buffer management
 *
 * Automatically returns buffer to pool when destroyed.
 * Provides pointer-like interface to underlying buffer.
 */
class ScopedBuffer {
public:
    /**
     * @brief Acquire buffer from pool (constructor)
     * @param pool Buffer pool to acquire from
     */
    explicit ScopedBuffer(BufferPool& pool);

    /**
     * @brief Return buffer to pool (destructor)
     */
    ~ScopedBuffer();

    // Pointer-like interface for easy access
    ByteBuffer* operator->() const { return buffer_.get(); }
    ByteBuffer& operator*() const { return *buffer_; }
    ByteBuffer* get() const { return buffer_.get(); }

    /**
     * @brief Get buffer size
     * @return Buffer size in bytes
     */
    size_t size() const { return buffer_ ? buffer_->size() : 0; }

    /**
     * @brief Check if buffer is valid
     * @return true if buffer is acquired and valid
     */
    explicit operator bool() const { return buffer_ != nullptr; }

    // Allow move semantics
    ScopedBuffer(ScopedBuffer&& other) noexcept;
    ScopedBuffer& operator=(ScopedBuffer&& other) noexcept;

    // Disable copy semantics
    WEBSOCKET_DISABLE_COPY(ScopedBuffer)

private:
    BufferPool& pool_;                   ///< Reference to source pool
    UniquePtr<ByteBuffer> buffer_;       ///< Managed buffer
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_BUFFER_POOL_HPP