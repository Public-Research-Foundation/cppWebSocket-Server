#pragma once

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>

WEBSOCKET_NAMESPACE_BEGIN

class BufferPool {
public:
    explicit BufferPool(size_t bufferSize = 8192, size_t maxPoolSize = 100);
    ~BufferPool() = default;

    WEBSOCKET_DISABLE_COPY(BufferPool)

        UniquePtr<ByteBuffer> acquire();
    void release(UniquePtr<ByteBuffer> buffer);

    void clear();
    void resize(size_t newBufferSize, size_t newMaxPoolSize);

    // Statistics
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

// RAII wrapper for buffer management
class ScopedBuffer {
public:
    explicit ScopedBuffer(BufferPool& pool);
    ~ScopedBuffer();

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