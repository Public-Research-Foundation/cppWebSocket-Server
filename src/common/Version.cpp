#include "common/Version.hpp"
#include <sstream>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * Version - Implementation of version information and utilities
 * Provides version string formatting, compatibility checking, and build information
 */

 // Version string generation

 /**
  * Generate version string in semantic versioning format
  *
  * @return: String in format "MAJOR.MINOR.PATCH"
  *
  * Purpose: Provides standardized version string for display and logging
  * Example: "1.2.3" for MAJOR=1, MINOR=2, PATCH=3
  */
    std::string Version::toString() {
    std::ostringstream oss;
    oss << MAJOR << "." << MINOR << "." << PATCH;
    return oss.str();
}

/**
 * Generate full version string with additional build information
 *
 * @return: Comprehensive version string with build metadata
 *
 * Purpose: Provides detailed version information including build timestamp and git commit
 * Used for debugging, support, and version identification
 */
std::string Version::getFullVersion() {
    std::ostringstream oss;
    oss << "CppWebSocket-Server v" << toString();

    // Add build timestamp if available during compilation
    // BUILD_TIMESTAMP is defined by build system (CMake)
#ifdef BUILD_TIMESTAMP
    oss << " (built: " << BUILD_TIMESTAMP << ")";
#endif

    // Add git commit hash for precise version tracking
    // GIT_COMMIT_HASH is defined by build system
#ifdef GIT_COMMIT_HASH
    oss << " (commit: " << GIT_COMMIT_HASH << ")";
#endif

    // Add build type (Debug, Release, etc.)
    // BUILD_TYPE is defined by CMake
#ifdef BUILD_TYPE
    oss << " [" << BUILD_TYPE << "]";
#endif

    return oss.str();
}

// Version compatibility checking

/**
 * Check if current version is compatible with specified major.minor version
 *
 * @param major: Required major version
 * @param minor: Required minimum minor version
 * @return: true if compatible according to semantic versioning rules
 *
 * Purpose: Implements semantic versioning compatibility rules
 * - Same major version required
 * - Current minor version must be >= required minor version
 * - Patch version doesn't affect compatibility
 */
bool Version::isCompatibleWith(int major, int minor) {
    // Semantic versioning rule: 
    // Same major version, current minor >= required minor
    return MAJOR == major && MINOR >= minor;
}

/**
 * Check if current version is at least the specified version
 *
 * @param major: Required major version
 * @param minor: Required minor version
 * @param patch: Required patch version
 * @return: true if current version >= specified version
 *
 * Purpose: Exact version comparison for feature availability checking
 * Used to enable/disable features based on version requirements
 */
bool Version::isAtLeast(int major, int minor, int patch) {
    // Compare major version first
    if (MAJOR > major) return true;
    if (MAJOR == major && MINOR > minor) return true;
    if (MAJOR == major && MINOR == minor && PATCH >= patch) return true;
    return false;
}

/**
 * Get detailed build information
 *
 * @return: Multi-line string with compiler, build time, and platform information
 *
 * Purpose: Provides comprehensive build environment information
 * Used for technical support and debugging build-related issues
 */
std::string Version::getBuildInfo() {
    std::ostringstream oss;

    // Compiler version information
    // COMPILER_VERSION is defined by build system
#ifdef COMPILER_VERSION
    oss << "Compiler: " << COMPILER_VERSION << "\n";
#endif

    // Build timestamp
#ifdef BUILD_TIMESTAMP
    oss << "Build time: " << BUILD_TIMESTAMP << "\n";
#endif

    // Git source control information
#ifdef GIT_COMMIT_HASH
    oss << "Git commit: " << GIT_COMMIT_HASH << "\n";
#endif

    // Target platform information
#ifdef BUILD_PLATFORM
    oss << "Platform: " << BUILD_PLATFORM;
#endif

    return oss.str();
}

WEBSOCKET_NAMESPACE_END