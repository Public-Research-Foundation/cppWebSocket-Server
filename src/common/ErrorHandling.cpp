#include "common/Types.hpp"
#include "common/Macros.hpp"
#include <system_error>
#include <exception>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * ErrorHandling - System error and exception handling utilities
 * Provides consistent error reporting and conversion between error systems
 */

 /**
  * Get last system error code and message
  *
  * @return: Error object containing system error code and description
  *
  * Purpose: Captures the last error from system calls (errno on Unix, GetLastError on Windows)
  * Used to convert system-level errors to our Error format
  */
    Error getLastSystemError() {
    int errorCode =
#ifdef _WIN32
        GetLastError();  // Windows system error code
#else
        errno;          // Unix/Linux error code
#endif

    // Use C++11 system_error category for portable error message
    std::error_code ec(errorCode, std::system_category());
    return Error(errorCode, ec.message());
}

/**
 * Convert C++ exception to Error object
 *
 * @param e: Standard exception caught in catch block
 * @return: Error object representing the exception
 *
 * Purpose: Converts C++ exceptions to our unified Error format
 * Ensures consistent error handling regardless of error source
 */
Error errorFromException(const std::exception& e) {
    return Error(-1, std::string("Exception: ") + e.what());
}

/**
 * Convert unknown exception to Error object
 *
 * @return: Error object for unknown exception type
 *
 * Purpose: Catch-all for non-standard exceptions (not derived from std::exception)
 * Prevents unhandled exceptions from crashing the application
 */
Error errorFromUnknownException() {
    return Error(-2, "Unknown exception occurred");
}

/**
 * Get human-readable description for error codes
 *
 * @param errorCode: Application-specific error code
 * @return: Descriptive string for the error code
 *
 * Purpose: Provides meaningful descriptions for our custom error codes
 * Used in logging, user interfaces, and debugging
 */
std::string getErrorDescription(int errorCode) {
    switch (errorCode) {
    case 0: return "Success";
    case -1: return "Generic error";
    case -2: return "Invalid argument";
    case -3: return "Operation timed out";
    case -4: return "Resource unavailable";
    case -5: return "Operation not permitted";
    case -6: return "I/O error";
    case -7: return "Network error";
    case -8: return "Protocol error";
    case -9: return "Connection closed";
    case -10: return "Buffer overflow";
    case -11: return "Invalid state";
    case -12: return "Not implemented";
    case -13: return "Configuration error";
    default: return "Unknown error";
    }
}

// Result checking utilities

/**
 * Check result and handle errors
 *
 * @param result: Result value to check
 * @param context: Context string for error logging
 * @return: true if result is SUCCESS, false otherwise
 *
 * Purpose: Centralized result checking with automatic error handling
 * Reduces boilerplate error checking code throughout the application
 */
bool checkResult(Result result, const std::string& context) {
    if (result != Result::SUCCESS) {
        // Log error with context for debugging
        // In production, this would use the logging system
        return false;
    }
    return true;
}

/**
 * Convert boolean to Result with specified success/failure values
 *
 * @param success: Boolean indicating operation success
 * @param successResult: Result to return if success is true
 * @param failureResult: Result to return if success is false
 * @return: Appropriate Result value based on success
 *
 * Purpose: Clean conversion from boolean operations to Result enum
 * Improves code readability in conditionals
 */
Result resultFromBool(bool success, Result successResult, Result failureResult) {
    return success ? successResult : failureResult;
}

/**
 * Convert system error code to Result enum
 *
 * @param errorCode: System error code (errno or Windows error code)
 * @return: Corresponding Result enum value
 *
 * Purpose: Maps platform-specific error codes to our portable Result enum
 * Provides consistent error handling across different platforms
 */
Result resultFromErrorCode(int errorCode) {
    if (errorCode == 0) return Result::SUCCESS;

    // Map common system error codes to our Result enum
    switch (errorCode) {
    case ETIMEDOUT:        // Unix timeout
    case WSAETIMEDOUT:     // Windows socket timeout
        return Result::TIMEOUT;

    case ECONNRESET:       // Unix connection reset
    case WSAECONNRESET:    // Windows connection reset  
    case EPIPE:            // Unix broken pipe
        return Result::CONNECTION_CLOSED;

    case EINVAL:           // Unix invalid argument
    case WSAEINVAL:        // Windows invalid argument
        return Result::INVALID_STATE;

    default:
        return Result::ERROR;
    }
}

WEBSOCKET_NAMESPACE_END