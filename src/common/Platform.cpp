#include "common/Types.hpp"
#include "common/Macros.hpp"
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Platform - Cross-platform system utility implementations
 * Provides unified interface for platform-specific system operations
 */

 // Process and thread identification

 /**
  * Get current thread ID
  *
  * @return: Platform-independent thread identifier
  *
  * Purpose: Provides thread ID for logging, debugging, and thread-specific operations
  * Uses system-specific APIs but returns consistent uint64_t format
  */
    uint64_t getCurrentThreadId() {
#ifdef _WIN32
    // Windows: GetCurrentThreadId returns DWORD (32-bit)
    return GetCurrentThreadId();
#else
    // Linux/Unix: syscall gettid for actual kernel thread ID
    return static_cast<uint64_t>(syscall(SYS_gettid));
#endif
}

/**
 * Get current process ID
 *
 * @return: Platform-independent process identifier
 *
 * Purpose: Provides process ID for logging, process management, and isolation
 */
uint64_t getCurrentProcessId() {
#ifdef _WIN32
    // Windows: GetCurrentProcessId
    return GetCurrentProcessId();
#else
    // Linux/Unix: getpid
    return static_cast<uint64_t>(getpid());
#endif
}

/**
 * Get platform name
 *
 * @return: String identifying the operating system platform
 *
 * Purpose: Platform detection for conditional compilation alternatives
 * Used for logging and platform-specific feature enabling
 */
std::string getPlatformName() {
#ifdef _WIN32
    return "Windows";
#elif __linux__
    return "Linux";
#elif __APPLE__
    return "macOS";
#elif __unix__
    return "Unix";
#else
    return "Unknown";
#endif
}

// Timing and sleep functions

/**
 * Sleep for specified milliseconds
 *
 * @param milliseconds: Number of milliseconds to sleep
 *
 * Purpose: Cross-platform millisecond precision sleep
 * Used for delays, timeouts, and rate limiting
 */
void sleepMilliseconds(uint32_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

/**
 * Sleep for specified microseconds
 *
 * @param microseconds: Number of microseconds to sleep
 *
 * Purpose: High-precision sleep for performance-critical timing
 * Used in tight loops and high-frequency operations
 */
void sleepMicroseconds(uint32_t microseconds) {
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

// Time retrieval functions

/**
 * Get current time in milliseconds since epoch
 *
 * @return: Milliseconds since steady_clock epoch
 *
 * Purpose: High-resolution timing for performance measurements
 * Uses steady_clock for monotonic timing (not affected by system clock changes)
 */
uint64_t getCurrentTimeMillis() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

/**
 * Get current time in microseconds since epoch
 *
 * @return: Microseconds since steady_clock epoch
 *
 * Purpose: Very high-resolution timing for micro-benchmarks
 * Used for fine-grained performance analysis
 */
uint64_t getCurrentTimeMicros() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
}

/**
 * Get current time in nanoseconds since epoch
 *
 * @return: Nanoseconds since steady_clock epoch
 *
 * Purpose: Highest resolution timing for nanosecond-level measurements
 * Used in performance-critical code paths
 */
uint64_t getCurrentTimeNanos() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
}

// System information

/**
 * Get number of available processors
 *
 * @return: Number of logical processors available to the system
 *
 * Purpose: Used for thread pool sizing and parallelization decisions
 * Helps optimize performance by matching thread count to available cores
 */
uint32_t getProcessorCount() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

/**
 * Get total system memory size
 *
 * @return: Total physical memory in bytes
 *
 * Purpose: Memory management and resource allocation decisions
 * Used for setting memory limits and monitoring resource usage
 */
uint64_t getSystemMemorySize() {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#else
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#endif
}

// High-resolution timer implementation

/**
 * HighResolutionTimer - RAII timer for precise performance measurements
 * Uses highest resolution clock available on the platform
 */
class HighResolutionTimer {
private:
    // Start time captured when timer is constructed or reset
    std::chrono::high_resolution_clock::time_point start_;

public:
    /**
     * Constructor - captures current time as start point
     */
    HighResolutionTimer() : start_(std::chrono::high_resolution_clock::now()) {}

    /**
     * Reset timer to current time
     *
     * Purpose: Reuse timer object for multiple measurements
     */
    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }

    /**
     * Get elapsed time in microseconds
     *
     * @return: Microseconds since timer start or last reset
     *
     * Purpose: Precise timing for performance measurements
     */
    uint64_t elapsedMicroseconds() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            end - start_).count();
    }

    /**
     * Get elapsed time in nanoseconds
     *
     * @return: Nanoseconds since timer start or last reset
     *
     * Purpose: Highest precision timing for micro-benchmarks
     */
    uint64_t elapsedNanoseconds() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start_).count();
    }

    /**
     * Get elapsed time in seconds as floating point
     *
     * @return: Seconds since timer start or last reset
     *
     * Purpose: Convenient timing for longer operations
     */
    double elapsedSeconds() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<double>>(
            end - start_).count();
    }
};

WEBSOCKET_NAMESPACE_END