#pragma once
#ifndef WEBSOCKET_CRYPTO_HPP
#define WEBSOCKET_CRYPTO_HPP

#include "../common/Types.hpp"
#include "../common/Macros.hpp"
#include <string>
#include <vector>
#include <stdexcept>

// Forward declarations to avoid including OpenSSL headers in interface
typedef struct evp_md_ctx_st EVP_MD_CTX;
typedef struct hmac_ctx_st HMAC_CTX;

WEBSOCKET_NAMESPACE_BEGIN

/**
 * @class Crypto
 * @brief Cryptographic utilities for WebSocket protocol compliance and security
 *
 * Implements RFC 6455 required cryptographic operations including:
 * - WebSocket handshake key generation and verification
 * - Frame masking operations
 * - Hashing and HMAC calculations
 * - Base64 encoding/decoding
 *
 * Wrapper around OpenSSL with RAII and exception safety.
 */
    class Crypto {
    public:
        /**
         * @brief Crypto operation result codes
         */
        enum class Result {
            SUCCESS,
            INVALID_INPUT,
            OPENSSL_ERROR,
            BUFFER_TOO_SMALL,
            UNSUPPORTED_OPERATION
        };

        /**
         * @brief Get singleton instance
         * @return Reference to crypto utilities instance
         */
        static Crypto& getInstance();

        // Delete copy and move operations
        WEBSOCKET_DISABLE_COPY(Crypto)
            WEBSOCKET_DISABLE_MOVE(Crypto)

            // ===== INITIALIZATION AND CLEANUP =====

            /**
             * @brief Initialize crypto subsystem
             * @return true if initialization successful
             */
            static bool initialize();

        /**
         * @brief Cleanup crypto resources
         */
        static void cleanup();

        // ===== HASHING FUNCTIONS =====

        /**
         * @brief Compute SHA-1 hash of data (used in WebSocket handshake)
         * @param data Input data to hash
         * @return SHA-1 hash as byte buffer
         * @throws std::runtime_error if hashing fails
         */
        static ByteBuffer sha1(const ByteBuffer& data);

        /**
         * @brief Compute SHA-256 hash of data
         * @param data Input data to hash
         * @return SHA-256 hash as byte buffer
         * @throws std::runtime_error if hashing fails
         */
        static ByteBuffer sha256(const ByteBuffer& data);

        /**
         * @brief Compute SHA-1 hash and return as hex string
         * @param data Input string to hash
         * @return SHA-1 hash as hexadecimal string (lowercase)
         */
        static std::string sha1Hex(const std::string& data);

        /**
         * @brief Compute SHA-256 hash and return as hex string
         * @param data Input string to hash
         * @return SHA-256 hash as hexadecimal string (lowercase)
         */
        static std::string sha256Hex(const std::string& data);

        // ===== HMAC FUNCTIONS =====

        /**
         * @brief Compute HMAC-SHA1 of data with key
         * @param data Data to authenticate
         * @param key Secret key
         * @return HMAC-SHA1 result as byte buffer
         */
        static ByteBuffer hmacSha1(const ByteBuffer& data, const ByteBuffer& key);

        /**
         * @brief Compute HMAC-SHA256 of data with key
         * @param data Data to authenticate
         * @param key Secret key
         * @return HMAC-SHA256 result as byte buffer
         */
        static ByteBuffer hmacSha256(const ByteBuffer& data, const ByteBuffer& key);

        // ===== BASE64 ENCODING =====

        /**
         * @brief Base64 encode binary data (RFC 4648)
         * @param data Binary data to encode
         * @return Base64 encoded string (without line breaks)
         */
        static std::string base64Encode(const ByteBuffer& data);

        /**
         * @brief Base64 decode string to binary data
         * @param encoded Base64 encoded string
         * @return Decoded binary data
         * @throws std::runtime_error if decoding fails
         */
        static ByteBuffer base64Decode(const std::string& encoded);

        // ===== WEBSOCKET SPECIFIC OPERATIONS =====

        /**
         * @brief Generate WebSocket accept key from client key
         * Implements RFC 6455 Section 1.3: base64(sha1(key + GUID))
         * @param clientKey Client's Sec-WebSocket-Key (base64 encoded)
         * @return Sec-WebSocket-Accept value (base64 encoded)
         * @throws std::runtime_error if input is invalid
         */
        static std::string generateWebSocketAcceptKey(const std::string& clientKey);

        /**
         * @brief Verify WebSocket key handshake
         * @param clientKey Client's Sec-WebSocket-Key
         * @param serverAccept Server's Sec-WebSocket-Accept
         * @return true if handshake is cryptographically valid
         */
        static bool verifyWebSocketKey(const std::string& clientKey, const std::string& serverAccept);

        // ===== RANDOM GENERATION =====

        /**
         * @brief Generate cryptographically secure random bytes
         * @param length Number of bytes to generate
         * @return Random bytes as buffer
         * @throws std::runtime_error if random generation fails
         */
        static ByteBuffer generateRandomBytes(size_t length);

        /**
         * @brief Generate cryptographically secure random string
         * @param length Desired string length
         * @param charset Character set to use (default: alphanumeric)
         * @return Random string
         */
        static std::string generateRandomString(size_t length,
            const std::string& charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

        // ===== FRAME MASKING =====

        /**
         * @brief Apply XOR mask to WebSocket frame data (RFC 6455 Section 5.3)
         * @param data Data to mask (modified in-place)
         * @param mask 4-byte masking key
         * @throws std::invalid_argument if mask is not 4 bytes
         */
        static void applyMask(ByteBuffer& data, const ByteBuffer& mask);

        /**
         * @brief Apply XOR mask to raw byte array
         * @param data Pointer to data (must be valid for length bytes)
         * @param length Data length in bytes
         * @param mask 4-byte masking key
         * @throws std::invalid_argument if mask is not 4 bytes or data is null
         */
        static void applyMask(Byte* data, size_t length, const Byte* mask);

        /**
         * @brief Generate random masking key for WebSocket frames
         * @return 4-byte random masking key
         */
        static ByteBuffer generateMask();

        // ===== UTILITY FUNCTIONS =====

        /**
         * @brief Convert bytes to hexadecimal string
         * @param data Binary data to convert
         * @return Hexadecimal string (lowercase)
         */
        static std::string bytesToHex(const ByteBuffer& data);

        /**
         * @brief Convert hexadecimal string to bytes
         * @param hex Hexadecimal string (with or without 0x prefix)
         * @return Binary data
         * @throws std::invalid_argument if hex string is invalid
         */
        static ByteBuffer hexToBytes(const std::string& hex);

        /**
         * @brief Constant-time memory comparison to prevent timing attacks
         * @param a First buffer
         * @param b Second buffer
         * @return true if buffers are equal, false otherwise
         */
        static bool constantTimeCompare(const ByteBuffer& a, const ByteBuffer& b);

    private:
        /**
         * @brief Private constructor for singleton
         */
        Crypto() = default;

        /**
         * @brief Private destructor
         */
        ~Crypto() = default;

        /**
         * @brief Check if OpenSSL is initialized
         * @throws std::runtime_error if OpenSSL not initialized
         */
        static void checkInitialized();

        // Static initialization flag
        static std::atomic<bool> initialized_;
};

WEBSOCKET_NAMESPACE_END

#endif // WEBSOCKET_CRYPTO_HPP