#pragma once
#ifndef WEBSOCKET_VERSION_HPP
#define WEBSOCKET_VERSION_HPP

#include "Macros.hpp"
#include "Types.hpp"
#include <string>

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @struct Version
 * @brief Version information for the CppWebSocket-Server library
 *
 * Provides compile-time and runtime version information,
 * following semantic versioning (semver.org) principles.
 */
    struct Version {
    // ============================================================================
    // COMPILE-TIME VERSION CONSTANTS
    // ============================================================================

    static constexpr int MAJOR = 1;     ///< Major version (incompatible API changes)
    static constexpr int MINOR = 0;     ///< Minor version (backwards-compatible functionality)
    static constexpr int PATCH = 0;     ///< Patch version (backwards-compatible bug fixes)

    static constexpr const char* PRERELEASE = "";   ///< Pre-release identifier (alpha, beta, rc)
    static constexpr const char* BUILD = "";        ///< Build metadata

    // ============================================================================
    // VERSION STRING GENERATION
    // ============================================================================

    /**
     * @brief Get version as string (e.g., "1.0.0")
     * @return Semantic version string
     */
    static constexpr const char* getVersionString() {
        return "1.0.0";
    }

    /**
     * @brief Get full version string with pre-release and build info
     * @return Complete version string
     */
    static String getFullVersionString() {
        String version = getVersionString();

        if (PRERELEASE[0] != '\0') {
            version += "-";
            version += PRERELEASE;
        }

        if (BUILD[0] != '\0') {
            version += "+";
            version += BUILD;
        }

        return version;
    }

    /**
     * @brief Get library name with version
     * @return Library identification string
     */
    static String getLibraryName() {
        return "CppWebSocket-Server v" + getFullVersionString();
    }

    // ============================================================================
    // VERSION COMPARISON
    // ============================================================================

    /**
     * @brief Check if current version is compatible with required version
     * @param major Required major version
     * @param minor Required minor version
     * @return true if compatible (same major version and minor >= required)
     */
    static bool isCompatibleWith(int major, int minor = 0) {
        return (MAJOR == major) && (MINOR >= minor);
    }

    /**
     * @brief Compare versions
     * @param otherMajor Other major version
     * @param otherMinor Other minor version
     * @param otherPatch Other patch version
     * @return -1 if older, 0 if same, 1 if newer
     */
    static int compare(int otherMajor, int otherMinor = 0, int otherPatch = 0) {
        if (MAJOR != otherMajor) {
            return MAJOR < otherMajor ? -1 : 1;
        }
        if (MINOR != otherMinor) {
            return MINOR < otherMinor ? -1 : 1;
        }
        if (PATCH != otherPatch) {
            return PATCH < otherPatch ? -1 : 1;
        }
        return 0;
    }

    // ============================================================================
    // FEATURE DETECTION
    // ============================================================================

    /**
     * @brief Check if compiled with SSL/TLS support
     * @return true if SSL support enabled
     */
    static bool hasSSLSupport() {
#ifdef WEBSOCKET_SSL_SUPPORT
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if compiled with compression support
     * @return true if compression support enabled
     */
    static bool hasCompressionSupport() {
#ifdef WEBSOCKET_COMPRESSION_SUPPORT
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if compiled with metrics support
     * @return true if metrics collection enabled
     */
    static bool hasMetricsSupport() {
#ifdef WEBSOCKET_METRICS_SUPPORT
        return true;
#else
        return false;
#endif
    }

    // ============================================================================
    // BUILD INFORMATION
    // ============================================================================

    /**
     * @brief Get build timestamp
     * @return Build timestamp string
     */
    static String getBuildTimestamp() {
#ifdef WEBSOCKET_BUILD_TIMESTAMP
        return WEBSOCKET_BUILD_TIMESTAMP;
#else
        return "Unknown";
#endif
    }

    /**
     * @brief Get compiler information
     * @return Compiler identification string
     */
    static String getCompilerInfo() {
#if WEBSOCKET_COMPILER_MSVC
        return "MSVC";
#elif WEBSOCKET_COMPILER_GCC
        return "GCC";
#elif WEBSOCKET_COMPILER_CLANG
        return "Clang";
#else
        return "Unknown";
#endif
    }

    /**
     * @brief Get build configuration
     * @return Build configuration string
     */
    static String getBuildConfiguration() {
#ifdef NDEBUG
        return "Release";
#else
        return "Debug";
#endif
    }

    /**
     * @brief Get complete build information
     * @return Multi-line build information string
     */
    static String getBuildInfo() {
        return getLibraryName() + "\n" +
            "Built: " + getBuildTimestamp() + "\n" +
            "Compiler: " + getCompilerInfo() + "\n" +
            "Configuration: " + getBuildConfiguration() + "\n" +
            "Features: " + getFeatureString();
    }

    private:
        /**
         * @brief Get feature string for build information
         * @return Comma-separated feature list
         */
        static String getFeatureString() {
            String features;

            if (hasSSLSupport()) {
                features += "SSL,";
            }
            if (hasCompressionSupport()) {
                features += "Compression,";
            }
            if (hasMetricsSupport()) {
                features += "Metrics,";
            }

            // Remove trailing comma
            if (!features.empty()) {
                features.pop_back();
            }
            else {
                features = "None";
            }

            return features;
        }
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_VERSION_HPP